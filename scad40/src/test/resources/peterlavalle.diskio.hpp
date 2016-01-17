
#pragma once

///
/// These classes are used to bridge-the-gap between Scala-JS and C++
/// ... they're not thread-safe but then ... neither is DukTape
/// ... all modules (should) use the same definition of them - so don't worry about namespace collisions
#ifndef _scad40_head
#define _scad40_head

#include <duktape.h>

#include "pal_adler32.hpp"

#include <array>
#include <string>
#include <iostream>

#include <assert.h>
#include <stdint.h>

#define scad40__pre_string ("\xFF" "scad40")
#define scad40__pre_strlen (7)

namespace scad40
{
	/// goofy wrapper to push member-like functions (so methods and accessors)
	/// ... should I call it "push_method?" or "push_member?"
	template<typename T>
	inline void push_selfie(duk_context* ctx, T* self, duk_idx_t nargs, duk_ret_t(*code)(duk_context*, T*));

	/// checks if the table at {idx} has a key named {key} and
	/// @returns false if the value is undefined_or_null.
	/// restores the stack
	bool has_prop(duk_context* ctx, const duk_idx_t idx, const char* key);

	/// base object for things that play with with pointers
	/// don't look to closesly at the guts
	/// used for both the "_handle" subclasses and the generated derrived types
	class _object
	{
		/// magical pointer to the object's hosting context
		/// ... it will/should be set before construction (did I mention `magic`)
		duk_context* _ctx;

		friend struct _handle;

		/// usercode should never actually call this (trust me)
		_object(duk_context*);


		/// copy operators are hidden since they should only be used by _handle
		_object(const _object& other) :
			_object(other._ctx)
		{
		}
		_object& operator=(const _object& other)
		{
			assert(_ctx == other._ctx);
			return *this;
		}
	protected:

		/// usercode will laways use this method to invoke the parent
		_object(void)
		{
			/// this will/should have been magically setup
			assert(nullptr != _ctx);
		}

	public:
		duk_context* Host(void) const;
	};

	/// this refers to a thing in script-land but is not itself a thing
	/// it's used for the duk_(ref|str|ptr) classes
	/// Google calls it a handle - https://developers.google.com/v8/embed?hl=en
	struct _handle : scad40::_object
	{
		/// pushes (a reference to) the value onto the stack (or maybe null if the value is null)
		inline void Push(void) const;

		bool IsNull(void) const;

	protected:

		_handle(duk_context* ctx) : scad40::_object(ctx)
		{
		}


		std::array<char, scad40__pre_strlen + (sizeof(void*) * 2) + 1> KeyString(void) const;

		/// partially expose these operators
		_handle(const _handle& other);
		_handle& operator=(const _handle& other);

		~_handle(void)
		{
			if (nullptr == _ctx)
			{
				return;
			}

			// stack -> ... ;

			duk_push_global_stash(_ctx);
			// stack -> ... ; [global stash] ;

			duk_del_prop_string(_ctx, -1, KeyString().data());

			duk_pop(_ctx);
			// stack -> ... ;

			_ctx = nullptr;
		}
	};

	/// this allows manipulating a pure-script object from C++ using a predefined interface
	/// ironically ; less sophiticated than the ref
	template<typename T>
	struct duk_ptr : public scad40::_handle
	{
		duk_ptr(duk_context* ctx, duk_idx_t idx) : scad40::_handle(ctx)
		{
			idx = duk_normalize_index(ctx, idx);
			assert(T::As(ctx, idx));

			// stack -> ... ; idx ... ;

			duk_push_global_stash(ctx);
			// stack -> ... ; idx ... ; [stash] ;

			duk_dup(ctx, idx);
			// stack -> ... ; idx ... ; [stash] ; [val] ;

			duk_put_prop_string(ctx, -2, KeyString().data());
			// stack -> ... ; idx ... ; [stash] ;

			duk_pop(ctx);
			// stack -> ... ; idx ... ;
		}

		T* operator->(void)
		{
			return reinterpret_cast<T*>(this);
		}

		const T* operator->(void) const
		{
			return reinterpret_cast<T*>(this);
		}
	};

	/// baseclass used for _handle things with a native pointer
	template<typename T>
	struct _native : scad40::_handle
	{

	protected:
		T* _pointer;
		_native(duk_context* ctx) : scad40::_handle(ctx)
		{
		}

	public:

