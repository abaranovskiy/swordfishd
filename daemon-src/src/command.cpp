/**
 * @file server.cpp
 * @author  Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#include "command.hpp"
//-------------------------------------------------------------------------------------------------
namespace wapstart {
  namespace privacy {
    const char _spacebar = ' ';
    //---------------------------------------------------------------------------------------------
    const std::string _unknown_command = "unknown";
    //---------------------------------------------------------------------------------------------
  } // namespace privacy
  //-----------------------------------------------------------------------------------------------
  Command::Command(const std::string &cmd_line)
  {
    parse_command(cmd_line);
  }
  //-----------------------------------------------------------------------------------------------
  void Command::parse_command(const std::string &cmd_line)
  {
    boost::split(args_, cmd_line, boost::is_any_of(" "), boost::token_compress_on);
  }
  //-----------------------------------------------------------------------------------------------
  const std::string &Command::name() const
  {
    return args_.size() > 0 ? args_[0] : privacy::_unknown_command; 
  }
  void Command::name(const std::string& name)
  {
      args_[0] = name;
  }
  //-----------------------------------------------------------------------------------------------
  Command::size_type Command::argc() const
  {
    return args_.size() > 0 ? args_.size() - 1 : 0;
  }
  //-----------------------------------------------------------------------------------------------
  const std::string &Command::operator [](size_type x) const
  {
    return args_[x + 1];
  }
  //-----------------------------------------------------------------------------------------------
  const std::string &Command::at(size_type x) const
  {
    return args_.at(x + 1);
  }
  //-----------------------------------------------------------------------------------------------
  Command::arg_iterator Command::arg_begin() const
  {
    return argc() ? args_.begin() + 1 : args_.end();
  }
  //-----------------------------------------------------------------------------------------------
  Command::arg_iterator Command::arg_end() const
  {
    return args_.end();
  }
  void Command::insert(const std::string& arg)
  {
      args_.push_back(arg);
  }

  //-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------

