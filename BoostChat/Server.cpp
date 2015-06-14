#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <cmath>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "chat_message.hpp"
#include "GameState.h"

using boost::asio::ip::tcp;

/************************************************************************/
/* Declaration                                                          */
/************************************************************************/

/************************************************************************/
/* Game Data                                                            */
/************************************************************************/
Current current;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
public:
	virtual ~chat_participant() {}
	virtual void deliver(const chat_message& msg) = 0;
	int id_;

};

typedef boost::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
public:
	void join(chat_participant_ptr participant)
	{
		participants_.insert(participant);
	}

	void leave(chat_participant_ptr participant, int id)
	{
		participants_.erase(participant);

		// 클라이언트들에게 해당 유저가 떠났다는 메세지를 보냄
		chat_message msg = make_close_msg(id);
		deliver(msg);

		// 서버가 가지고 있는 정보를 업데이트
		remove_player(id);

	}

	// msg에는 보내는이의 id가 반드시 포함되어야 한다.
	void deliver(const chat_message& msg)
	{
		for each (chat_participant_ptr participant in participants_)
		{
			// 메세지를 보낸 클라이언트를 제외
			if (participant->id_ == msg.get_id())
				continue;

			// 전달
			participant->deliver(boost::ref(msg));
		}
	}

	// id에게만 msg를 보냄.
	void deliver_to(const chat_message& msg, int id)
	{
		for each (chat_participant_ptr participant in participants_)
		{
			if (participant->id_ == id) {
				participant->deliver(boost::ref(msg));
				break;
			}
		}
	}

	int size()
	{
		return participants_.size();
	}

	int get_new_id()
	{
		return count++;
	}

	void add_player(int id, int size, int x, int y, double vx, double vy)
	{
		Player p;
		p.id = id;
		p.size = size;
		p.x = x;
		p.y = y;
		p.vx = vx;
		p.vy = vy;

		players.push_back(p);

		std::cout << id << " 님이 접속하였습니다." << std::endl;
	}

	void remove_player(int id)
	{
		auto i = find_if(players.begin(), players.end(), [id](Player e) { return e.id == id; });

		if (i != players.end())
			players.erase(i);

		std::cout << id << " 님이 게임을 떠났습니다." << std::endl;
		std::cout << "남은 수: " << players.size() << std::endl;
	}

	// 클라이언트에게 받은 메세지를 분석해서 사용
	void apply_msg(const chat_message& msg)
	{
		using namespace std;

		int id = msg.get_id();
		int type = msg.get_type();

		switch (type)
		{
		case chat_message::normal:
		{
			auto i = find_if(players.begin(), players.end(), [id](Player e) { return e.id == id; });

			// 메세지에서 정보 획득
			int size = msg.get_body_size();
			int x = msg.get_body_x();
			int y = msg.get_body_y();
			double vx = msg.get_body_vx();
			double vy = msg.get_body_vy();

			//std::cout << id << ": " << x << " " << y << " " << vx << " " << vy << std::endl;

			// 새로운 플레이어
			if (players.empty() || i == players.end())
			{
				add_player(id, size, x, y, vx, vy);
			}
			// 존재하는 플레이어 업데이트
			else
			{
				(*i).size = size;
				(*i).x = x;
				(*i).y = y;
				(*i).vx = vx;
				(*i).vy = vy;
			}

			collision_detection(msg);

			break;
		}
		}

	}

	chat_message make_normal_msg(Player p)
	{
		chat_message normal_msg;
		normal_msg.set_id(p.id);
		normal_msg.set_type(chat_message::normal);
		normal_msg.set_body_size(p.size);
		normal_msg.set_body_x(p.x);
		normal_msg.set_body_y(p.y);
		normal_msg.set_body_vx(p.vx);
		normal_msg.set_body_vy(p.vy);
		normal_msg.encode_header();

		return normal_msg;
	}

	chat_message make_close_msg(int sender_id)
	{
		chat_message close_msg;
		close_msg.set_id(sender_id);
		close_msg.set_type(chat_message::close);
		close_msg.encode_header();

		return close_msg;
	}

	chat_message make_collision_msg(int sender_id, int absorber_id, int target_id)
	{
		chat_message collision_msg;
		collision_msg.set_id(sender_id);
		collision_msg.set_type(chat_message::collision);
		collision_msg.set_absorber_id(absorber_id);
		collision_msg.set_target_id(target_id);
		collision_msg.encode_header();

		return collision_msg;
	}

	void collision_detection(chat_message msg)
	{
		int sender_id = msg.get_id();
		Player sender = get_player(sender_id);

		for each (Player p in players)
		{
			if (p.id == sender_id) continue;

			if (is_overlap(sender, p))
			{
				std::cout << "overlapped!!" << std::endl;
				int absorber_id, target_id;
				if (is_win(sender, p)) {
					absorber_id = sender_id;
					target_id = p.id;
				}
				else {
					absorber_id = p.id;
					target_id = sender_id;
				}

				chat_message msg = make_collision_msg(sender_id, absorber_id, target_id);
				std::cout << absorber_id << "가 " << target_id << "를 먹음" << std::endl;
				deliver_to(msg, absorber_id);
				deliver_to(msg, target_id);
			}
		}
	}

	bool is_win(Player sender, Player p)
	{
		if (sender.size > p.size) {
			return true;
		}
		else if (sender.size < p.size) {
			return false;
		}
		else {
			int v1 = get_velocity_size(sender);
			int v2 = get_velocity_size(p);

			if (v1 > v2)
				return true;
			else
				return false;
		}
	}

	double get_velocity_size(Player p)
	{
		return sqrt(pow(p.vx, 2.0) + pow(p.vy, 2.0));
	}

	bool is_overlap(Player p1, Player p2)
	{
		return (player_distance(p1, p2) < p1.size + p2.size);
	}

	double player_distance(Player p1, Player p2)
	{
		return sqrt(pow(p1.x - p2.x, 2.0) + pow(p1.y - p2.y, 2.0));
	}

	Player get_player(int id)
	{
		auto found = std::find_if(players.begin(), players.end(), [id](Player p){ return p.id == id; });
		if (found == players.end()) {
			std::cout << "player not found" << std::endl;
			exit(1);
		}
		else
			return (*found);
	}

	int get_player_number()
	{
		return players.size();
	}