		operator T* (void) const { return _pointer; }
		T* operator->(void) { return _pointer; }
		const T* operator->(void) const { return _pointer; }
	};

	/// holds a ref to a C++ object built for duktape using magic
	/// the pointer is updated on copy so that you get a bit faster access to it
	template<typename T>
	struct duk_ref : public scad40::_native<T>
	{
	public:

		/// grab an instance from the stack. fails violently if types are wrong
		duk_ref(duk_context*, const duk_idx_t);

	};

	/// holds a ref to a duktape string using magic
	/// as much as possible I stick strings into duktape and try not to think too hard about them
	/// the pointer is updated on copy so that you get a bit faster access to it
	class duk_str : public scad40::_native<const char>
	{
	public:
		/// create an instance and set it to point to the passed string
		duk_str(duk_context* ctx, const char* = nullptr);

		/// create an instance and set it to point to the passed string
		duk_str(duk_context* ctx, const std::string&);

		/// grab an instance from the stack. fails violently if types are wrong
		duk_str(duk_context* ctx, const duk_idx_t);

		duk_str& operator= (const char*);
		duk_str& operator= (const std::string&);

		duk_str& operator== (const char*) const;
		duk_str& operator== (const std::string&) const;
		duk_str& operator== (const duk_str&) const;
	};

	/// tools to pick at DukTape's global table
	namespace env
	{
		/// puts whatever is on top of the stack somewhere into the global namespace
		/// happily splits up names with '.' in them - otherwise it'd redumdant
		/// creates whatever tables it needs to along the way
		/// throws an error if it finds a pre-existing value
		void assign(duk_context* ctx, const char* key);

		/// checks the global namespace for a non null_or_undefined value at path
		bool exists(duk_context* ctx, const char* path);

		/// reads something from the global namespace and pushes it on the stack
		/// happily splits up names with '.' in them - otherwise it'd redumdant
		/// happily pushes undefined if there's no value there
		void lookup(duk_context* ctx, const char* binding);

		/// drops the value at key from the global namespace
		/// happily splits up names with '.' in them - otherwise it'd redumdant
		/// leaves empty containers et-al in place
		/// bails if there's no key
		/// really-really calls the delete function (instead of assigning null/undef to it)
		/// throws an error iff nothing exists there
		void remove(duk_context* ctx, const char* path);
	};
};
#endif // ... okay - that's the end of predef

namespace peterlavalle {
namespace diskio {

	/// a script class
	// script C++ classes are really just wrappers to access the ECMAScript implementation
	class ChangeListener
	{
		/// used for const-char wrapping
		duk_context* Host(void) { return reinterpret_cast<scad40::duk_ptr<ChangeListener>*>(this)->Host(); }
	public:

		/// the user's requested members
			void fileChanged (const scad40::duk_str& path);

		/// alternative const char* interfaces
			inline void fileChanged (const char* path)
			{
				fileChanged(
					scad40::duk_str(Host(), path)
				);
			}

		/// create an instance of a scripted class that extends this class
		static scad40::duk_ptr<ChangeListener> New(duk_context* ctx, const char* subclass);

		/// is the value at the stack index useable as an instance of this class
		static bool As(duk_context* ctx, duk_idx_t idx);

		/// pull whatever is at the stack index into C++
		static scad40::duk_ptr<ChangeListener> To(duk_context* ctx, duk_idx_t idx);
	};

	/// a native class
	struct Reading : public scad40::_object
	{
		/// the Reading constructor
		/// ... the user must implement this
		Reading(void);

		Reading(const Reading&) = delete;
		Reading& operator= (const Reading&) = delete;

		/// the user's requested members
		/// the user must implement these
			int8_t read ();
			void close ();
			bool endOfFile ();
			float _number;
			scad40::duk_str _path;

		/// alternative const char* interfaces

		/// the Reading destructor
		/// the user must implement this
		~Reading(void);

		/// queries if the passed index is a Reading object
		static bool Is(duk_context* ctx, duk_idx_t idx);

		/// creates a new Reading object and returns a magical pointer to it
		static scad40::duk_ref<Reading> New(duk_context* ctx);

		/// pulls the the passed index into a Reading object
		/// ... if the passed index is not a Reading object - behaviour is undefined
		static scad40::duk_ref<Reading> To(duk_context* ctx, duk_idx_t idx);
	};

