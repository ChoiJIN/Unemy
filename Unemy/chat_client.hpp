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
			do_close();
		}
	}

	void handle_read_body(const boost::system::error_code& error)
	{
		if (!error)
		{
			read_msg(read_msg_);

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

	void read_msg(chat_message& msg)
	{
		using namespace std;

		int id = msg.get_id();
		int type = msg.get_type();


		auto i = find_if(enemies.begin(), enemies.end(), [id](Enemy e) { return e.id == id; });

		switch (type)
		{
		case chat_message::normal:
		{
			char size[4+1] = "";
			char x[4+1] = "";
			char y[4+1] = "";
			char vx[4 + 1] = "";
			char vy[4 + 1] = "";

			// 새로운 적
			if (enemies.empty() || i == enemies.end())
			{
				Enemy e;

				e.id = id;
				strncat(size, msg.body(), body_offset);
				e.size = atoi(size);
				strncat(x, msg.body() + body_offset, body_offset);
				e.x = atoi(x);
				strncat(y, msg.body() + 2 * body_offset, body_offset);
				e.y = atoi(y);
				strncat(vx, msg.body() + 3 * body_offset, body_offset);
				e.vx = atoi(vx) / 100.0;
				strncat(vy, msg.body() + 4 * body_offset, body_offset);
				e.vy = atoi(vy) / 100.0;

				enemies.push_back(e);
			}
			// 존재하는 적
			else {
				strncat(size, msg.body(), body_offset);
				(*i).size = atoi(size);
				strncat(x, msg.body() + body_offset, body_offset);
				(*i).x = atoi(x);
				strncat(y, msg.body() + 2 * body_offset, body_offset);
				(*i).y = atoi(y);
				strncat(vx, msg.body() + 3 * body_offset, body_offset);
				(*i).vx = atoi(vx) / 100.0;
				strncat(vy, msg.body() + 4 * body_offset, body_offset);
				(*i).vy = atoi(vy) / 100.0;
			}
			break;
		}

		case chat_message::collision:
		{

			break;
		}

		case chat_message::close:
		{
			enemies.erase(i);
			break;
		}
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
	int body_offset = 4;
	int mycount = 0;

	// Game
	std::vector<Enemy> enemies;
	int my_id_;

};

#endif
