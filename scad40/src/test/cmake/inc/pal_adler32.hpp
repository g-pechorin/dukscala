///	Peter LaValle / gmail
///	pal_adler32.hpp
///
/// This file provides adler32 functionality compatible with Visual Studio 2013 CE.
///
/// 
///	Contents;
///		> a function `fun(...)` to calculate an adler32 sum from a null-terminated string
///		> an object `obj` that wraps this into an imutable compare-to-anything class
///		> a TMP construct that can calculate the value at compile-time and is suitable for switch statements
///
/// The last one is possibly redundant in the face of constexpr - `fun` could be marked constexpr and used instead (maybe)
/// ... but constexpr isn't available on V$2013 so I haven't checked
///
///
///	======================================================================================================================
#pragma once

#include <assert.h>
#include <stdint.h>

#include <string>

namespace pal_adler32
{
	// https://en.wikipedia.org/wiki/Adler-32#Example
	inline uint32_t fun(const char* text)
	{
		assert(nullptr != text);
		
		uint32_t a = 1, b = 0;

		for (size_t index = 0; text[index]; ++index)
		{
			a = (a + text[index]) % 65521;
			b = (b + a) % 65521;
		}

		return (b << 16) | a;
	}

	///
	/// Object for sum/string comparisons
	///
	struct obj
	{
	private:
		const uint32_t _sum;
	public:
		obj(const char* text) : _sum(fun(text))
		{}

		obj(const std::string& text) : _sum(fun(text.c_str()))
		{}

		obj(const std::type_info& type_info) : _sum(fun(type_info.name()))
		{}

		template<typename T> static obj sum(void)		{ return obj(typeid(T)); }

		bool operator==(const char* text) const			{ return _sum == fun(text); }

		bool operator!=(const char* text) const			{ return _sum != fun(text); }

		bool operator==(const std::string& text) const	{ return _sum == fun(text.c_str()); }

		bool operator!=(const std::string& text) const	{ return _sum != fun(text.c_str()); }

		operator const uint32_t(void) const				{ return _sum; }
	};


	/// I'm using the funky `template<...> struct ...` form below because these aren't type-teplates

	// with help from ; http://eli.thegreenplace.net/2014/variadic-templates-in-c/
	// pal_adler32::fun("Wikipedia")) == pal_adler32::charlist<'W', 'i', 'k', 'i', 'p', 'e', 'd', 'i', 'a'>::sum<>::val
	// should let code-generators use almost-strings in switch statements
	// ... not sure what'll happen on big-endian CPUs (best you put an assertion in place)
	template <char...> struct charlist
	{
		template <uint32_t a, uint32_t b> struct sum
		{
			enum _tmp : uint32_t
			{
				val = (b << 16) | a,
			};
		};

		enum _tmp : size_t { len = 0, };

		static void cpy(char* out) { *out = '\0'; }
		static const char* str(void) { return ""; }
	};

	template <char head, char... tail> struct charlist<head, tail...>
	{
		template <uint32_t a = 1, uint32_t b = 0> struct sum
		{
			enum _tmp : uint32_t
			{
				val = charlist<tail...>::sum<(a + head) % 65521, (b + ((a + head) % 65521)) % 65521>::val,
			};
		};

		enum _tmp : size_t { len = 1 + charlist<tail...>::len, };

		// copies the string to a normal C-string
		static void cpy(char* out) { *out = head; charlist<tail...>::cpy(out + 1); }

		// returns a static-local version of the C-string
		static const char* str(void)
		{
			static char mem[len];
			cpy(mem);
			return mem;
		}
	};
};

/// Tests for Zhanyong Wan's framework
/// ... which also show usage
#pragma region "Google Tests"
#if defined(PAL_ADLER32_TEST)

#if !defined(GTEST_INCLUDE_GTEST_GTEST_H_) 
#error I need GoogleTest
#endif

TEST(pal_adler32, Wikipedia_switch)
{
	bool hit = false;
	switch (pal_adler32::fun("Wikipedia"))
	{
	case pal_adler32::charlist<'W', 'i', 'k', 'i', 'p', 'e', 'd', 'i', 'a'>::sum<>::val:
		hit = true;
	default:
		break;
	}
	EXPECT_TRUE(hit);
}

TEST(pal_adler32, Wikipedia_const_fun)
{
	const static pal_adler32::obj Wikipedia = "Wikipedia";

	EXPECT_EQ(
		pal_adler32::fun("Wikipedia"),
		Wikipedia
	);
}

TEST(pal_adler32, Wikipedia_const_str)
{
	const static pal_adler32::obj Wikipedia = "Wikipedia";

	EXPECT_TRUE(Wikipedia == "Wikipedia");	
}

#endif