	/// a global class
	struct Disk : public scad40::_object
	{
		/// the Disk constructor
		/// ... the user must implement this
		Disk(void);

		Disk(const Disk&) = delete;
		Disk& operator = (const Disk&) = delete;

		/// the user's requested members
		/// ... the user must implement these
			void foobar (const scad40::duk_str& text);
			scad40::duk_ref<Reading> open (const scad40::duk_str& path);
			scad40::duk_str _pwd;
			void subscribe (const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener);
			void unsubscribe (const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener);

		/// alternative const char* interfaces
			inline void foobar (const char* text)
			{
				foobar(
					scad40::duk_str(Host(), text)
				);
			}
			inline scad40::duk_ref<Reading> open (const char* path)
			{
				return open(
					scad40::duk_str(Host(), path)
				);
			}
			inline void subscribe (const char* path, scad40::duk_ptr<ChangeListener> listener)
			{
				subscribe(
					scad40::duk_str(Host(), path),
					listener
				);
			}
			inline void unsubscribe (const char* path, scad40::duk_ptr<ChangeListener> listener)
			{
				unsubscribe(
					scad40::duk_str(Host(), path),
					listener
				);
			}

		/// the Disk destructor
		/// ... the user must implement this
		~Disk(void);

		/// locates the singleton instance
		static Disk& get(duk_context*);
	};

	/// sets up the tables and calls to this VM
	inline void install(duk_context* ctx)
	{
		auto base = duk_get_top(ctx);

		// >> check for name collisions
		if (scad40::env::exists(ctx, "peterlavalle.diskio"))
		{
			duk_error(ctx, 314, "Can't redefine module `peterlavalle.diskio`");
			return;
		}
		assert(duk_get_top(ctx) == base);

		// >> bind lambdas for native class construction
		{

			// stack -> .... base .. ;

			duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

				duk_error(ctx, 314, "??? STUB ; scad40 needs to create an instance of peterlavalle.diskio.Reading");
				return -1;

			}, 0);
			// stack -> .... base .. ; class:Reading() ;

			scad40::env::assign(ctx, "peterlavalle.diskio.Reading");
			// stack -> .... base .. ;

			assert(duk_get_top(ctx) == base);
		}
		assert(duk_get_top(ctx) == base);

		// >> allocate / in-place-new and store ALL global objects (including context pointers)
		{
			// stack -> .... base .. ;


			// peterlavalle.diskio/Disk
			{
				duk_push_object(ctx);
				// stack -> .... base .. ; [Disk] ;

				peterlavalle::diskio::Disk* thisDisk = (peterlavalle::diskio::Disk*) duk_alloc(ctx, sizeof(peterlavalle::diskio::Disk));
				duk_push_pointer(ctx, thisDisk);
				// stack -> .... base .. ; [Disk] ; *Disk ;

				duk_put_prop_string(ctx, -2, "\xFF" "*Disk");
				// stack -> .... base .. ; [Disk] ;

				scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 0, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {

					assert(ctx == thisDisk->Host());

					thisDisk->~Disk();
					duk_free(ctx, thisDisk);
					// scad40::env::remove(ctx, "peterlavalle.diskio.Disk");
					// TODO ; find a way to actually DO THIS!?!?

					return 0;
				});
				// stack -> .... base .. ; [Disk] ; ~Disk() ;

				duk_set_finalizer(ctx, -2);
				// stack -> .... base .. ; [Disk] ;


				// def foobar(text: string): void
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 1, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->foobar(
							scad40::duk_str(ctx, 0)
						);
						return 0;
					});
					duk_put_prop_string(ctx, -2, "foobar");


				// def open(path: string): Reading
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 1, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						auto result = thisDisk->open(
							scad40::duk_str(ctx, 0)
						);
						result.Push();
						return 1;
					});
					duk_put_prop_string(ctx, -2, "open");

				// var pwd: string
					duk_push_string(ctx, "pwd");
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 3, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->_pwd.Push();
						return 1;
					});
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 3, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->_pwd = scad40::duk_str(ctx, 0);
						return 0;
					});
					duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_ENUMERABLE);

				// def subscribe(path: string, listener: ChangeListener): void
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 2, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->subscribe(
							scad40::duk_str(ctx, 0),
							scad40::duk_ptr<peterlavalle::diskio::ChangeListener>(ctx, 1)
						);
						return 0;
					});
					duk_put_prop_string(ctx, -2, "subscribe");

				// def unsubscribe(path: string, listener: ChangeListener): void
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 2, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->unsubscribe(
							scad40::duk_str(ctx, 0),
							scad40::duk_ptr<peterlavalle::diskio::ChangeListener>(ctx, 1)
						);
						return 0;
					});
					duk_put_prop_string(ctx, -2, "unsubscribe");

				assert(duk_get_top(ctx) == 1 + base);

				// stack -> .... base .. ; [Disk] ;

				scad40::env::assign((*reinterpret_cast<duk_context**>(thisDisk) = ctx), "peterlavalle.diskio.Disk");
				// stack -> .... base .. ;

				new (thisDisk) peterlavalle::diskio::Disk();

				assert(duk_get_top(ctx) == base);
			}

		}
		assert(duk_get_top(ctx) == base);
	}
}
}

