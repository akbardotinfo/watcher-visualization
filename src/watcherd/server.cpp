//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modified by Geoff Lawler, SPARTA inc.

#include "server.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

using namespace watcher;

INIT_LOGGER(watcher::server, "server");

server::server(
        const std::string& address, 
        const std::string& port, 
        // boost::shared_ptr<request_handler> &handler,
        std::size_t thread_pool_size) :
    thread_pool_size_(thread_pool_size),
    acceptor_(io_service_)
    // new_connection_(new serverConnection(io_service_, request_handler_)),
    // request_handler_(boost::shared_ptr<request_handler>(new request_handler))
{
    TRACE_ENTER();

    request_handler_=boost::shared_ptr<request_handler>(new request_handler);
    new_connection_=server_connection_ptr(new serverConnection(io_service_, request_handler_));

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(address, port);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    acceptor_.async_accept(
            new_connection_->socket(),
            boost::bind(
                &server::handle_accept, 
                this, 
                boost::asio::placeholders::error));

    TRACE_EXIT();
}

void server::run()
{
    TRACE_ENTER();
    // Create a pool of threads to run all of the io_services.
    std::vector<boost::shared_ptr<boost::thread> > threads;
    for (std::size_t i = 0; i < thread_pool_size_; ++i)
    {
        boost::shared_ptr<boost::thread> thread(
                new boost::thread(boost::bind( &boost::asio::io_service::run, &io_service_)));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
        threads[i]->join();

    TRACE_EXIT();
}

void server::stop()
{
    TRACE_ENTER();
    io_service_.stop();
    TRACE_EXIT();
}

void server::handle_accept(const boost::system::error_code& e)
{
    TRACE_ENTER();
    if (!e)
    {
        new_connection_->start();
        new_connection_.reset(new serverConnection(io_service_, request_handler_));
        acceptor_.async_accept(
                new_connection_->socket(),
                boost::bind(
                    &server::handle_accept, 
                    this,
                    boost::asio::placeholders::error));
    }
    TRACE_EXIT();
}

