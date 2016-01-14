
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

void peterlavalle::diskio::Disk::foobar(const scad40::duk_str& text)
{
	std::stringstream log;

	log << __FUNCTION__ << "(" << text << ")";

	stupid_mock::get(Host()).soft_call(log.str().c_str());
}

TEST(duktape, onoff)
{
	duk_context* context = duk_create_heap_default();

	EXPECT_NE(nullptr, context);

	duk_destroy_heap(context);
}

TEST(scad40, onoff)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();

	peterlavalle::diskio::install(context);

	EXPECT_NE(nullptr, context);

	duk_destroy_heap(context);
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
	mock.hard_call("peterlavalle::diskio::Disk::foobar(pokey)");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(from `???` to `thing`)");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(pokei)");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(pokey)");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(from `???` to `beyond`)");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(from `thing` to `cake!`)");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	{
		duk_eval_string_noresult(
			context,
			HEREDOC(
				newscripted = function()
				{
					peterlavalle.diskio.Disk.foobar('pokey');

					this.last = "???";
					this.fileChanged = function(path)
					{
						peterlavalle.diskio.Disk.foobar('from `' + this.last + '` to `' + path + '`');
						this.last = path;
					};
				};
			)
		);

		EXPECT_EQ(0, duk_get_top(context)) << "Unexpected stack values";

		duk_eval_string(context, "new newscripted();");

		EXPECT_EQ(1, duk_get_top(context)) << "That should have yielded something";

		EXPECT_TRUE(peterlavalle::diskio::ChangeListener::As(context, 0)) << "That should be the thing I expected?";

		auto newscripted = peterlavalle::diskio::ChangeListener::To(context, 0);
		EXPECT_FALSE(newscripted.IsNull()) << "That should have produced an object";



		duk_pop(context);

		auto changeListener = peterlavalle::diskio::ChangeListener::New(context, "newscripted");

		// looks like a problem with the copy-constructor or the copy operator

		EXPECT_FALSE(changeListener.IsNull()) << "That should have produced an object";

		changeListener->fileChanged("thing");
		// causes TypeError: 

		peterlavalle::diskio::Disk::get(context).foobar("pokei");

		auto changeListener2 = peterlavalle::diskio::ChangeListener::New(context, "newscripted");

		changeListener2->fileChanged("beyond");

		changeListener->fileChanged("cake!");
	}

	duk_destroy_heap(context);
}