#ifndef _scad40_tail
#define _scad40_tail
#pragma region "scad40 and scad40::env"
inline bool scad40::has_prop(duk_context* ctx, const duk_idx_t idx, const char* key)
{
	// stack -> .. idx ... ;

	duk_get_prop_string(ctx, idx, key);
	// stack -> .. idx ... ; value ;

	const bool result = duk_is_null_or_undefined(ctx, -1) ? false : true;

	duk_pop(ctx);
	// stack -> .. idx ... ;

	return result;
}

template<typename T>
inline void scad40::push_selfie<T>(duk_context* ctx, T* self, duk_idx_t nargs, duk_ret_t(*code)(duk_context*, T*))
{
	duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {
		duk_push_current_function(ctx);
		duk_get_prop_string(ctx, -1, "\xFF" "self");
		duk_get_prop_string(ctx, -2, "\xFF" "code");
		auto self = (T*)duk_to_pointer(ctx, -2);
		auto code = (duk_ret_t(*)(duk_context*, T*))duk_to_pointer(ctx, -1);
		duk_pop_3(ctx);
		return code(ctx, self);
	}, nargs);
	duk_push_pointer(ctx, self);
	duk_put_prop_string(ctx, -2, "\xFF" "self");
	duk_push_pointer(ctx, code);
	duk_put_prop_string(ctx, -2, "\xFF" "code");
}

inline void scad40::env::assign(duk_context* ctx, const char* key)
{
	size_t idx = 0, len = 0;

	const auto val = duk_get_top(ctx);
	// stack -> ... ; val ;

	duk_push_global_object(ctx);
	// stack -> ... ; val ; [global host] ;

	while (key[idx + len])
	{
		// stack -> ... ; val ; [host] ;

		if ('.' != key[idx + len])
		{
			++len;
		}
		else
		{
			duk_push_lstring(ctx, key + idx, len);
			// stack -> ... ; val ; [outer host] ; "key" ;

			duk_get_prop(ctx, -2);
			// stack -> ... ; val ; [outer host] ; ?[inner host]? ;

			if (duk_is_null_or_undefined(ctx, -1))
			{
				// stack -> ... ; val ; [outer host] ; <undefined> ;

				duk_pop(ctx);
				// stack -> ... ; val ; [outer host] ;

				duk_push_object(ctx);
				// stack -> ... ; val ; [outer host] ; [inner host] ;

				duk_push_lstring(ctx, key + idx, len);
				// stack -> ... ; val ; [outer host] ; [inner host] ; "key" ;

				duk_dup(ctx, -2);
				// stack -> ... ; val ; [outer host] ; [inner host] ; "key" ; [inner host] ;

				duk_put_prop(ctx, -4);
				// stack -> ... ; val ; [outer host] ; [inner host] ;
			}
			else if (!duk_is_object(ctx, -1))
			{
				duk_push_lstring(ctx, key, idx + len);
				duk_error(ctx, 314, "Collision `%s`", duk_to_string(ctx, -1));
				return;
			}

			// stack -> ... ; val ; [outer host] ; [inner host] ;

			duk_remove(ctx, -2);
			// stack -> ... ; val ; [inner host] ;

			idx = idx + len + 1;
			len = 0;
		}
	}

	// stack -> ... ; val ; [host] ;

	duk_push_lstring(ctx, key + idx, len);
	// stack -> ... ; val ; [host] ; "key" ;

	duk_dup(ctx, -3);
	// stack -> ... ; val ; [host] ; "key" ; val ;

	duk_remove(ctx, -4);
	// stack -> ... ; [host] ; "key" ; val ;

	duk_put_prop(ctx, -3);
	// stack -> ... ; [host] ;

	duk_pop(ctx);
	// stack -> ... ;

}

