/**
 * @file worker.cpp
 * @author  Litvinova Alina
 */
//-------------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp>


//-------------------------------------------------------------------------------------------------
#include "worker.hpp"
#include "command.hpp"
#include "logger.hpp"
#include "log_level.hpp"
#include <ctime>

#include "timer.hpp"

//-------------------------------------------------------------------------------------------------
namespace wapstart {
Worker::Worker(service_type &service,
               StorageController *&storage_controller): service_(service),
    strand_(service),
    socket_(service),
    storage_controller_(storage_controller)
{
}
//-------------------------------------------------------------------------------------------------
Worker::pointer_type Worker::create(service_type &service,
                                    StorageController *&storage_controller)
{
    return pointer_type(new Worker(service, storage_controller));
}
//-------------------------------------------------------------------------------------------------
void Worker::run()
{
    socket_.async_read_some(boost::asio::buffer(buffer_),
      strand_.wrap(boost::bind(&Worker::on_read, shared_from_this(),
                   boost::asio::placeholders::error,
                   boost::asio::placeholders::bytes_transferred)));
/*
    try {
        boost::asio::streambuf request;

        while(true) {
            boost::asio::read_until(socket_, request, "\r\n");

            std::istream command_stream(&request);

            std::string command_line;
            while(std::getline(command_stream, command_line)) {
                boost::trim_if(command_line, boost::is_any_of("\r\n"));

                Command cmd(command_line);

                if (cmd.name() == "quit")
                {
                    socket_.close();
                    break;
                }
                std::string message;
                storage_controller_->processRequest(cmd, message);
                auto elapsed = std::chrono::high_resolution_clock::now() - start;
                long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
                __LOG_DEBUG << "Storage controller pricessing takes " << microseconds << " ms"


                if (message.empty()) message = "END\r\n";

                boost::asio::write(socket_, boost::asio::buffer(message), boost::asio::transfer_all());
            }
        }
    }
    catch(...)
    {
        //printf("[Worker::_do] Exception\n");
    }
    shared_from_this().reset();*/
}
void Worker::stop()
{
    socket_.close();
}

//-----------------------------------------------------------------------------------------------
void Worker::on_read(const boost::system::error_code &error,
                     size_type                        bytes_transfered)
{
    if(!error) {
        tokenizer_.append(std::string(buffer_.begin(),
                                      buffer_.begin() + bytes_transfered));



        for(CommandTokenizer::iterator x =
                    tokenizer_.begin(); x != tokenizer_.end(); ++x) {

            Timer timer("Storage processed request");
            Command command(*x);

            if(command.name() == "quit") {
                __LOG_DEBUG << "Client has sent a quit-command";
                socket_.close();
                return;
            }

            std::string response;

            if(command.name() == "log_level") {
                if (command.argc() == 1) {
                    LogLevel::type level = LogLevel::text2level(command.at(0));
                    logger_set_severity_level(level);

                    __LOG_INFO << "Client change LogLevel to " << level;

                    response = "OK\r\n";
                } else
                    response = "ERROR\r\n";

                response.append("END\r\n");
            } else
                storage_controller_->processRequest(command, response);


            if(response.empty())
                response = "END\r\n";

            /*
            try {
                boost::asio::write(socket_, boost::asio::buffer(response), boost::asio::transfer_all());
            } catch (...) {
                __LOG_INFO << "write failed";
            }*/

            timer.show();

            boost::asio::async_write(socket_, boost::asio::buffer(response),
              strand_.wrap(boost::bind(&Worker::on_write, shared_from_this(),
                           boost::asio::placeholders::error)));

        }

        socket_.async_read_some(boost::asio::buffer(buffer_),
                                strand_.wrap(boost::bind(&Worker::on_read, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred)));
    }
    else {
        __LOG_DEBUG << "Client closed the connection: " << error;
    }
}
//-----------------------------------------------------------------------------------------------
void Worker::on_write(const boost::system::error_code &error)
{
    if(error) {
        __LOG_ERROR << "Failed to respond to the client: " << error;
    }
    else
        __LOG_DEEP_DEBUG << "Response has sent to the client";
}
//-----------------------------------------------------------------------------------------------
} // namespace wapstart
//-------------------------------------------------------------------------------------------------

