/**
 * @file server.hpp
 * @author Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#ifndef __WAPSTART_SWORDFISH_SERVER__H__
#define __WAPSTART_SWORDFISH_SERVER__H__
//-------------------------------------------------------------------------------------------------
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

//-------------------------------------------------------------------------------------------------
#include "worker.hpp"
#include "storage.hpp"
#include "abstract_filler.hpp"
#include "storage_controller.hpp"
//-------------------------------------------------------------------------------------------------

#define WAPSTART_SWORDFISH_SLEEP_ON_ERROR_MS 500

namespace wapstart {
  class Server {
  public:
    typedef Server                           class_type;
    typedef boost::asio::io_service          service_type;
    typedef boost::asio::ip::tcp::socket     socket_type;
    typedef boost::asio::ip::tcp::endpoint   endpoint_type;
    typedef boost::asio::ip::tcp::acceptor   acceptor_type;
    typedef boost::system::error_code        error_code_type;
    typedef int                              port_type;
    typedef std::size_t                      size_type; 
    /**
     * @param storage Reference to the storage object
     * @param port    Port to listen on
     * @param workers Number of workers in the worker pool 
     */
    Server(service_type &service,
           StorageController*&,
           port_type     port,
           port_type     additional_port,
           size_type     workers = 10); 
    /**
     *  
     */
    ~Server();  
    /**
     *
     */
    void run();
    /**
     *
     */
    void stop();
    void reload();
  private:
    Server(const class_type &);
    void operator =(const class_type &);
    service_type        &service_;
    StorageController*  storage_controller_;
    acceptor_type        acceptor_;    
    acceptor_type        additional_acceptor_;
    size_type            workers_;
    Worker::pointer_type worker2_;
    Worker::pointer_type worker_;
    /**
     *
     */
    void on_accept(const error_code_type &error);    
    void on_additional_accept(const error_code_type &error);

    bool stop_;
  };
  //-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------
#endif // __WAPSTART_SWORDFISH_SERVER__H__
//-------------------------------------------------------------------------------------------------

