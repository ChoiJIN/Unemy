#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "GameState.h"
#include "chat_message.hpp"

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
	chat_client(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service),
		socket_(io_service)
	{
		boost::asio::async_connect(socket_, endpoint_iterator,
			boost::bind(&chat_client::handle_connect, this,
			boost::asio::placeholders::error));
	}

	void write(const chat_message& msg)
	{
		io_service_.post(boost::bind(&chat_client::do_write, this, msg));
	}

	void close()
	{
		io_service_.post(boost::bind(&chat_client::do_close, this));
	}

	std::vector<Enemy> get_enemies()
	{
		return enemies;
	}

private:

	void handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.data(), chat_message::header_length),
				boost::bind(&chat_client::handle_read_header, this,
				boost::asio::placeholders::error));
		}
	}

	bool read_id(int id)
	{
			return true;
	}



	void read_enemy(int id, const char* body)
	{
		using namespace std;
		char size[5] = "";
		char x[5] = "";
		char y[5] = "";
		int offset = 3;

		auto i = find_if(enemies.begin(), enemies.end(), [id](Enemy e) { return e.id == id; });

		// 새로운 적
		if (enemies.empty() || i == enemies.end())
		{
			Enemy e;

			e.id = id;
			strncat(size, body, offset);
			e.size = atoi(size);
			strncat(x, body + offset, offset);
			e.x = atoi(x);
			strncat(y, body + 2 * offset, offset);
			e.y = atoi(y);

			enemies.push_back(e);
		}
		// 존재하는 적
		else {
			strncat(size, body, offset);
			(*i).size = atoi(size);
			strncat(x, body + offset, offset);
			(*i).x = atoi(x);
			strncat(y, body + 2 * offset, offset);
			(*i).y = atoi(y);
		}
	}

	void handle_read_header(const boost::system::error_code& error)
	{
		if (!error && read_msg_.decode_header())
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
				boost::bind(&chat_client::handle_read_body, this,
				boost::asio::placeholders::error));
		}
		else
		{
			if (mycount > 3)
			{
				do_close();
			}
			if (!read_msg_.decode_header()) {
				std::cout << "not successed... ";
				mycount++;
			}
		}
	}

	void handle_read_body(const boost::system::error_code& error)
	{
		if (!error)
		{
			//mtx_.lock();
			read_enemy(read_msg_.get_id(), read_msg_.body());
			//mtx_.unlock();

			std::cout.write(read_msg_.body(), read_msg_.body_length());
			std::cout << "\n";
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.data(), chat_message::header_length),
				boost::bind(&chat_client::handle_read_header, this,
				boost::asio::placeholders::error));

		}
		else
		{
			do_close();
		}
	}

	void do_write(chat_message msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(write_msgs_.front().data(),
				write_msgs_.front().length()),
				boost::bind(&chat_client::handle_write, this,

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
					boost::asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
					boost::bind(&chat_client::handle_write, this,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			do_close();
		}
	}

	void do_close()
	{
		socket_.close();
		std::cout << "서버와 접속이 끊겼습니다." << std::endl;
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
	std::vector<Enemy> enemies;

	boost::mutex mtx_;

	int mycount = 0;
};

#endif
