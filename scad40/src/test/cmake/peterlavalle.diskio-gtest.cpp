
#include "gtest/gtest.h"
#define PAL_ADLER32_TEST

#include "peterlavalle.diskio-gtest.hpp"

#include <duktape.h>
#include <peterlavalle.diskio.hpp>

#define HEREDOC(TEXT) (#TEXT)

/// this need some extra fiddlin so it's done here
scad40::duk_ref<peterlavalle::diskio::Reading> peterlavalle::diskio::Disk::open(const scad40::duk_str& path)
{
	stupid_mock::get(Host()).soft_call((std::string(__FUNCTION__ "(") + (const char*)path + ")").c_str());

	auto result = peterlavalle::diskio::Reading::New(Host());

	result->_path = path;

	return result;
}

TEST(duktape, onoff)
{
	duk_context* context = duk_create_heap_default();

	EXPECT_NE(nullptr, context);

	duk_destroy_heap(context);
}

TEST(scad40, onoff)
{
	duk_context* context = duk_create_heap_default();

	{
		stupid_mock mock;
		
		mock.hard_call("peterlavalle::diskio::Disk::Disk");
		mock.hard_call("peterlavalle::diskio::Disk::~Disk");		                

		mock.replay(context);

		peterlavalle::diskio::install(context);

		EXPECT_NE(nullptr, context);

		duk_destroy_heap(context);
	}

}

TEST(scad40, runcall)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(pokei)");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	peterlavalle::diskio::Disk::get(context).foobar("pokei");

	duk_destroy_heap(context);
}


TEST(scad40, newscripted)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(pokey)");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	{
		duk_eval_string_noresult(
			context,
			HEREDOC(
				newscripted = function()
				{
					/*peterlavalle.diskio.Disk.foobar('pokey');*/

					this.last = "???";
					this.fileChanged = function(path)
					{
						/*peterlavalle.diskio.Disk.foobar('from `' + this.last + '` to `' + path + '`');*/
						this.last = path;
					};
				};
			)
		);
	
		auto changeListener = peterlavalle::diskio::ChangeListener::New(context, "newscripted");

		changeListener->fileChanged("thing");
		// causes TypeError: 

		peterlavalle::diskio::Disk::get(context).foobar("pokei");

		auto changeListener2 = peterlavalle::diskio::ChangeListener::New(context, "newscripted");

		changeListener2->fileChanged("beyond");

		changeListener->fileChanged("cake!");
	}

	duk_destroy_heap(context);
}