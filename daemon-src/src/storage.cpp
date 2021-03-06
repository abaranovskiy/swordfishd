/**
 * @file storage.cpp
 * @author  Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "storage.hpp"
#include "boost/lexical_cast.hpp"
#include "logger.hpp"
//-------------------------------------------------------------------------------------------------
namespace wapstart {
  Storage::Storage(size_t ttl, size_t max_storage_size, size_t max_queue_size, size_t expirate_size)
    : storage_(storage_type::ttl_type(boost::posix_time::seconds(ttl))),
      max_storage_size_(max_storage_size),
      max_queue_size_(max_queue_size),
      expirate_size_(expirate_size)
  {
      stats_ = new Stats();
  }

  void Storage::set_defaults(size_t ttl, size_t max_storage_size, size_t max_queue_size, size_t expirate_size)
  {
      __LOG_DEBUG << "Reconfiguring storage";
      max_storage_size_ = max_storage_size;
      max_queue_size_ = max_queue_size;
      expirate_size_ = expirate_size;
      storage_.set_ttl(storage_type::ttl_type(boost::posix_time::seconds(ttl)));
  }

  //-----------------------------------------------------------------------------------------------
  void Storage::_do(const cmd_type  &command, result_type& result) 
  {
    /*if (command.arg_begin() != command.arg_end())
      printf("%s : [", command.name().c_str());
    for(Command::arg_iterator x = command.arg_begin(); x != command.arg_end(); ++x) 
      if((x + 1) != command.arg_end()) 
        printf("%s, ", x->c_str());
      else
        printf("%s]\n", x->c_str());
    */
    if (command.name() == "stats")
      get_stats(result);
    else if (command.name() == "get")
      get_val(command, result);
    else if (command.name() == "quit")
      return;
  }
//-------------------------------------------------------------------------------------------------

  void Storage::add_item(const key_type& key, const val_type& val)
  {
    boost::mutex::scoped_lock lock(mutex_);
    while (storage_.get_storage_size() >= max_storage_size_)
    {
      expirate();
      sleep(1);
    }
    storage_.add(key, val); 
  }
//-------------------------------------------------------------------------------------------------

  void Storage::reset_stats()
  {
    __LOG_DEBUG << "[Storage::reset_stats]";
    stats_->set_start_time();
    stats_->set_storage_size(storage_.get_storage_size());
    stats_->set_deleted(0);
    stats_->set_queue_size(queue_.size());
    stats_->set_gets(0);
    stats_->set_values_size(storage_.get_values_size());
  }
//-------------------------------------------------------------------------------------------------

  size_t Storage::queue_size()
  {
    return queue_.size(); 
  }
//-------------------------------------------------------------------------------------------------

  void Storage::pop_key(key_type& key)
  {
    queue_.wait_and_pop(key);  
  }
//-------------------------------------------------------------------------------------------------

  void Storage::push_key(const key_type& key)
  {
    if (queue_.size() < max_queue_size_)
      stats_->set_queue_size(queue_.push(key));
    else
      __LOG_NOTICE << "[Storage::push_key] queue is full";
  }
//-------------------------------------------------------------------------------------------------

  void Storage::expirate()
  {
    storage_.expirate(expirate_size_);
  }
//-------------------------------------------------------------------------------------------------

  void Storage::refresh_stats()
  {
    __LOG_DEBUG << "[Storage::refresh_stats]"; 
    stats_->set_storage_size(storage_.get_storage_size());
    stats_->set_deleted(storage_.get_deleted());
    stats_->set_queue_size(queue_.size());
    stats_->set_gets(storage_.get_gets());
    stats_->set_values_size(storage_.get_values_size());
    stats_->set_keys_size(storage_.get_keys_size());
    stats_->set_updates(storage_.get_updates());
  }
//-------------------------------------------------------------------------------------------------

  void Storage::get_stats(result_type& res)
  {
    refresh_stats();
    stats_->get(res);
  }

  void Storage::get_stats(const std::string& name, result_type& res)
  {
    refresh_stats();
    stats_->get(name, res);
  }
//-------------------------------------------------------------------------------------------------

  bool Storage::get_val(const Command& cmd, result_type& res)
  {
    key_type normalized_key;
    for(Command::arg_iterator x = cmd.arg_begin(); x != cmd.arg_end(); ++x) {
      std::string value;

      bool result = storage_.get(*x, value, normalized_key);
      if (!value.empty())
        res_append(*x, value, res);

      if (!result)
       push_key(normalized_key); 
    }
    //printf("res: %s", res.c_str());
    return true;
  }
//-----------------------------------------------------------------------------------------------
  
  void Storage::res_append(const arg_type& arg, const val_type& value, result_type& result)
  {
    result.append("VALUE ");
    result.append(arg);
    result.append(" 0 "); // хардкод нулевого флага - пожелание заказчика
    result.append(boost::lexical_cast<result_type>(value.length()));
    result.append("\r\n");
    result.append(value);
    result.append("\r\n"); 
  }
//-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------

