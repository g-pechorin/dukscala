
/// this file contains the "stubs" used to log invocations

#pragma once


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

	stupid_mock(void)
	{
		_recording = true;
	}
	
	void replay(duk_context* ctx)
	{
		EXPECT_TRUE(_recording) << "Mock was already switched into `replay` mode";
		duk_push_pointer(ctx, this);
		duk_put_global_string(ctx, "\xFF" "stupid_mock");
		_recording = false;
	}

	duk_context* replay(void)
	{
		auto context = duk_create_heap_default();

		replay(context);

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
			EXPECT_FALSE(empty()) << "Unexpected call `" << name << "` - I'm out of expectations";
			
			if (empty())
			{
				return;
			}

			std::map<std::string, size_t>& head = _expectations.front();

			EXPECT_FALSE(head.end() == head.find(key));
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
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

void peterlavalle::diskio::Disk::foobar(const scad40::duk_str& text)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

void peterlavalle::diskio::Disk::subscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

void peterlavalle::diskio::Disk::unsubscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

peterlavalle::diskio::Disk::~Disk(void)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}


#if 0
class scad40mock
{
public:
	static void install(duk_context* ctx);

	static scad40mock& get(duk_context* ctx);

	template<typename T, typename ... ARGS>
	T replay(const char* name, T result, std::function<bool(ARGS&& ...args)> checker);

	template<typename T, typename ... ARGS>
	T verify(const char* name, ARGS&& ...args);
};

peterlavalle::diskio::Reading::Reading(void) :
	_path(Host())
{
	scad40mock::get(Host()).verify<void>("peterlavalle::diskio::Reading::Reading(void)");
}

peterlavalle::diskio::Reading::~Reading(void)
{
	scad40mock::get(Host()).verify<void>("peterlavalle::diskio::Reading::~Reading(void)");
}

peterlavalle::diskio::Disk::Disk(void) :
	_pwd(Host())
{
	scad40mock::get(Host()).verify<void>("peterlavalle::diskio::Disk::Disk(void)");
}

void peterlavalle::diskio::Disk::foobar(const scad40::duk_str& text)
{
	scad40mock::get(Host()).verify<void>("peterlavalle::diskio::Disk::foobar", text);
}

scad40::duk_ref<peterlavalle::diskio::Reading> peterlavalle::diskio::Disk::open(const scad40::duk_str& path)
{
	return scad40mock::get(Host()).verify<scad40::duk_ref<peterlavalle::diskio::Reading>>("peterlavalle::diskio::Disk::open", path);
}

void peterlavalle::diskio::Disk::subscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	scad40mock::get(Host()).verify<void>("peterlavalle::diskio::Disk::subscribe", path, listener);
}

void peterlavalle::diskio::Disk::unsubscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	scad40mock::get(Host()).verify<void>("peterlavalle::diskio::Disk::unsubscribe", path, listener);
}

peterlavalle::diskio::Disk::~Disk(void)
{
	scad40mock::get(Host()).verify<void>("peterlavalle::diskio::Disk::~Disk(void)");
}

template<typename T, typename ... ARGS>
T scad40mock::replay(const char* name, T result, std::function<bool(ARGS&& ...args)> checker)
{
	struct anyblob

}

template<typename T, typename ... ARGS>
T verify(const char* name, ARGS&& ...args);
#endif