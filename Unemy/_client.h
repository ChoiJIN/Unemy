#pragma once

#include <iostream>
#include <deque>

#ifndef _WIN32_WINNT         
#define _WIN32_WINNT 0x0500    // 윈도우 2000 이상 지원
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "_message.h"

using namespace std;
using namespace boost::asio::ip;

class _client
{
public:
	_client(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service),
		socket_(io_service)
	{
		boost::asio::async_connect(socket_, endpoint_iterator,
			boost::bind(&_client::handle_connect, this,
			boost::asio::placeholders::error));
	}

	void write(const _message& msg);

	void do_write(_message msg);

	void do_close();

private:

	void handle_connect(const boost::system::error_code& error);

	void handle_read_header(const boost::system::error_code& error);
	
	void handle_read_body(const boost::system::error_code& error);

	void handle_write(const boost::system::error_code& error);

	//////////////////////////////////////////////////////////////////////////



private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;

	deque<_message> write_msgs_;

	_message read_msg_;
};

