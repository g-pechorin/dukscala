
/// this file contains the "stubs" used to log invocations

#pragma once

#include "gtest/gtest.h"

#include <duktape.h>
#include <peterlavalle.diskio.hpp>

#include <functional>
#include <list>
#include <map>
#include <string>

class stupid_mock
{
	std::list<std::map<std::string, size_t>> _expectations;
	bool _recording;
public:
	size_t remaining(void) const
	{
		size_t remaining = 0;
		for (auto& group : _expectations)
		{
			for (auto& pait : group)
			{
				remaining += pait.second;
			}
		}
		return remaining;
	}

	stupid_mock(void)
	{
		_recording = true;
	}

	duk_context* replay(void)
	{
		EXPECT_TRUE(_recording) << "Mock was already switched into `replay` mode";
		auto context = duk_create_heap_default();
		duk_push_pointer(context, this);
		duk_put_global_string(context, "\xFF" "stupid_mock");
		_recording = false;
		return context;
	}

	static stupid_mock& get(duk_context* ctx)
	{
		duk_get_global_string(ctx, "\xFF" "stupid_mock");
		void* ptr = duk_to_pointer(ctx, -1);
		duk_pop(ctx);
		return *reinterpret_cast<stupid_mock*>(ptr);
	}

	bool empty(void)
	{
		return _expectations.empty();
	}

	~stupid_mock(void)
	{
		auto roy = remaining();
		EXPECT_EQ(0, roy) << "There were " << roy << " calls that didn't happen";
		EXPECT_FALSE(_recording) << "Mock was never switched into `replay` mode";
		EXPECT_TRUE(empty()) << "Unspent expectations";
	}

	void soft_call(const char* name)
	{
		const std::string key = name;

		if (_recording)
		{
			if (empty())
			{
				_expectations.emplace_back();
			}

			std::map<std::string, size_t>& back = _expectations.back();

			if (back.end() == back.find(key))
			{
				back[key] = 0;
			}

			back[key] = back[key] + 1;
		}
		else
		{
			EXPECT_FALSE(empty()) << "\n\t>>>>Unexpected call `" << name << "` - I'm out of expectations" << std::endl;
			
			if (empty())
			{
				return;
			}

			std::map<std::string, size_t>& head = _expectations.front();

			EXPECT_FALSE(head.end() == head.find(key)) << "\n\t>>>>Unexpected call `" << name << "` - it doesn't match the present expectations" << std::endl;
			if (head.end() == head.find(key))
			{
				return;
			}
			
			if (0 == (head[key] = head[key] - 1))
			{
				head.erase(key);
			}

			while (_expectations.front().empty())
			{
				_expectations.pop_front();

				if (empty())
				{
					return;
				}
			}
		}
	}


	void soft_call(const std::string& string)
	{
		soft_call(string.c_str());
	}

	void soft_call(const std::stringstream& stream)
	{
		soft_call(stream.str());
	}

	void add_block(void)
	{
		EXPECT_TRUE(_recording)<< "You can't add execution blocks in replay mode ... not sure why you'd try though";

		if (empty() || _expectations.back().empty())
		{
			return;
		}

		_expectations.emplace_back();
	}

	void hard_call(const char* name)
	{
		EXPECT_TRUE(_recording) << "You can't make hard calls durring replay mode";

		add_block();
		soft_call(name);
		add_block();
	}
};


peterlavalle::diskio::Reading::Reading(void) :
	_path(Host())
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

peterlavalle::diskio::Reading::~Reading(void)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

peterlavalle::diskio::Disk::Disk(void) :
_pwd(Host())
{
	std::stringstream log;
	log << __FUNCTION__ << "(" << ")";

	stupid_mock::get(Host()).soft_call(log);
}

void peterlavalle::diskio::Disk::subscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	std::stringstream log;

	log << __FUNCTION__ << "(" << path << ", " << "ChangeListener@" << (size_t)(&listener) << ")";

	stupid_mock::get(Host()).soft_call(log);
}

void peterlavalle::diskio::Disk::unsubscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

peterlavalle::diskio::Disk::~Disk(void)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}
