#include <iostream>

#include "gtest/gtest.h"

TEST(scad40Test, a_thing)
{
	EXPECT_EQ(4, (2 + 2)) << "Obvious";
}

#include "duktape.h"

TEST(scad40Test, duk_context)
{
	duk_context* ctx = duk_create_heap_default();

	ASSERT_TRUE(nullptr != ctx) << "Couldn't create Duktape";

	duk_destroy_heap(ctx);
}

#include "D40.hpp"

peterlavalle::diskio::Disk::Disk(void)
{
}

scad40::duk_native<peterlavalle::diskio::Reading> peterlavalle::diskio::Disk::open(scad40::duk_string& path)
{
	auto result = peterlavalle::diskio::Reading::New(Host());

	result->_handle = fopen(path.c_str(), "r");
	result->_path = path;

	return result;
}

peterlavalle::diskio::Disk::~Disk(void)
{
}


peterlavalle::diskio::Reading::Reading(void) :
_path(Host(), ""),
_handle(nullptr)
{
}

int8_t peterlavalle::diskio::Reading::read()
{
	assert(nullptr != _handle);

	int8_t byte;
	EXPECT_EQ(1, fread(&byte, 1, 1, _handle));
	return byte;
}

void peterlavalle::diskio::Reading::close()
{
	assert(nullptr != _handle);

	fclose(_handle);

	_handle = nullptr;
}

bool peterlavalle::diskio::Reading::endOfFile()
{
	assert(nullptr != _handle);
	auto cur = ftell(_handle);
	if (fseek(_handle, 0, SEEK_END))
	{
		assert(false);
	}
	auto end = ftell(_handle);
	if (fseek(_handle, cur, SEEK_SET))
	{
		assert(false);
	}
	return end == cur;
}

peterlavalle::diskio::Reading::~Reading(void)
{
	if (_handle)
	{
		close();
	}

	assert(nullptr == _handle);
}

TEST(scad40Test, scad40)
{
	duk_context* ctx = duk_create_heap_default();

	ASSERT_TRUE(nullptr != ctx) << "Couldn't create Duktape";

	peterlavalle::diskio::install(ctx);

	duk_destroy_heap(ctx);
}

TEST(scad40Test, readFileFromNative)
{
	duk_context* ctx = duk_create_heap_default();

	ASSERT_TRUE(nullptr != ctx) << "Couldn't create Duktape";

	peterlavalle::diskio::install(ctx);

	const char* name = "test-temp.file";

	int8_t data[] = {
		3, 14, 19, 83
	};

	{
		auto file = fopen(name, "w");
		ASSERT_NE(nullptr, file) << "Failed to open file";
		ASSERT_EQ(1, fwrite(data, sizeof(data), 1, file)) << "Failed to write data";
		ASSERT_EQ(0, fclose(file)) << "Failed to close file";
	}

	{
		auto reading = peterlavalle::diskio::Disk::get(ctx).open(name);

		for (size_t i = 0; i < sizeof(data); ++i)
		{
			ASSERT_FALSE(reading->endOfFile()) << "Premature end of file @ " << i;

			ASSERT_EQ(data[i], reading->read()) << "Mismatching data @ " << i;
		}

		ASSERT_TRUE(reading->endOfFile()) << "File ran-on";
		reading->close();
	}

	duk_destroy_heap(ctx);
}
