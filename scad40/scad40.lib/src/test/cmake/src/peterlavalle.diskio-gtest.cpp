
#define PAL_ADLER32_TEST
#include "gtest/gtest.h"

#include "peterlavalle.diskio-gtest.hpp"
#include "peterlavalle.diskio-stupid_mock.hpp"

#define HEREDOC(TEXT) (#TEXT)

TEST(duktape, onoff)
{
	duk_context* context = duk_create_heap_default();

	EXPECT_NE(nullptr, context);

	duk_destroy_heap(context);
}

TEST(scad40, onoff)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();

	peterlavalle::diskio::install(context);

	EXPECT_NE(nullptr, context);

	duk_destroy_heap(context);
}

TEST(scad40, runcall)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(pokei)");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	peterlavalle::diskio::Disk::get(context).foobar("pokei");

	duk_destroy_heap(context);
}

TEST(scad40, thing_in_script)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Disk::foobar(pokey)");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	duk_eval_string_noresult(
		context,
		HEREDOC(
			newscripted = function()
			{
				peterlavalle.diskio.Disk.foobar('pokey');
			};
		)
	);

	EXPECT_EQ(0, duk_get_top(context)) << "Unexpected stack values";

	duk_eval_string(context, "new newscripted();");

	EXPECT_EQ(1, duk_get_top(context)) << "That should have yielded something";
	EXPECT_FALSE(peterlavalle::diskio::ChangeListener::As(context, 0)) << "That should NOT be suitable";

	duk_destroy_heap(context);
}

TEST(scad40, newscripted)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
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

TEST(scad40, check__duk_ptr)
{
	duk_context* ctx = duk_create_heap_default();

	void* thing = duk_alloc(ctx, 128);

	duk_push_pointer(ctx, thing);

	struct junk : scad40::_handle
	{

	};



	duk_destroy_heap(ctx);
}

TEST(scad40, create_native_in_script)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Reading::Reading");
	mock.soft_call("peterlavalle::diskio::Reading::~Reading");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	EXPECT_EQ(DUK_EXEC_SUCCESS, duk_peval_string(context, "new peterlavalle.diskio.Reading();")) << "Script failed `" << duk_safe_to_string(context, -1) << "`";
	EXPECT_EQ(1, duk_get_top(context)) << "Stack was the wrong size";
	EXPECT_TRUE(
		peterlavalle::diskio::Reading::Is(context, 0)
	) << "Did not return a Reading instance, it was `" << duk_safe_to_string(context, 0) << "`";
	duk_destroy_heap(context);
}

TEST(scad40, Reading_read__from_script)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Reading::Reading");
	mock.hard_call("peterlavalle::diskio::Reading::read()");
	mock.soft_call("peterlavalle::diskio::Reading::~Reading");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	EXPECT_EQ(DUK_EXEC_SUCCESS, duk_peval_string(context, "new peterlavalle.diskio.Reading().read();")) << "Script failed `" << duk_safe_to_string(context, -1) << "`";
	EXPECT_EQ(1, duk_get_top(context)) << "Stack was the wrong size";
	EXPECT_EQ(14, duk_to_int32(context, 0));
	duk_destroy_heap(context);
}

TEST(scad40, Reading_close__from_script)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Reading::Reading");
	mock.hard_call("peterlavalle::diskio::Reading::close()");
	mock.soft_call("peterlavalle::diskio::Reading::~Reading");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	EXPECT_EQ(DUK_EXEC_SUCCESS, duk_peval_string(context, "new peterlavalle.diskio.Reading().close();")) << "Script failed `" << duk_safe_to_string(context, -1) << "`";
	EXPECT_EQ(1, duk_get_top(context)) << "Stack was the wrong size";
	EXPECT_TRUE(duk_is_null_or_undefined(context, 0));
	duk_destroy_heap(context);
}

TEST(scad40, read_var_from_script)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Reading::Reading");
	mock.soft_call("peterlavalle::diskio::Reading::~Reading");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	EXPECT_EQ(DUK_EXEC_SUCCESS, duk_peval_string(context, "new peterlavalle.diskio.Reading().number;")) << "Script failed `" << duk_safe_to_string(context, -1) << "`";
	EXPECT_EQ(1, duk_get_top(context)) << "Stack was the wrong size";
	EXPECT_EQ(1983.3f, duk_to_number(context, 0));
	duk_destroy_heap(context);
}

TEST(scad40, write_var_from_script)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Reading::Reading");
	mock.soft_call("peterlavalle::diskio::Reading::~Reading");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	EXPECT_EQ(DUK_EXEC_SUCCESS, duk_peval_string(context, "var r = new peterlavalle.diskio.Reading(); r.number = 3.14; r")) << "Script failed `" << duk_safe_to_string(context, -1) << "`";
	EXPECT_EQ(1, duk_get_top(context)) << "Stack was the wrong size";
	EXPECT_TRUE(peterlavalle::diskio::Reading::Is(context, 0));

	if (peterlavalle::diskio::Reading::Is(context, 0))
	{
		auto reading = peterlavalle::diskio::Reading::To(context, 0);
		EXPECT_EQ(3.14f, reading->_number);
	}

	duk_destroy_heap(context);
}

TEST(scad40, read_val_from_script)
{
	stupid_mock mock;

	mock.hard_call("peterlavalle::diskio::Disk::Disk()");
	mock.hard_call("peterlavalle::diskio::Reading::Reading");
	mock.soft_call("peterlavalle::diskio::Reading::~Reading");
	mock.hard_call("peterlavalle::diskio::Disk::~Disk");

	duk_context* context = mock.replay();
	peterlavalle::diskio::install(context);

	EXPECT_EQ(DUK_EXEC_SUCCESS, duk_peval_string(context, "new peterlavalle.diskio.Reading().path;")) << "Script failed `" << duk_safe_to_string(context, -1) << "`";
	EXPECT_EQ(1, duk_get_top(context)) << "Stack was the wrong size";
	EXPECT_STRCASEEQ("my/path.spot", duk_to_string(context, 0));
	duk_destroy_heap(context);
}