inline bool scad40::env::exists(duk_context* ctx, const char* path)
{
	// stack -> .... base .. ;

	scad40::env::lookup(ctx, path);
	// stack -> .... base .. ; ??? ;

	const auto result = duk_is_null_or_undefined(ctx, -1) ? false : true;

	duk_pop(ctx);
	// stack -> .... base .. ;

	return result;
}

inline void scad40::env::lookup(duk_context* ctx, const char* binding)
{
	size_t idx = 0, len = 0;

	auto base = duk_get_top(ctx);
	// stack -> .... base .. ;

	duk_push_global_object(ctx);
	// stack -> .... base .. ; [global host] ;

	while (binding[idx + len])
	{
		// stack -> .... base .. ; [host] ;
		if ('.' != binding[idx + len])
		{
			++len;
		}
		else
		{
			duk_push_lstring(ctx, binding + idx, len);
			// stack -> .... base .. ; [outer host] ; "key" ;

			assert((2 + base) == duk_get_top(ctx));
			duk_get_prop(ctx, -2);
			// stack -> .... base .. ; [outer host] ; ?[inner host]? ;

			if (duk_is_null_or_undefined(ctx, -1))
			{
				duk_pop_2(ctx);
				// stack -> .... base .. ;

				assert(base == duk_get_top(ctx));
				duk_push_undefined(ctx);
				// stack -> .... base .. ; <undefined> ;

				return;
			}

			// stack -> .... base .. ; [outer host] ; [inner host] ;

			duk_remove(ctx, base);
			// stack -> .... base .. ; [inner host] ;

			idx = idx + len + 1;
			len = 0;
		}
	}

	assert(0 != len);
	// stack -> .... base .. ; [host] ;

	duk_get_prop_string(ctx, -1, binding + idx);
	// stack -> .... base .. ; [host] ; val ;

	duk_remove(ctx, -2);
	// stack -> .... base .. ; val ;
}

inline void scad40::env::remove(duk_context* ctx, const char* binding)
{
	size_t idx = 0, len = 0;

	auto base = duk_get_top(ctx);
	// stack -> .... base .. ;

	duk_push_global_object(ctx);
	// stack -> .... base .. ; [global host] ;

	while (binding[idx + len])
	{
		// stack -> .... base .. ; [host] ;
		if ('.' != binding[idx + len])
		{
			++len;
		}
		else
		{
			const char* key = duk_push_lstring(ctx, binding + idx, len);
			// stack -> .... base .. ; [outer host] ; "key" ;

			assert((2 + base) == duk_get_top(ctx));
			duk_get_prop(ctx, -2);
			// stack -> .... base .. ; [outer host] ; ?[inner host]? ;

			if (duk_is_null_or_undefined(ctx, -1))
			{
				duk_pop_2(ctx);
				// stack -> .... base .. ;

				return;
			}

			// stack -> .... base .. ; [outer host] ; [inner host] ;

			duk_remove(ctx, base);
			// stack -> .... base .. ; [inner host] ;

			idx = idx + len + 1;
			len = 0;
		}
	}

	assert(0 != len);
	assert(0 == binding[idx + len]);
	assert((base + 1) == duk_get_top(ctx));
	// stack -> .... base .. ; [host] ;

	duk_del_prop_string(ctx, -1, binding + idx);
	// stack -> .... base .. ; [host] ;

	duk_pop(ctx);
	// stack -> .... base .. ;
}
#pragma endregion

#pragma region "object / handle"
inline scad40::_object::_object(duk_context* ctx) :
	_ctx(ctx)
{
	assert(nullptr != _ctx);
}

inline scad40::_handle::_handle(const scad40::_handle& other) : scad40::_object(other)
{
	duk_push_global_stash(_ctx);
	duk_get_prop_string(Host(), -1, other.KeyString().data());
	duk_put_prop_string(Host(), -2, KeyString().data());
	duk_pop(_ctx);
}

inline duk_context* scad40::_object::Host(void) const
{
	return _ctx;
}

