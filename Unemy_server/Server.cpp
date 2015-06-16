#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <set>
#include <deque>
#include <string>

#ifndef _WIN32_WINNT         
#define _WIN32_WINNT 0x0500    // 윈도우 2000 이상 지원
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "_message.h"

using namespace std;
using namespace boost::asio::ip;

struct Player {
	int id;
	int size;
	int x, y;
	double vx, vy;
};

class _participant
{
public:
	virtual ~_participant() {}
	virtual void deliver(const _message& msg) = 0;
	int id_;

};

typedef boost::shared_ptr<_participant> _participant_ptr;


class _room
{
public:
	void join(_participant_ptr participant)
	{
		participants_.insert(participant);
		join_count++;

		cout << participant->id_ << " 님이 접속하였습니다." << endl;
	}

	void leave();

	// deliver to all session
	void deliver(_message& msg)
	{
		for each(_participant_ptr participant in participants_)
		{
			if (participant->id_ == msg.get_id())
				continue;

			participant->deliver(boost::ref(msg));
		}
	}

	int get_new_id()
	{
		return join_count;
	}

private:
	set<_participant_ptr> participants_;

	int join_count = 1;
};


class _session
	: public _participant,
	public boost::enable_shared_from_this < _session >
{
public:
	_session(boost::asio::io_service& io_service, _room& room)
		: socket_(io_service),
		room_(room)
	{

	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		id_ = room_.get_new_id();
		room_.join(shared_from_this());

		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), _message::header_length),
			boost::bind(&_session::handle_read_header, shared_from_this(),
			boost::asio::placeholders::error));
	}

	void handle_read_header(const boost::system::error_code& error)
	{
		if (!error)
		{
			read_msg_.decode_header();
			async_read(socket_,
				boost::asio::buffer(read_msg_.body(), read_msg_.get_body_length()),
				boost::bind(&_session::handle_read_body, shared_from_this(),
				boost::asio::placeholders::error));
		}
	}

	void handle_read_body(const boost::system::error_code& error)
	{
		if (!error)
		{
			// deliver to server
			read_msg_.set_id(id_);
			room_.deliver(read_msg_);

			cout << id_ << ": ";
			cout.write(read_msg_.body(), read_msg_.get_body_length()) << endl;

			async_read(socket_,
				boost::asio::buffer(read_msg_.data(), _message::header_length),
				boost::bind(&_session::handle_read_header, shared_from_this(),
				boost::asio::placeholders::error));
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			write_msgs_.pop_front();
			if (!write_msgs_.empty())
			{
				boost::asio::async_write(socket_,
					boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().get_body_length()),
					boost::bind(&_session::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void deliver(const _message& msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().get_body_length()),
				boost::bind(&_session::handle_write, shared_from_this(),
				boost::asio::placeholders::error));
		}
	}

private:
	_room& room_;

	tcp::socket socket_;
	_message read_msg_;
	deque<_message> write_msgs_;

};

typedef boost::shared_ptr<_session> _session_ptr;

class _server
{
public:
	_server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint)
		: io_service_(io_service),
		acceptor_(io_service, endpoint)
	{
		start_accept();
	}

	void start_accept()
	{
		_session_ptr new_session(new _session(io_service_, room_));
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&_server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}

	void handle_accept(_session_ptr session, const boost::system::error_code& error)
	{
		if (!error)
		{
			session->start();
		}
		start_accept();
	}

	//////////////////////////////////////////////////////////////////////////
	_room room_;
private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;


	vector<Player> players_;
};

int main()
{
	boost::asio::io_service io_service;

	tcp::endpoint endpoint(tcp::v4(), atoi("5166"));

	_server server(io_service, endpoint);

	io_service.run();

	//boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

	_message test;
	sprintf(test.data(), "%02d", 1);
	sprintf(test.data() + _message::length_length, "%04d", 10);
	sprintf(test.body(), "%04d", 100);

	char* tr = "1234";
	test.set_body(tr);
	test.encode_header(strlen(tr), 1);

	server.room_.deliver(test);

	return 0;
}