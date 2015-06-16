#pragma once
#include <string>
using namespace std;

class _message
{
public:
	enum {
		max_lenght = 512,
		id_length = 2,
		length_length = 4,
		header_length = id_length + length_length
	};


	_message()
	{

	}

	char* data()
	{
		return data_;
	}

	void encode_header(int length, int id)
	{
		this->length_ = length;
		this->id_ = id;

		string size;

		size = "";
		size += "%0";
		size += to_string(length_length);
		size += "d";
		char lc[length_length + 1] = "";
		sprintf(lc, size.c_str(), length);
		memcpy(data(), lc, length_);

		size = "";
		size += "%0";
		size += to_string(id_length);
		size += "d";
		char ic[id_length + 1] = "";
		sprintf(ic, size.c_str(), id);
		memcpy(data() + length_length, ic, strlen(ic));
	}

	void decode_header()
	{
		using namespace std;

		// decode length
		char length[length_length + 1] = "";
		memcpy(length, data(), length_length);
		this->length_ = atoi(length);

		// decode id
		char id[id_length + 1] = "";
		memcpy(id, data()+length_length, id_length);
		this->id_ = atoi(id);
	}

	char* body()
	{
		return (data_ + header_length);
	}

	void set_body(const char* msg)
	{
		int length = strlen(msg);

		memcpy(body(), msg, length);
	}

	int get_length()
	{
		return length_;
	}
	
	int set_length(int length)
	{
		this->length_ = length;
	}

	int get_id()
	{
		return id_;
	}

	void set_id(int id)
	{
		this->id_ = id;
	}

private:

private:
	char data_[max_lenght];
	int length_;
	int id_;
};