inline bool scad40::_handle::IsNull(void) const
{
	// stack -> ... ;

	duk_push_global_stash(Host());
	// stack -> ... ; [global stash] ;

	duk_get_prop_string(Host(), -1, KeyString().data());
	// stack -> ... ; [global stash] ; ??thing?? ;

	const bool result = duk_is_null_or_undefined(Host(), -1) ? true : false;

	duk_pop_2(Host());
	// stack -> ... ;

	return result;
}

inline std::array<char, scad40__pre_strlen + (sizeof(void*) * 2) + 1> scad40::_handle::KeyString(void) const
{
	assert(strlen(scad40__pre_string) == scad40__pre_strlen);
	std::array<char, scad40__pre_strlen + (sizeof(void*) * 2) + 1> result;

	strcpy(result.data(), scad40__pre_string);
	size_t write = scad40__pre_strlen;

	union {
		uint8_t _chars[(sizeof(void*) * 2)];
		void* _cast;
	} swang;

	swang._cast = const_cast<void*>(reinterpret_cast<const void*>(this));

	for (size_t i = 0; i < sizeof(void*); ++i)
	{
		uint8_t pair = swang._chars[i];

		result[write++] = 'A' + (pair & 0x0F);
		result[write++] = 'A' + ((pair & 0xF0) >> 4);
	}

	result[write] = '\0';

	return result;
}

inline void scad40::_handle::Push(void) const
{
	duk_push_global_stash(Host());
	duk_get_prop_string(Host(), -1, KeyString().data());
	duk_remove(Host(), -2);
}

inline scad40::_handle& scad40::_handle::operator=(const scad40::_handle& other)
{
	assert(Host() == other.Host());

	duk_push_global_stash(_ctx);
	duk_get_prop_string(Host(), -1, other.KeyString().data());
	duk_put_prop_string(Host(), -2, KeyString().data());
	duk_pop(_ctx);

	return *this;
}
#pragma endregion

#pragma region "duk_str"
inline scad40::duk_str::duk_str(duk_context* ctx, const char* str) :
	scad40::_native<const char>(ctx)
{
	duk_push_global_stash(ctx);
	if (nullptr == str)
	{
		duk_push_undefined(ctx);
	}
	else
	{
		duk_push_string(ctx, str);
	}
	duk_put_prop_string(ctx, -2, KeyString().data());
	duk_pop(ctx);
}

inline scad40::duk_str::duk_str(duk_context* ctx, duk_idx_t idx) :
	scad40::_native<const char>(ctx)
{
	assert(duk_is_string(ctx, idx));

	idx = duk_normalize_index(ctx, idx);

	// stack -> ... ; "val" ; ... ;

	duk_push_global_stash(ctx);
	// stack -> ... ; "val" ; ... ; [global stash] ;

	duk_dup(ctx, idx);
	// stack -> ... ; "val" ; ... ; [global stash] ; "val" ;

	duk_put_prop_string(ctx, -2, KeyString().data());
	// stack -> ... ; "val" ; ... ; [global stash] ;

	duk_pop(ctx);
	// stack -> ... ; "val" ; ... ;
}
#pragma endregion

#pragma region "duk_ref"
template<typename T>
inline scad40::duk_ref<T>::duk_ref(duk_context* ctx, const duk_idx_t idx) :
	scad40::_native<T>(ctx)
{
	assert(T::Is(ctx, idx));
	// stack -> ... ; [T] ; ... ;

	duk_get_prop_string(ctx, idx, "\xFF" "*");
	// stack -> ... ; [T] ; ... ; T* ;

	_pointer = (T*)duk_to_pointer(ctx, -1);

	duk_pop(ctx);
	// stack -> ... ; [T] ; ... ;

	duk_push_global_stash(ctx);
	// stack -> ... ; [T] ; ... ; [global stash] ;

	duk_dup(ctx, idx);
	// stack -> ... ; [T] ; ... ; [global stash] ; [T] ;

	duk_put_prop_string(ctx, -2, KeyString().data());
	// stack -> ... ; [T] ; ... ; [global stash] ;

	duk_pop(ctx);
	// stack -> ... ; [T] ; ... ;
}
#pragma endregion
#endif // ... okay - that's the end of predef

inline std::ostream& operator<<(std::ostream& ostream, const scad40::duk_str& string)
{
	return ostream << static_cast<const char*>(string);
}


// =====================================================================================================================
// boilerplate usercode implementations - these things wrap/cast/adapt stuff for your "real" methods
// ---------------------------------------------------------------------------------------------------------------------