private:
	std::set<chat_participant_ptr> participants_;
	std::vector<Player> players;

	int count = 0;
};

//----------------------------------------------------------------------

class chat_session
	: public chat_participant,
	public boost::enable_shared_from_this < chat_session >
{
public:
	chat_session(boost::asio::io_service& io_service, chat_room& room)
		: socket_(io_service),
		deadline_(io_service),
		room_(room)
	{
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		room_.join(shared_from_this());
		id_ = room_.get_new_id();

		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), chat_message::header_length),
			boost::bind(
			&chat_session::handle_read_header, shared_from_this(),
			boost::asio::placeholders::error));
	}

	void deliver(const chat_message& msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(write_msgs_.front().data(),
				write_msgs_.front().length()),
				boost::bind(&chat_session::handle_write, shared_from_this(),
				boost::asio::placeholders::error));
		}
	}

	void handle_read_header(const boost::system::error_code& error)
	{
		if (!error && read_msg_.decode_header())
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
				boost::bind(&chat_session::handle_read_body, shared_from_this(),
				boost::asio::placeholders::error));
		}
		else
		{
			room_.leave(shared_from_this(), id_);
		}
	}

	void handle_read_body(const boost::system::error_code& error)
	{
		if (!error)
		{
			// 메세지를 보낸 id를 확인.
			read_msg_.set_id(id_);

			// 받은 메세지로 서버의 정보를 업데이트한다.
			room_.apply_msg(read_msg_);

			// 메세지를 다른 클라이언트들에게 전달한다.
			room_.deliver(read_msg_);

			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.data(), chat_message::header_length),
				boost::bind(&chat_session::handle_read_header, shared_from_this(),
				boost::asio::placeholders::error));
		}
		else
		{
			room_.leave(shared_from_this(), id_);
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
					boost::bind(&chat_session::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
			}
		}
		else
		{
			room_.leave(shared_from_this(), id_);
		}
	}

	int get_id()
	{
		return id_;
	}

private:
	tcp::socket socket_;

	bool stopped_;
	boost::asio::deadline_timer deadline_;

	chat_room& room_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};

typedef boost::shared_ptr<chat_session> chat_session_ptr;

//----------------------------------------------------------------------

class chat_server
{
public:
	chat_server(boost::asio::io_service& io_service,
		const tcp::endpoint& endpoint)
		: io_service_(io_service),
		acceptor_(io_service, endpoint)
	{
		start_accept();
	}

	void start_accept()
	{
		chat_session_ptr new_session(new chat_session(io_service_, room_));
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&chat_server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}

	void handle_accept(chat_session_ptr session,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			session->start();
		}

		start_accept();
	}

	chat_room room_;

private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------
const int ai_enemy_appear_time = 10;
const int window_width = 800;
const int window_height = 600;

void make_enemy(const boost::system::error_code& /*e*/,
	boost::asio::deadline_timer* t, chat_server_ptr server)
{
	if (server->room_.get_player_number() != 0)
	{
		srand((unsigned int)time(NULL));
		std::cout << " test tstest" << std::endl;

		int id = server->room_.get_new_id();
		int size = 10;
		int x = rand() % window_width;
		int y = rand() % window_height;
		Player p = { id, size, x, y };

		chat_message msg = server->room_.make_normal_msg(p);

		server->room_.deliver(msg);
	}

	t->expires_at(t->expires_at() 
		+ boost::posix_time::seconds(ai_enemy_appear_time));

	t->async_wait(boost::bind(make_enemy,
		boost::asio::placeholders::error, t, server));
}

void move_enemy(const boost::system::error_code& /*2*/)
{

}

//----------------------------------------------------------------------

int main()
{
	try
	{
		boost::asio::io_service io_service;

		chat_server_list servers;

		{
			using namespace std; // For atoi.
			tcp::endpoint endpoint(tcp::v4(), atoi("5166"));
			chat_server_ptr server(new chat_server(io_service, endpoint));
			servers.push_back(server);
		}

		{
			int count = 0;
			boost::asio::deadline_timer t(io_service, boost::posix_time::seconds(1));
			t.async_wait(boost::bind(make_enemy,
				boost::asio::placeholders::error, &t, servers.back()));
		}

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

