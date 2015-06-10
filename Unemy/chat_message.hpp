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
		normal, close
	};

	enum { max_body_length = 512 };

	chat_message()
		: body_length_(0),
		id_(0),
		type_(0)
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

	void set_id(int id)
	{
		using namespace std;
		data_[size_length] = id + '0';
	}

	int get_id()
	{
		return id_;
	}

	int get_type()
	{
		return type_;
	}

private:
	char data_[header_length + max_body_length];
	int id_;
	int type_;
	size_t body_length_;
};

#endif // CHAT_MESSAGE_HPP