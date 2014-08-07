/**
 * @file daemon.cpp
 * @author  Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#include <iostream>
#include <sys/stat.h>
#include <execinfo.h>
#include <fcntl.h>

#include "daemon.hpp"
#include "logger.hpp"
//-------------------------------------------------------------------------------------------------
namespace wapstart {
  namespace ugly {
    Daemon *__this = NULL;
  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::signal_handler(int sig, siginfo_t *siginfo, void *context)
  {
    if(ugly::__this) ugly::__this->dispatch_signal_handler(sig);
  }
  //-----------------------------------------------------------------------------------------------
  Daemon::Daemon(const std::string &config_path): 
    cfg_(config_path), 
    done_(false), 
    server_(NULL),
    storage_controller_(NULL)
  {
      storage_controller_ = new StorageController();
    ugly::__this = this;
  }
  //-----------------------------------------------------------------------------------------------
  Daemon::~Daemon()
  {
    reset();
    __LOG_INFO << "Goodbye!...";
  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::run(bool is_daemonize)
  {
    init_logger();
    
    try {

      if(is_daemonize) daemonize();

      // Счетчик перезапусков
      std::size_t rn = 0;

      create_storage();
      create_server();

      while(!done_) {
        if(rn++) init_logger();


        set_signal_handlers();
        
        __LOG_INFO << "I have started " << rn << " time...";
        __LOG_INFO << "I'm waiting for the events...";
      
        server_->run();
        sleep(1);

        __LOG_INFO << "Event loop has stopped...";
      }
    }
    catch(const std::exception &x) {
      __LOG_CRIT << "Exceptions! " << x.what();
    }
  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::init_logger()
  {
    bool fl = false;
    
    logger_backends_init_start();
    
    if(cfg_.is_log_syslog()) {
      logger_syslog_sink_init();
    }

    if(cfg_.is_log_stdout()) {
      logger_stdout_sink_init();
    }
    
    if(cfg_.is_log_file()) {
      fl = logger_file_sink_init(cfg_.log_file_path());
    }
    
    logger_set_severity_level(cfg_.log_level());

    logger_backends_init_commit();

    if(cfg_.is_log_file() && !fl)
      __LOG_ERROR << "Failed to open '" << cfg_.log_file_path() << "' for logging...";
  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::set_signal_handlers() 
  {
    struct sigaction act;
     
    memset (&act, '\0', sizeof(act));
       
    act.sa_sigaction = &signal_handler;
           
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGTERM, &act, NULL) < 0)
      throw std::runtime_error("Can't set sigaction for SIGTERM");

    if (sigaction(SIGINT, &act, NULL) < 0)
      throw std::runtime_error("Can't set sigaction for SIGINT");

    if (sigaction(SIGUSR1, &act, NULL) < 0)
      throw std::runtime_error("Can't set sigaction for SIGUSR1");

    if (sigaction(SIGUSR2, &act, NULL) < 0)
      throw std::runtime_error("Can't set sigaction for SIGUSR2");

    if (sigaction(SIGSEGV, &act, NULL) < 0)
      throw std::runtime_error("Can't set sigaction for SIGSEGV");
  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::dispatch_signal_handler(signal_type signal) 
  {
    if(signal == SIGTERM || signal == SIGINT)
      service_.dispatch(boost::bind(&Daemon::on_exit, this));
    else if(signal == SIGUSR1)
      service_.dispatch(boost::bind(&Daemon::on_config, this));
    else if(signal == SIGUSR2)
      service_.dispatch(boost::bind(&Daemon::on_expirate, this));
    else if(signal == SIGSEGV)
      service_.dispatch(boost::bind(&Daemon::on_segfault, this));
  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::daemonize()
  {
    __LOG_DEBUG << "I'm demonizing...";
    
    if(pid_t pid = fork()) {
      if(pid > 0) {
        exit(0);
      }
      else {
        __LOG_EMERG << "Failed to first fork!";
        exit(1);
      }
    }

    __LOG_DEBUG << "First fork ok..."; 

    __LOG_DEBUG << "setsid()...";
    
    setsid();
    
    __LOG_DEBUG << "chdir(\"/\")...";

    chdir("/");
    
    __LOG_DEBUG << "umask(0)...";
    
    umask(0);

    if(pid_t pid = fork()) {
      if(pid > 0) {
        exit(0);
      }
      else {
        __LOG_EMERG << "Failed to second fork!";
        exit(1);
      }
    }

    __LOG_DEBUG << "Second fork ok...";

    __LOG_DEBUG << "Closing std io descriptors...";
    
    close(0);
    close(1);
    close(2);
  }

  //-----------------------------------------------------------------------------------------------
  void Daemon::init()
  {
      create_storage();
      create_server();
  }

  void Daemon::create_server()
  {
    __LOG_DEBUG << "I'm creating the server...";
    // Конструируем новый сервер
    server_ = new Server(service_, storage_controller_, cfg_.port(), cfg_.workers());
    server_->reload();
    //server_->configure();
  }

    void Daemon::create_storage()
    {
        __LOG_DEBUG << "I'm creating the storage...";
        std::vector<std::string> storage_names_ = cfg_.storage_list();
        std::vector<std::string>::iterator i = storage_names_.begin();
        while (i != storage_names_.end()) {
            __LOG_DEBUG << "Create storage " << *i;
            Storage* storage = new Storage(
                cfg_.storage_ttl(*i),
                cfg_.storage_size(*i),
                cfg_.filler_queue_size(*i),
                cfg_.storage_expirate_size(*i)
            );
            storage->Configure(cfg_.filler(*i));
            storage_controller_->addStorage(*i, storage, cfg_.storage_functions(*i));

            __LOG_DEBUG << "Starting " << cfg_.fillers(*i) << " for storage " << *i;
            create_fillers(storage, cfg_.fillers(*i), *i);

            // Fill function map
            if (cfg_.storage_default(*i)) {
                storage_function_map_["default"] = *i;
            } else {
                std::vector<std::string> functions = cfg_.storage_functions(*i);
                std::vector<std::string>::iterator func = functions.begin();
                while (func != functions.end()) {
                    storage_function_map_[*func] = *i;
                    func++;
                }
            }

            i++;
        }
    }

    void Daemon::recreate_fillers()
    {
        __LOG_DEBUG << "Recreate fillers";
        reset_fillers();

        std::vector<std::string> storage_names_ = cfg_.storage_list();
        std::vector<std::string>::iterator i = storage_names_.begin();
        while (i != storage_names_.end()) {
            create_fillers(
                storage_controller_->getStorage(*i),
                cfg_.fillers(*i),
                *i
            );
            i++;
        }
    }

  void Daemon::create_fillers(Storage*& storage, size_t fillers_count, const std::string& storage_id)
  {
      __LOG_DEBUG << "Creating fillers (" << fillers_count << ") for storage " << storage_id;
      for (int i = 0; i < fillers_count; i++) {
          AbstractFiller* filler;
          filler = create_filler(storage, storage_id);
          // Создаем поток наполнятора
          fillers_.push_back(filler);
          filler_threads_.add_thread(new boost::thread(boost::ref(*filler)));
          __LOG_DEBUG << "Filler #" << i << " for " << storage_id << " has been started.";
      }
  }

  AbstractFiller* Daemon::create_filler(Storage*& storage, const std::string& storage_id)
  {
      AbstractFiller *filler;
      __LOG_DEBUG << "I'm creating the filler...";
      filler = new AbstractFiller(storage, cfg_.get_config());

      filler->Configure(cfg_.filler(storage_id), cfg_.filler_function(), cfg_.max_fill_size(storage_id));

      return filler;
  }

  //-----------------------------------------------------------------------------------------------
    void Daemon::reset()
    {
        reset_storage();
        reset_fillers();
        reset_server();

    }

    void Daemon::reload()
    {
        reset();
        init();
    }

  void Daemon::reset_server()
  {
      __LOG_DEBUG << "Deleting server";
      if (server_) {
          service_.stop();
          delete server_;

      }
  }

  void Daemon::reset_storage()
  {
      __LOG_DEBUG << "Reseting storage";
      storage_list_.clear();
  }

  void Daemon::reset_fillers()
  {
      __LOG_INFO << "I'm deleting filler threads...";
      boost::ptr_list<AbstractFiller>::iterator i = fillers_.begin();
      while (i != fillers_.end()) {
        __LOG_INFO << "Shutdown filler ... ";
        i->Shutdown();
        i++;
      }
      __LOG_INFO << "I'm deleting fillers...";
      filler_threads_.join_all();
      fillers_.clear();
  }

  //-----------------------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------------------
  void Daemon::on_exit()
  {
    __LOG_INFO << "I'm stopping...";
    
    done_ = true;
    

    service_.stop();
  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::on_config()
  {
    __LOG_INFO << "I'm restarting...";
    
    cfg_.reload();

    // FIXME: Need to reconfigure storages in StorageController
    std::vector<std::string> storage_names_ = cfg_.storage_list();
    std::vector<std::string>::iterator i = storage_names_.begin();
    while (i != storage_names_.end()) {
        storage_controller_->
                getStorage(*i)->
                set_defaults(
                    cfg_.storage_ttl(*i),
                    cfg_.storage_size(*i),
                    cfg_.filler_queue_size(*i),
                    cfg_.storage_expirate_size(*i)
                );

        i++;
    }
/*
    storage_->set_defaults(
        cfg_.storage_ttl(),
        cfg_.storage_size(),
        cfg_.filler_queue_size(),
        cfg_.storage_expirate_size()
    );
*/
    server_->stop();
    service_.stop();
    reset_server();
    recreate_fillers();
    create_server();

  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::on_expirate()
  {
    __LOG_INFO << "I'm expirating the storage...";
    
    storage_list_type::iterator iter = storage_list_.begin();
    while (iter != storage_list_.end())
        (*iter->second)->expirate();

  }
  //-----------------------------------------------------------------------------------------------
  void Daemon::on_segfault()
  {
    __LOG_CRIT << "Segfault happens...";
    __LOG_CRIT << "-- Backtrace --";

    void *callstack[128];
    int frames=backtrace(callstack, 128);
    char **strs=backtrace_symbols(callstack, frames);

    for(int i = 0; i < frames; ++i)
      __LOG_CRIT << strs[i];

    free(strs);

    __LOG_CRIT << "-- /Backtrace --";

    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_handler = SIG_DFL;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &act, NULL) < 0)
      throw std::runtime_error("Can't set sigaction for SIGSEGV to SIG_DFL");
  }
  //-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------