#pragma region "script ChangeListener"
inline void peterlavalle::diskio::ChangeListener::fileChanged(const scad40::duk_str& path)
{
	auto ptr = reinterpret_cast<scad40::duk_ptr<ChangeListener>*>(this);

	assert(Host() == ptr->Host() && "SAN failed");

	assert(Host() == path.Host());
	const auto base = duk_get_top(Host());

	// stack -> .. base .. ;

	ptr->Push();
	// stack -> .. base .. ; [self] ;

#ifdef _DEBUG
	const auto is_object = duk_is_object(Host(), -1) ? true : false;
	const auto is_null = ptr->IsNull();

	duk_get_prop_string(Host(), -1, "fileChanged");
	// stack -> .. base .. ; [self] ; fileChanged() ;

	auto is_function = duk_is_function(Host(), -1) ? true : false;

	duk_pop(Host());
	// stack -> .. base .. ; [self] ;
#endif

	duk_push_string(Host(), "fileChanged");
	// stack -> .. base .. ; [self] ; "fileChanged" ;

	path.Push();
	// stack -> .. base .. ; [self] ; "fileChanged" ; "path" ;

#ifndef _DEBUG
	duk_call_prop(Host(), -3, 1);
#else
	auto result = duk_pcall_prop(Host(), -3, 1);
	if (DUK_EXEC_SUCCESS != result)
	{
		const char* message = duk_safe_to_string(Host(), -1);
		std::cerr << "Failed to call `peterlavalle.diskio/ChangeListener::fileChanged()` because\n\t" << message << std::endl;
		duk_error(Host(), 314, "Failed to call `peterlavalle.diskio/ChangeListener::fileChanged()` because\n\t%s", message);
	}
#endif
	// stack -> .. base .. ; ?result? ;

	duk_pop(Host());
	return;
}

inline scad40::duk_ptr<peterlavalle::diskio::ChangeListener> peterlavalle::diskio::ChangeListener::New(duk_context* ctx, const char* subclass)
{
	// stack -> ... ;

	scad40::env::lookup(ctx, subclass);
	// stack -> ... ; ?class? ;

	if (!duk_is_function(ctx, -1))
	{
		duk_pop(ctx);
		duk_error(ctx, 314, "Thing `%s` is not a function", subclass);
	}

	// stack -> ... ; class() ;

	duk_new(ctx, 0);
	assert(duk_is_object(ctx, -1) && "SAN failed");
	// stack -> ... ; ?object? ;

	if (!peterlavalle::diskio::ChangeListener::As(ctx, -1))
	{
		duk_pop(ctx);
		duk_error(ctx, 314, "Thing `%s` is not usable as peterlavalle::diskio::ChangeListener", subclass);
	}

	// stack -> ... ; object ;

	scad40::duk_ptr<peterlavalle::diskio::ChangeListener> object(ctx, -1);
	duk_pop(ctx);
	// stack -> ... ;

	assert(!object.IsNull());

	return object;
}

inline bool peterlavalle::diskio::ChangeListener::As(duk_context* ctx, duk_idx_t idx)
{
	// stack -> ... ; idx .. base .. ;

	//
	// check each function / member ... not sure what to do about values

	duk_get_prop_string(ctx, idx, "fileChanged");
	// stack -> ... ; idx .. base .. ; ?fileChanged() ;

	if (duk_is_function(ctx, -1))
		duk_pop(ctx);
	else
	{
		duk_pop(ctx);
		return false;
	}




	return true;
}

inline scad40::duk_ptr<peterlavalle::diskio::ChangeListener> peterlavalle::diskio::ChangeListener::To(duk_context* ctx, duk_idx_t idx)
{
	return scad40::duk_ptr<peterlavalle::diskio::ChangeListener>(ctx, idx);
}




#pragma endregion

#pragma region "native Reading"
inline bool peterlavalle::diskio::Reading::Is(duk_context* ctx, duk_idx_t idx)
{
	// stack -> ... ; ?[T]? ;
	if (!duk_is_object(ctx, idx))
	{
		return false;
	}

	duk_get_prop_string(ctx, idx, "\xFF" "typeid().name()");
	// stack -> ... ; ?[T]? ; ?"Reading"? ;

	const bool matches = strcmp(typeid(peterlavalle::diskio::Reading).name(), duk_to_string(ctx, -1)) ? false : true;

	duk_pop(ctx);
	// stack -> ... ; ?[T]? ;

	return matches;
}

