/**
 * @file server.cpp
 * @author  Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
//-------------------------------------------------------------------------------------------------
#include "server.hpp"
#include "logger.hpp"
//-------------------------------------------------------------------------------------------------
namespace wapstart {
    Server::Server(service_type &service,
                 StorageController*& storage_controller,
                 port_type     port, 
                 port_type     additional_port, 
                 size_type     workers): service_(service),
                                         storage_controller_(storage_controller),
                                         workers_(workers),
                                         worker_(Worker::create(service_, storage_controller_)),
                                         worker2_(Worker::create(service_, storage_controller_)),
                                         acceptor_(service_, endpoint_type(
                                           boost::asio::ip::tcp::v4(), port)),
                                         additional_acceptor_(service_, endpoint_type(
                                           boost::asio::ip::tcp::v4(), additional_port))
    {
        stop_ = false;
        __LOG_DEBUG << "Creating server";
    }
    //-----------------------------------------------------------------------------------------------
    Server::~Server()
    {
        stop();

        __LOG_DEBUG << "Delete server";
    }
    void Server::reload()
    {
        acceptor_.async_accept(worker_->socket(), boost::bind(
            &Server::on_accept, this, boost::asio::placeholders::error));

        additional_acceptor_.async_accept(worker2_->socket(), boost::bind(
            &Server::on_additional_accept, this, boost::asio::placeholders::error));
    }
    //-----------------------------------------------------------------------------------------------
    void Server::on_accept(const error_code_type &error)
    {
        if(!error) {
            worker_->run();
        } else {
            __LOG_CRIT << "can't accept new request. Sleeping a bit";
            __LOG_CRIT << "acceptor said: " <<  error.message();
        
            boost::this_thread::sleep(
                boost::posix_time::milliseconds(WAPSTART_SWORDFISH_SLEEP_ON_ERROR_MS)
            );

        }
        if (!stop_) {
            worker_ = Worker::create(service_, storage_controller_);
            acceptor_.async_accept(worker_->socket(), boost::bind(
                &Server::on_accept, this, boost::asio::placeholders::error));
        }
    }
    //-----------------------------------------------------------------------------------------------
    void Server::on_additional_accept(const error_code_type &error)
    {
        if(!error) {
            worker2_->run();
        } else {
            __LOG_CRIT << "can't accept new request. Sleeping a bit";
            __LOG_CRIT << "additional_acceptor said: " <<  error.message();
        
            boost::this_thread::sleep(
                boost::posix_time::milliseconds(WAPSTART_SWORDFISH_SLEEP_ON_ERROR_MS)
            );
        }

        if (!stop_) {
            worker2_ = Worker::create(service_, storage_controller_);
            additional_acceptor_.async_accept(worker2_->socket(), boost::bind(
                &Server::on_additional_accept, this, boost::asio::placeholders::error));
        }
  }
  //-----------------------------------------------------------------------------------------------
    void Server::run()
    {
        __LOG_DEBUG << "Server::run started";
        typedef boost::ptr_vector<boost::thread> thread_vector;

        service_.reset();

        thread_vector tv;
        for(size_type x = 0; x < workers_; ++x)
            tv.push_back(new boost::thread(
                boost::bind(&service_type::run, &service_)));

        for(size_type x = 0; x < tv.size(); ++x)
            tv[x].join();
        __LOG_DEBUG << "Server::run finished";
    }
    //-----------------------------------------------------------------------------------------------
    void Server::stop()
    {
        if (stop_)
            return;

        stop_ = true;
        __LOG_DEBUG << "Stop acceptors";
        acceptor_.close();
        additional_acceptor_.close();
    }
    //-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------

