//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class chat_message
{
public:
	enum {
		size_index = 0, size_length = 4,
		id_index = size_length, id_length = 1,
		type_index = size_length + id_length, type_length = 1,
		header_length = size_length + id_length + type_length
	};

	// type
	enum {
		normal, close, collision
	};

	enum { max_body_length = 512 };

	chat_message()
		: body_length_(0),
		id_(0),
		type_(normal)
	{
	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	size_t length() const
	{
		return header_length + body_length_;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	size_t body_length() const
	{
		return body_length_;
	}

	void body_length(size_t new_length)
	{
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	// header를 분석해서 데이터를 넣는다.
	// 어떤 클라이언트가 보냈는지 id 확인
	bool decode_header()
	{
		using namespace std; // For strncat and atoi.

		// size
		char size[size_length + 1] = "";
		strncat_s(size, data_ + size_index, size_length);
		body_length_ = atoi(size);

		// id
		char id[id_length + 1] = "";
		strncat_s(id, data_ + id_index, id_length);
		id_ = atoi(id);

		// type
		char type[type_length + 1] = "";
		strncat_s(type, data_ + type_index, type_length);
		type_ = atoi(type);

		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		using namespace std; // For sprintf and memcpy.
		char header[header_length + 1] = "";

		sprintf(header + size_index, "%04d", body_length_);
		sprintf(header + id_index, "%01d", id_);
		sprintf(header + type_index, "%01d", type_);

		memcpy(data_, header, header_length);
	}

	//////////////////////////////////////////////////////////////////////////
	// Header
	void set_id(int id)
	{
		id_ = id;
		data_[id_index] = id + '0';
	}

	int get_id() const
	{
		return id_;
	}

	void set_type(int type)
	{
		type_ = type;
		data_[type_index] = type;
	}

	int get_type()
	{
		return type_;
	}

	//////////////////////////////////////////////////////////////////////////
	// Body
	int get_body_size()
	{
		char size[4 + 1] = "";
		strncat(size, body(), body_offset_);

		return atoi(size);
	}

	int get_body_x()
	{
		char y[4 + 1] = "";
		strncat(y, body() + body_offset_, body_offset_);

		return atoi(y);
	}

	int get_body_y()
	{
		char y[4 + 1] = "";
		strncat(y, body() + 2 * body_offset_, body_offset_);

		return atoi(y);
	}

	double get_body_vx()
	{
		char vx[4 + 1] = "";
		strncat(vx, body() + 3 * body_offset_, body_offset_);

		return (atoi(vx) / 100.0);
	}

	double get_body_vy()
	{
		char vy[4 + 1] = "";
		strncat(vy, body() + 4 * body_offset_, body_offset_);

		return (atoi(vy) / 100.0);
	}

	int get_absorber_id()
	{
		char id[1 + 1] = "";
		strncat(id, body(), id_length);

		return atoi(id);
	}

	void set_absorber_id(int absorber_id)
	{
		body()[0] = (int)absorber_id;
	}

	int get_target_id()
	{
		char id[1 + 1] = "";
		strncat(id, body() + 1, id_length);

		return atoi(id);
	}

	void set_target_id(int target_id)
	{
		body()[1] = (int)target_id;
	}

private:
	char data_[header_length + max_body_length];
	// header
	int id_;
	int type_;
	int body_length_;

	// body
	const int body_offset_ = 4;
};

#endif // CHAT_MESSAGE_HPP