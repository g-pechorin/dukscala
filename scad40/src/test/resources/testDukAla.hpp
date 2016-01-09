
#pragma once

#include <array>
#include <string>

#include <duktape.h>
#include <stdint.h>

///
/// These classes are used to bridge-the-gap between Scala-JS and C++
/// ... they're not thread-safe but then ... neither is DukTape
/// ... all modules (should) use the same definition of them - so don't worry about namespace collisions
#ifndef _scad40_duk_ref_et_str
#define _scad40_duk_ref_et_str

#define scad40_duk_ref_et_str__string ("\xFF" "scad40")
#define scad40_duk_ref_et_str__strlen (7)

namespace scad40
{
	class object
	{
		/// magical pointer to the object's hosting context
		/// ... it will/should be set before construction
		duk_context* _ctx;

		template<typename T>
		friend class duk_ref<T>;

		friend class duk_str;

		std::array<char, scad40_duk_ref_et_str__strlen + (sizeof(void*) * 2)> key_string(void) const;

		template<typename T>
		static const char* type_string(void);
	protected:
		object(void);
	public:
		duk_context* Host(void);
	};

	///
	/// holds a ref to a duktape object using magic
	template<typename T>
	class duk_ref : public scad40::object
	{
		T* _ptr;
	public:
		duk_ref(void);

		duk_ref(const duk_ref<T>&);
		duk_ref<T>& operator = (const duk_ref<T>&);

		T* operator -> (void);
		const T* operator -> (void) const;

		~duk_ref(void);
	};

	///
	/// holds a ref to a duktape string using magic
	struct duk_str : public scad40::object
	{
		duk_str(void);
		duk_str(duk_context* ctx);
		duk_str(duk_context* ctx, const char*);
		duk_str(duk_context* ctx, const std::string&);

		duk_str(const duk_str&);
		duk_str& operator = (const duk_str&);

		duk_str& operator = (const char*);
		duk_str& operator = (const std::string&);

		operator const char*(void) const;

		~duk_str(void);
	};

};
#endif // ... okay - that's the end of predef

namespace peterlavalle {
namespace diskio {

	/// a script class
	struct ChangeListener : public scad40::object
	{
		misc head stuff

		/// the user's requested members
		void fileChanged (const scad40::duk_str& path);

		void Push(void)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		static bool Is(duk_context* ctx, duk_idx_t idx)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		static scad40::duk_ref<ChangeListener> To(duk_context* ctx, duk_idx_t idx)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		static scad40::duk_ref<ChangeListener> New(duk_context* ctx)
		{
			assert(false && "??? scad40 needs to provide this");
		}
	};

	/// a native class
	struct Reading : public scad40::object
	{
		/// the Reading constructor
		/// the user must implement this
		Reading(void);

		/// the user's requested members
		/// the user must implement these
		int8_t read ();
		void close ();
		bool endOfFile ();
		float _number;
		const scad40::duk_str _path;

		/// the Reading destructor
		/// the user must implement this
		~Reading(void);

		/// pushes this object onto its hosting stack
		void Push(void)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		/// queries if the passed index is a Reading object
		static bool Is(duk_context* ctx, duk_idx_t index)
		{
			idx = idx < 0 ? duk_normalize(ctx, idx) : idx;

			assert(false && "??? scad40 needs to provide this");
		}

		/// pulls the the passed index into a Reading object
		/// ... if the passed index is not a Reading object - behaviour is undefined
		static scad40::duk_ref<Reading> To(duk_context* ctx, duk_idx_t idx)
		{
			idx = idx < 0 ? duk_normalize(ctx, idx) : idx;

			if (!Is(ctx, idx))
			{
				duk_error(ctx, 314, "Tried to grab `%s` as a {Reading} (which it is not)", duk_to_string(ctx, idx);
			}

			assert(false && "??? scad40 needs to provide this");
		}

		/// creates a new Reading object and returns a magical pointer to it
		static scad40::duk_ref<Reading> New(duk_context* ctx)
		{
			assert(false && "??? scad40 needs to provide this");
		}
	};

	/// a global class
	struct Disk : public scad40::object
	{
		/// the Disk constructor
		/// the user must implement this
		Disk(void);

		/// the user's requested members
		/// the user must implement these
		scad40::duk_ref<Reading> open (const scad40::duk_str& path);
		scad40::duk_str _pwd;
		void subscribe (const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener);
		void unsubscribe (const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener);

		/// locates the instance
		static Disk& get(duk_context*);
	};

	/// sets up the tables and calls to this VM
	void install(duk_context* ctx)
	{
		assert(false && "?? check for name collisions");

		assert(false && "?? load up code to make script super-classes");

		assert(false && "?? bind lambdas for native class construction");

		assert(false && "?? allocate and store ALL global objects (including context pointers)");

		assert(false && "?? in-place-new global objects in-order");
	}
}
}

#pragma region "Implementations"

inline scad40::object::object(void)
{
	assert(nullptr != _ctx);
}

inline duk_context* scad40::object::Host(void)
{
	return _ctx;
}

inline std::array<char, scad40_duk_ref_et_str__strlen + (sizeof(void*) * 2)> scad40::object::key_string(void) const
{
	assert(strlen(scad40_duk_ref_et_str__string) == scad40_duk_ref_et_str__strlen);
	std::array<char, scad40_duk_ref_et_str__strlen + (sizeof(void*) * 2)> result;

	strcpy(result + 0, scad40_duk_ref_et_str__string);
	size_t write += strlen(scad40_duk_ref_et_str__string);

	union {
		uint8_t _chars[(sizeof(void*) * 2)];
		void* _cast;
	} swang;

	swang._cast = const_cast<void*>(reinterpret_cast<const void*>(this));

	for (size_t i = 0; i < sizeof(void*); ++i)
	{
		uint8_t pair = swang._chars[i];

		char c;

		result[write++] = 'A' + (pair & 0x0F);
		result[write++] = 'A' + ((pair & 0xF0) >> 4);
	}

	return result;
}

template<typename T>
inline const char* scad40::object::type_string(void)
{
	/// there is an insanely small chance of a leak
	static char* _ptr = nullptr;
	if (nullptr != _ptr )
	{
		return _ptr ;
	}

	const char* str = typeid(T).name();
	size_t len = strlen(str);
	char* ptr = (const char*) malloc(len + 1)

	size_t src = 0, out = 0;

	while (str[src])
	{
		const auto c = str[src++];
		if (('a' <= c && c <= 'z')||('A' <= c && c <= 'Z')||('0' <= c && c <= '9')||('_' == c))
		{
			ptr[out++] = c;
			continue;
		}

		if (c == ':' && ':' == str[src])
		{
			ptr[out++] = '.';
			++src;
			continue;
		}

		out = 0;
	}

	assert(out > 0);
	assert(ptr[0]);
	assert(ptr[out - 1]);
	ptr[out] = '\0';
	return _ptr = ptr;
}
#pragma endregion