inline scad40::duk_ref<peterlavalle::diskio::Reading> peterlavalle::diskio::Reading::New(duk_context* ctx)
{
	Reading* thisReading = (Reading*)duk_alloc(ctx, sizeof(Reading));

	// stack -> ... ;

	duk_push_object(ctx);
	// stack -> ... ; [Reading] ;

	scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
		thisReading->~Reading();
		duk_free(ctx, thisReading);
		return 0;
	});
	// stack -> ... ; [Reading] ; ~Reading() ;

	duk_set_finalizer(ctx, -2);
	// stack -> ... ; [Reading] ;

	duk_push_string(ctx, typeid(peterlavalle::diskio::Reading).name());
	// stack -> ... ; [Reading] ; typeid( peterlavalle::diskio::Reading ).name() ;

	duk_put_prop_string(ctx, -2, "\xFF" "typeid().name()");
	// stack -> ... ; [Reading] ;

	duk_push_pointer(ctx, thisReading);
	// stack -> ... ; [Reading] ; *Reading ;

	duk_put_prop_string(ctx, -2, "\xFF" "*");
	// stack -> ... ; [Reading] ;

	{
		// def read(): sint8
		scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

			assert(false && "??? scad40 needs to provide this");
			return -1;

		});
		duk_put_prop_string(ctx, -2, "read");

		// def close(): void
		scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

			assert(false && "??? scad40 needs to provide this");
			return -1;

		});
		duk_put_prop_string(ctx, -2, "close");

		// def endOfFile(): bool
		scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

			assert(false && "??? scad40 needs to provide this");
			return -1;

		});
		duk_put_prop_string(ctx, -2, "endOfFile");

		// var number: single
		// property name
		duk_push_string(ctx, "number");
		// getter
		scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 3, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
			duk_push_number(ctx, (duk_double_t)(thisReading->_number));
			return 1;
		});
		// setter
		scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 3, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
			thisReading->_number = (float)duk_to_number(ctx, 0);
			return 0;
		});
		// assign
		duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_ENUMERABLE);

		// val path: string
		// property name
		duk_push_string(ctx, "path");
		// getter
		scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 3, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
			thisReading->_path.Push();
			return 1;
		});
		// assign
		duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);
	}


	// stack -> ... ; [Reading] ;
	*reinterpret_cast<duk_context**>(thisReading) = ctx;
	auto t = duk_get_top(ctx);
	new (thisReading)Reading();
	assert(t == duk_get_top(ctx));
	auto ret = scad40::duk_ref<Reading>(ctx, -1);
	assert((t) == duk_get_top(ctx));
	// stack -> ... ; [Reading] ;
	duk_pop(ctx);

	assert(!ret.IsNull());
	assert(nullptr != ret.operator ->());

	return ret;
}

inline scad40::duk_ref<peterlavalle::diskio::Reading> peterlavalle::diskio::Reading::To(duk_context* ctx, duk_idx_t idx)
{
	if (!peterlavalle::diskio::Reading::Is(ctx, idx))
	{
		duk_error(ctx, 314, "Tried to grab `%s` as a {Reading} (which it is not)", duk_to_string(ctx, idx));
	}
	return scad40::duk_ref< peterlavalle::diskio::Reading >(ctx, idx);
}
#pragma endregion

#pragma region "global Disk"
inline peterlavalle::diskio::Disk& peterlavalle::diskio::Disk::get(duk_context* ctx)
{
	auto base = duk_get_top(ctx);

	// stack -> .... base .. ;

	scad40::env::lookup(ctx, "peterlavalle.diskio.Disk"); assert((1 + base) == duk_get_top(ctx));
	// stack -> .... base .. ; [Disk] ;

	duk_get_prop_string(ctx, -1, "\xFF" "*Disk"); assert((2 + base) == duk_get_top(ctx));
	// stack -> .... base .. ; [Disk] ; Disk[*] ;

	auto ptrDisk = reinterpret_cast<peterlavalle::diskio::Disk*>(duk_to_pointer(ctx, -1));
	duk_pop_2(ctx);
	// stack -> .... base .. ;

	assert(base == duk_get_top(ctx));

	return *ptrDisk;
}
#pragma endregion
