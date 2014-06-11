/** 
 * @file cfg.cpp
 * @author Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#include <fstream>
//-------------------------------------------------------------------------------------------------
#include <boost/property_tree/ini_parser.hpp>
//-------------------------------------------------------------------------------------------------
#include "cfg.hpp"
//-------------------------------------------------------------------------------------------------
namespace wapstart {
  Config::Config() {}
  //-----------------------------------------------------------------------------------------------
  Config::Config(const std::string &path): path_(path) { reload(); }
  //-----------------------------------------------------------------------------------------------
  void Config::load(const std::string &path) { path_ = path; reload(); }
  //-----------------------------------------------------------------------------------------------
  void Config::reload() 
  {
    std::ifstream stream(path_.c_str());
    if(stream) {
      load(stream);
    }
    else {
      throw file_not_found() << file_not_found::name(path_);
    }
  }
  //-----------------------------------------------------------------------------------------------
  void Config::load(stream_type &stream)
  {
    boost::property_tree::read_ini(stream, tree_);
  }
  //-----------------------------------------------------------------------------------------------
  Config::port_type Config::port() const 
  {
    return tree_.get<port_type>("general.port", 9696);
  }
  //-----------------------------------------------------------------------------------------------
  Config::port_type Config::additional_port() const 
  {
    return tree_.get<port_type>("general.additional_port", 9697);
  }
  //-----------------------------------------------------------------------------------------------
  size_t Config::workers() const
  {
    return tree_.get<size_t>("general.workers", 3);
  }
  //-----------------------------------------------------------------------------------------------
  std::vector<std::string> Config::storage_list() const
  {
      std::vector<std::string> strs;
      std::string a = tree_.get<std::string>("general.storage_list", "default");
      boost::split(
        strs,
        a,
        boost::is_any_of(";,")
      );

      return strs;
  }

    std::vector<std::string>  Config::storage_functions(const std::string& storage_id) const
    {
        std::vector<std::string> strs;
        std::string a = tree_.get<std::string>("storage_" + storage_id + ".functions", "default");
        boost::split(
            strs,
            a,
            boost::is_any_of(";,")
        );

        return strs;
    }

    std::string Config::filler(const std::string& storage_id) const
    {
        return tree_.get<std::string>("storage_" + storage_id + ".filler");
    }

    bool Config::storage_default(const std::string& storage_id) const
    {
        return tree_.get<std::string>("storage_" + storage_id + ".default", "off") == "on";
    }

  //-----------------------------------------------------------------------------------------------
  size_t Config::fillers(const std::string& storage_id) const
  {
    return tree_.get<size_t>("storage_" + storage_id + ".fillers", 10);
  }
  //-----------------------------------------------------------------------------------------------
  std::string Config::filler_function() const
  {
    return tree_.get<std::string>("general.filler_function", "get_values_from_outside");
  }
  //-----------------------------------------------------------------------------------------------
  size_t Config::storage_size(const std::string& storage_id) const
  {
    return tree_.get<size_t>("storage_" + storage_id + ".storage_size", 1000);
  }
  //-----------------------------------------------------------------------------------------------
  size_t Config::filler_queue_size(const std::string& storage_id) const
  {
    return tree_.get<size_t>("storage_" + storage_id + ".filler_queue_size", 1048576 /* 1mb by default */);
  }  
  //-----------------------------------------------------------------------------------------------
  size_t Config::max_fill_size(const std::string& storage_id) const
  {
    return tree_.get<size_t>("storage_" + storage_id + ".max_fill_size", 10 /* 10 by default */);
  }
  //-----------------------------------------------------------------------------------------------
  size_t Config::storage_expirate_size(const std::string& storage_id) const
  {
    return tree_.get<size_t>("storage_" + storage_id + ".storage_expirate_size", storage_size(storage_id) >> 3);
  }
  //-----------------------------------------------------------------------------------------------
  size_t Config::storage_ttl(const std::string& storage_id) const
  {
    return tree_.get<size_t>("storage_" + storage_id + ".storage_ttl", 10);
  }
  //-----------------------------------------------------------------------------------------------
  Config::log_level_type Config::log_level() const 
  {
    return LogLevel::text2level(tree_.get<std::string>("logging.log_level", "DEBUG"));
  }
  //-----------------------------------------------------------------------------------------------
  bool Config::is_log_file() const 
  {
    return tree_.get<std::string>("logging.log_file", "off") == "on";
  }
  //-----------------------------------------------------------------------------------------------
  bool Config::is_log_syslog() const 
  {
    return tree_.get<std::string>("logging.log_syslog", "off") == "on";
  }
  //-----------------------------------------------------------------------------------------------
  bool Config::is_log_stdout() const 
  {
    return tree_.get<std::string>("logging.log_stdout", "off") == "on";
  }
  //-----------------------------------------------------------------------------------------------
  std::string Config::log_file_path() const 
  {
    return tree_.get<std::string>("logging.log_file_path", "swordfish.log");
  }
  //-----------------------------------------------------------------------------------------------
  boost::property_tree::ptree * Config::get_config()
  {
    return &tree_;
  }
  //-----------------------------------------------------------------------------------------------
} // namespace SU13
//-------------------------------------------------------------------------------------------------



