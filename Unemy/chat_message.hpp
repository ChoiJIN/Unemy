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
	enum { id_length = 1 };
	enum { header_length = 4 + id_length };
	enum { max_body_length = 512 };

	chat_message()
		: body_length_(0)
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
		char header[header_length + 1 - id_length] = "";
		strncat(header, data_, header_length - id_length);
		body_length_ = atoi(header);
		char ids[id_length + 1] = "";
		strncat(ids, data_ + header_length - id_length, id_length);
		id_ = atoi(ids);
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
		sprintf(header, "%4d", body_length_);
		memcpy(data_, header, header_length - id_length);
	}

	void add_id(int id)
	{
		using namespace std;
		data_[header_length - id_length] = id+'0';
		//memcpy(data_ + 4, ids, 1);
	}

	int get_id()
	{
		return id_;
	}

private:
	char data_[header_length + max_body_length];
	int id_;
	size_t body_length_;
};

#endif // CHAT_MESSAGE_HPP