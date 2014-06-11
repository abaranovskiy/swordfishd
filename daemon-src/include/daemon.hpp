/**
 * @file daemon.hpp
 * @author Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#ifndef __WAPSTART_SWORDFISH_DAEMON__H__
#define __WAPSTART_SWORDFISH_DAEMON__H__
//-------------------------------------------------------------------------------------------------
#include <signal.h>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_map.hpp>

//-------------------------------------------------------------------------------------------------
#include "server.hpp"
#include "storage_controller.hpp"
#include "cfg.hpp"
#include "abstract_filler.hpp"
//-------------------------------------------------------------------------------------------------
namespace wapstart {
    typedef Storage* (*GetStorageFunction)(const std::string&);
    class Daemon {
    public:
        typedef Daemon                  class_type;
        typedef int                     signal_type;
        typedef boost::asio::io_service service_type;
        typedef boost::ptr_map<std::string, Storage*> storage_list_type;
        typedef std::map<std::string, std::string> string_map;


    /**
     *
     */
    Daemon(const std::string &config_path);
    /**
     *
     */
    ~Daemon();
    /**
     *
     */
    void run(bool is_daemonize);

  private:
    Daemon(const class_type &);
    void operator =(const class_type &);
    /**
     *
     */
    Config cfg_;
    /**
     *
     */
    bool done_;
    storage_list_type storage_list_;
    string_map storage_function_map_;
    StorageController* storage_controller_;
    /**
     * Абстрагирует систему ввода/вывода.
     */
    service_type service_;
    /**
     *  
     */
    boost::ptr_list<AbstractFiller> fillers_;
    boost::thread_group filler_threads_;
    /**
     *
     */
    Server *server_;
    /**
     * Устанавливает обработчики сигналов.
     */
    void set_signal_handlers();
    /**
     *  
     */
    void init_logger();
    /**
     *
     */
    void dispatch_signal_handler(signal_type signal);
    /**
     * 
     */
    void on_exit();
    /**
     *
     */
    void on_config();
    /**
     *  
     */
    void on_expirate();
    /**
     *
     */
    void on_segfault();
    /**
     *
     */
    void init();
    /**
     *
     */
    void create_server();
    /**
     *
     */
    void create_storage();
    /**
     *
     */
    void recreate_fillers();
    /**
     *
     */
    void create_fillers(Storage*&, size_t, const std::string&);
    /**
     *
     */
    AbstractFiller* create_filler(Storage*&, const std::string&);
    /**
     *
     */
    void daemonize();
    /**
     *  
     */
    void reset_server();
    /**
     *
     */
    void reset_fillers();
    /**
     *
     */
    void reset_storage();
    /**
     *
     */
    void reload();
    /**
     *
     */
    void reset();


    /**
     *
     */
    static void signal_handler(int sig, siginfo_t *siginfo, void *context);
  };
  //-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------
#endif // __WAPSTART_SWORDFISH_DAEMON__H__
//-------------------------------------------------------------------------------------------------

