/** 
 * @file cfg.hpp
 * @author Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#ifndef __WAPSTART_SWORDFISH_CFG__H__
#define __WAPSTART_SWORDFISH_CFG__H__
//-------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <vector>
//-------------------------------------------------------------------------------------------------
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
//-------------------------------------------------------------------------------------------------
#include "log_level.hpp"
#include "base_exception.hpp"
//-------------------------------------------------------------------------------------------------
namespace wapstart {
  /**
   *
   */
  class Config {
  public:
    typedef Config                      class_type;
    typedef LogLevel::type              log_level_type;
    typedef std::basic_istream<char>    stream_type;
    typedef boost::property_tree::ptree ptree_type;
    typedef uint16_t                    port_type;

    struct file_not_found: public base_exception {
      typedef boost::errinfo_file_name name;
    };
    
    Config();
    /**
     * @param path Path to a configuration file.
     */
    Config(const std::string &path);
    /**
     * Read a configuration file.
     */ 
    void load(const std::string &path);
    /**
     * Reread the configuration file.
     * @param path Path to a configation file.
     */ 
    void reload();
    /**
     *
     */
    port_type port() const;
    /**
     *
     */
    port_type additional_port() const;
    /**
     *
     */
    size_t workers() const;
    /**
     *
     */
    std::string filler(const std::string&) const;
    /**
     *
     */
    std::string filler_function() const;

    std::vector<std::string> storage_list() const;
    /**
     * Storage size in bytes.
     */
    size_t storage_size(const std::string&) const;
    /**
    * Filler's queue size in bytes.
    */
    size_t filler_queue_size(const std::string&) const;
    /**
     * Size of fill queue
     */
    size_t max_fill_size(const std::string&) const;
    /**
     *
     */
    size_t storage_expirate_size(const std::string&) const;
    /**
     *
     */
    std::vector<std::string>  storage_functions(const std::string&) const;
    /**
     *
     */
    bool storage_default(const std::string&) const;
    /**
     *
     */
    size_t storage_ttl(const std::string&) const;
    /**
     *
     */
    size_t fillers(const std::string&) const;
    /**
     *
     */
    log_level_type log_level() const;
    /**
     *
     */
    bool is_log_file() const;
    /** 
     *
     */
    bool is_log_syslog() const;
    /**
     *
     */
    bool is_log_stdout() const;
    /**
     *
     */
    std::string log_file_path() const;
    /**
     * 
     */
    boost::property_tree::ptree * get_config();
#ifndef __UNIT_TESTING_ON
  private:
#endif // __UNIT_TESTING_ON
    /**
     *
     */
    void load(stream_type &stream);
  private:
    Config(const class_type &);
    void operator =(const class_type &);
    std::string path_; /**< */
    ptree_type  tree_; /**< */
  };
  //-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------
#endif // __WAPSTART_SWORDFISH_CFG__H__
//-------------------------------------------------------------------------------------------------

