#include "_client.h"

void _client::handle_connect(const boost::system::error_code& error)
{
	if (!error)
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), _message::header_length),
			boost::bind(&_client::handle_read_header, this,
			boost::asio::placeholders::error));
	}
}

void _client::handle_read_header(const boost::system::error_code& error)
{
	if (!error)
	{
		read_msg_.decode_header();

		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.get_length()),
			boost::bind(&_client::handle_read_body, this,
			boost::asio::placeholders::error));
	}
	else
	{
		do_close();
	}
}

void _client::handle_read_body(const boost::system::error_code& error)
{
	if (!error)
	{
		parsing_player(read_msg_);
		std::cout.write(read_msg_.body(), read_msg_.get_length());
		std::cout << "\n";
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), _message::header_length),
			boost::bind(&_client::handle_read_header, this,
			boost::asio::placeholders::error));
	}
	else
	{
		do_close();
	}
}

void _client::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		if (!write_msgs_.empty())
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(write_msgs_.front().data(),
				write_msgs_.front().get_length()),
				boost::bind(&_client::handle_write, this,
				boost::asio::placeholders::error));
			write_msgs_.pop_front();
		}
	}
	else
	{
		do_close();
	}
}


void _client::parsing_player(_message msg)
{
	int id = msg.get_id();
	int size = msg.get_size();
	int x = msg.get_x();
	int y = msg.get_y();

	Player p = { id, size, x, y };

	auto i = find(players.begin(), players.end(), p);

	if (i != players.end())
	{
		(*i).size = size;
		(*i).x = x;
		(*i).y = y;
	}
	else
	{
		players.push_back(p);
	}
}


//////////////////////////////////////////////////////////////////////////
void _client::write(const _message& msg)
{
	io_service_.post(boost::bind(&_client::do_write, this, msg));
}

void _client::do_write(_message msg)
{
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(msg);
	if (!write_in_progress)
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().data(),
			write_msgs_.front().get_length()),
			boost::bind(&_client::handle_write, this,

			boost::asio::placeholders::error));
	}
}

void _client::do_close()
{
	socket_.close();
	std::cout << "서버와 접속이 끊겼습니다." << std::endl;
}

