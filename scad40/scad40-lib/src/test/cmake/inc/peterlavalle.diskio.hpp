#pragma once

///
/// These classes are used to bridge-the-gap between Scala-JS and C++
/// ... they're not thread-safe but then ... neither is DukTape
/// ... all modules (should) use the same definition of them - so don't worry about namespace collisions
#ifndef _scad40_head
#define _scad40_head

#include <duktape.h>

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
	void push_selfie(duk_context* ctx, T* self, duk_idx_t nargs, duk_ret_t(*code)(duk_context*, T*));

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
		_object(duk_context* ctx) :
			_ctx(ctx)
		{
			assert(nullptr != _ctx);
		}

	protected:
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

		/// usercode will laways use this method to invoke the parent
		_object(void)
		{
			/// this will/should have been magically setup
			assert(nullptr != _ctx);
		}

	public:
		duk_context* Host(void) const
		{
			return _ctx;
		}
	};

	/// this refers to a thing in script-land but is not itself a thing
	/// it's used for the duk_(ref|str|ptr) classes
	/// Google calls it a handle - https://developers.google.com/v8/embed?hl=en
	struct _handle : scad40::_object
	{
		/// pushes (a reference to) the value onto the stack (or maybe null if the value is null)
		void Push(void) const
		{
			duk_push_global_stash(_ctx);
			// stack -> ... ; [stash] ;

			// because we're using the pointer as a GUID this is acceptable
			duk_push_pointer(_ctx, const_cast<scad40::_handle*>(this));
			// stack -> ... ; [stash] ; *this ;

			duk_get_prop(_ctx, -2);
			// stack -> ... ; [stash] ; value ;

			duk_remove(_ctx, -2);
			// stack -> ... ; value ;
		}

		bool IsNull(void) const
		{
			// stack -> ... ;

			duk_push_global_stash(Host());
			// stack -> ... ; [global stash] ;

			duk_push_pointer(_ctx, const_cast<scad40::_handle*>(this));
			// stack -> ... ; [global stash] ; *this ;

			duk_get_prop(Host(), -2);
			// stack -> ... ; [global stash] ; value ;

			const bool result = duk_is_null_or_undefined(Host(), -1) ? true : false;

			duk_pop_2(Host());
			// stack -> ... ;

			return result;
		}

	protected:

		/// making this protected approaximates an abstract class without introducting method-tables
		/// ... introducting method-tables would kill us in evil little ways
		_handle(duk_context* ctx) : scad40::_object(ctx) { }

		/// partially expose the copy constructor to allow subclasses copy instances
		_handle(const scad40::_handle& that) : scad40::_object(that)
		{
			duk_push_global_stash(_ctx);
			// stack -> ... ; [stash] ;

			duk_push_pointer(_ctx, this);
			// stack -> ... ; [stash] ; *this ;

			duk_push_pointer(_ctx, const_cast<scad40::_handle*>(&that));
			// stack -> ... ; [stash] ; *this ; *that ;

			duk_get_prop(_ctx, -3);
			// stack -> ... ; [stash] ; *this ; value ;

			duk_put_prop(_ctx, -3);
			// stack -> ... ; [stash] ;

			duk_pop(_ctx);
		}

		/// partially expose the copy operator to allow subclasses copy instances
		_handle& operator=(const scad40::_handle& that)
		{
			assert(_ctx == that._ctx);

			// stack -> ... ;

			duk_push_global_stash(_ctx);
			// stack -> ... ; [stash] ;

			duk_push_pointer(_ctx, const_cast<scad40::_handle*>(this));
			// stack -> ... ; [stash] ; *this ;

			duk_push_pointer(_ctx, const_cast<scad40::_handle*>(&that));
			// stack -> ... ; [stash] ; *this ; *that ;

			duk_get_prop(_ctx, -3);
			// stack -> ... ; [stash] ; *this ; value ;

			duk_put_prop(_ctx, -3);
			// stack -> ... ; [stash] ;

			duk_pop(_ctx);
			// stack -> ... ;

			return *this;
		}

		~_handle(void)
		{
			if (nullptr == _ctx)
			{
				return;
			}

			// stack -> ... ;

			duk_push_global_stash(_ctx);
			// stack -> ... ; [global stash] ;

			duk_push_pointer(_ctx, this);
			// stack -> ... ; [global stash] ; *this ;

			duk_del_prop(_ctx, -2);
			// stack -> ... ; [global stash] ;

			duk_pop(_ctx);
			// stack -> ... ;

			_ctx = nullptr;
		}
	};

	/// this allows manipulating a pure-script object from C++ using a predefined interface
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

			duk_push_pointer(ctx, this);
			// stack -> ... ; idx ... ; [stash] ; *this ;

			duk_dup(ctx, idx);
			// stack -> ... ; idx ... ; [stash] ; *this ; [val] ;

			duk_put_prop(ctx, -3);
			// stack -> ... ; idx ... ; [stash] ;

			duk_pop(ctx);
			// stack -> ... ; idx ... ;
		}

		T* operator->(void) { return reinterpret_cast<T*>(this); }

		const T* operator->(void) const { return reinterpret_cast<T*>(this); }
	};

	/// baseclass used for _handle things with a native pointer
	/// ... which is really just an optimization for strings and native objects ...
	template<typename T>
	struct _native : scad40::_handle
	{

	protected:
		T* _pointer;
		_native(duk_context* ctx) : scad40::_handle(ctx) { }

	public:
		T* operator->(void) { return _pointer; }
		const T* operator->(void) const { return _pointer; }
	};

	/// holds a ref to a C++ object built for duktape using magic
	/// the pointer is updated on copy so that you get a bit faster access to it
	template<typename T>
	struct duk_native : public scad40::_native<T>
	{
	public:

		/// grab an instance from the stack. fails violently if types are wrong
		duk_native(duk_context* ctx, duk_idx_t idx) :
			scad40::_native<T>(ctx)
		{
			idx = duk_normalize_index(ctx, idx);
			assert(T::Is(ctx, idx));
			// stack -> ... ; [T] ; ... ;

			duk_get_prop_string(ctx, idx, "\xFF" "*");
			// stack -> ... ; [T] ; ... ; T* ;

			_pointer = (T*)duk_to_pointer(ctx, -1);

			duk_pop(ctx);
			// stack -> ... ; [T] ; ... ;

			duk_push_global_stash(ctx);
			// stack -> ... ; [T] ; ... ; [stash] ;

			duk_push_pointer(ctx, this);
			// stack -> ... ; [T] ; ... ; [stash] ; *this ;

			duk_dup(ctx, idx);
			// stack -> ... ; [T] ; ... ; [stash] ; *this ; [T] ;

			duk_put_prop(ctx, -3);
			// stack -> ... ; [T] ; ... ; [stash] ;

			duk_pop(ctx);
			// stack -> ... ; [T] ; ... ;
		}

	};

	/// holds a ref to a duktape string using magic
	/// as much as possible I stick strings into duktape and try not to think too hard about them
	/// the pointer is updated on copy so that you get a bit faster access to it
	class duk_string : public scad40::_native<const char>
	{
	public:
		const char* c_str(void) const { return _pointer; }
		operator std::string (void) const { return _pointer; }

		/// create an instance and set it to point to the passed string
		duk_string(duk_context* ctx, const char* str = nullptr) :
			scad40::_native<const char>(ctx)
		{
			duk_push_global_stash(ctx);
			// stack -> ... ; [stash] ;

			duk_push_pointer(ctx, this);
			// stack -> ... ; [stash] ; *this ;

			if (nullptr == str)
			{
				duk_push_undefined(ctx);
				_pointer = nullptr;
			}
			else
			{
				_pointer = duk_push_string(ctx, str);
			}
			// stack -> ... ; [stash] ; *this ; "_pointer" ;

			duk_put_prop(ctx, -3);
			// stack -> ... ; [stash] ;

			duk_pop(ctx);
			// stack -> ... ;
		}

		/// create an instance and set it to point to the passed string
		duk_string(duk_context* ctx, const std::string&);

		/// grab an instance from the stack
		duk_string(duk_context* ctx, duk_idx_t idx) :
			scad40::_native<const char>(ctx)
		{
			assert(duk_is_string(ctx, idx));

			idx = duk_normalize_index(ctx, idx);

			// stack -> ... ; "val" ; ... ;

			auto stash = duk_get_top(ctx);
			duk_push_global_stash(ctx);
			// stack -> ... ; "val" ; ... ; [stash] ;

			duk_push_pointer(ctx, this);
			// stack -> ... ; "val" ; ... ; [stash] ; *this ;

			_pointer = duk_is_null_or_undefined(ctx, idx) ? nullptr : duk_to_string(ctx, idx);
			if (!_pointer)
				duk_push_undefined(ctx);
			else
				duk_dup(ctx, idx);
			// stack -> ... ; "val" ; ... ; [stash] ; *this ; "val" ;

			duk_put_prop(ctx, stash);
			// stack -> ... ; "val" ; ... ; [stash] ;

			duk_pop(ctx);
			// stack -> ... ; "val" ; ... ;
		}

		duk_string& operator= (const char* value)
		{

			// stack -> ... ;

			auto stash = duk_get_top(Host());
			duk_push_global_stash(Host());
			// stack -> ... ; [stash] ;

			duk_push_pointer(Host(), this);
			// stack -> ... ; [stash] ; *this ;

			duk_push_string(Host(), value);
			// stack -> ... ; [stash] ; *this ; "value" ;

			duk_put_prop(Host(), stash);
			// stack -> ... ; [stash] ;

			duk_pop(Host());
			// stack -> ... ;

			return *this;
		}
		duk_string& operator= (const std::string& other)	{ return *this = other.c_str(); }

		bool operator== (const char* other) const			{ return 0 == strcmp(_pointer, other); }
		bool operator== (const std::string& other) const	{ return 0 == strcmp(_pointer, other.c_str()); }
		bool operator== (const duk_string& other) const		{ return 0 == strcmp(_pointer, other._pointer); }
	};

	/// tools to pick at DukTape's global table like we're in script-land
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

inline std::ostream& operator<<(std::ostream& ostream, const scad40::duk_string& string)
{
	return ostream << string.c_str();
}

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
			void fileChanged (scad40::duk_string& path);

		/// alternative const char* interfaces
			inline void fileChanged (const char* path)
			{
				fileChanged(
					scad40::duk_string(Host(), path)
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
			scad40::duk_string _path;
			void* _thing;

		/// alternative const char* interfaces

		/// the Reading destructor
		/// the user must implement this
		~Reading(void);

		/// queries if the passed index is a Reading object
		static bool Is(duk_context* ctx, duk_idx_t idx);

		/// creates a new Reading object and returns a magical pointer to it
		static scad40::duk_native<Reading> New(duk_context* ctx);

		/// pulls the the passed index into a Reading object
		/// ... if the passed index is not a Reading object - behaviour is undefined
		static scad40::duk_native<Reading> To(duk_context* ctx, duk_idx_t idx);
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
			void foobar (scad40::duk_string& text);
			scad40::duk_native<peterlavalle::diskio::Reading> open (scad40::duk_string& path);
			scad40::duk_string _pwd;
			void* _bar;
			void subscribe (scad40::duk_string& path, scad40::duk_ptr<peterlavalle::diskio::ChangeListener>& listener);
			void unsubscribe (scad40::duk_string& path, scad40::duk_ptr<peterlavalle::diskio::ChangeListener>& listener);

		/// alternative const char* interfaces
			inline void foobar (const char* text)
			{
				foobar(
					scad40::duk_string(Host(), text)
				);
			}
			inline scad40::duk_native<peterlavalle::diskio::Reading> open (const char* path)
			{
				return open(
					scad40::duk_string(Host(), path)
				);
			}
			inline void subscribe (const char* path, scad40::duk_ptr<peterlavalle::diskio::ChangeListener>& listener)
			{
				subscribe(
					scad40::duk_string(Host(), path),
					listener
				);
			}
			inline void unsubscribe (const char* path, scad40::duk_ptr<peterlavalle::diskio::ChangeListener>& listener)
			{
				unsubscribe(
					scad40::duk_string(Host(), path),
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
		const auto idxBase = duk_get_top(ctx);

		// >> check for name collisions
		if (scad40::env::exists(ctx, "peterlavalle.diskio"))
		{
			duk_error(ctx, 314, "Can't redefine module `peterlavalle.diskio`");
			return;
		}

		// >> bind lambdas for native class construction
		{

			// stack -> .... base .. ;

			duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

				auto ptr = peterlavalle::diskio::Reading::New(ctx);

				ptr.Push();

				assert(peterlavalle::diskio::Reading::Is(ctx, -1) && "SAN failed");

				return 1;

			}, 0);
			// stack -> .... base .. ; class:Reading() ;

			scad40::env::assign(ctx, "peterlavalle.diskio.Reading");
			// stack -> .... base .. ;

			assert(duk_get_top(ctx) == idxBase);
		}

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

				duk_put_prop_string(ctx, idxBase, "\xFF" "*Disk");
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

				duk_set_finalizer(ctx, idxBase);
				// stack -> .... base .. ; [Disk] ;


				// def foobar(text: string): void
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 1, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->foobar(
							scad40::duk_string(ctx, 0)
						);

						return 0;
					});
					duk_put_prop_string(ctx, idxBase, "foobar");


				// def open(path: string): Reading
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 1, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						auto result = thisDisk->open(
							scad40::duk_string(ctx, 0)
						);
						result.Push();
						return 1;
					});
					duk_put_prop_string(ctx, idxBase, "open");

				// var pwd: string
					duk_push_string(ctx, "pwd");
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 3, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->_pwd.Push();
						return 1;
					});
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 3, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->_pwd = scad40::duk_string(ctx, 0);
						return 0;
					});
					duk_def_prop(ctx, idxBase, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_ENUMERABLE);

				// def subscribe(path: string, listener: ChangeListener): void
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 2, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->subscribe(
							scad40::duk_string(ctx, 0),
							scad40::duk_ptr<peterlavalle::diskio::ChangeListener>(ctx, 1)
						);

						return 0;
					});
					duk_put_prop_string(ctx, idxBase, "subscribe");

				// def unsubscribe(path: string, listener: ChangeListener): void
					scad40::push_selfie<peterlavalle::diskio::Disk>(ctx, thisDisk, 2, [](duk_context* ctx, peterlavalle::diskio::Disk* thisDisk) -> duk_ret_t {
						thisDisk->unsubscribe(
							scad40::duk_string(ctx, 0),
							scad40::duk_ptr<peterlavalle::diskio::ChangeListener>(ctx, 1)
						);

						return 0;
					});
					duk_put_prop_string(ctx, idxBase, "unsubscribe");

				assert(duk_get_top(ctx) == 1 + idxBase);

				// stack -> .... base .. ; [Disk] ;

				scad40::env::assign(ctx, "peterlavalle.diskio.Disk");
				// stack -> .... base .. ;

				*reinterpret_cast<duk_context**>(thisDisk) = ctx;
				new (thisDisk) peterlavalle::diskio::Disk();

				assert(duk_get_top(ctx) == idxBase);
			}

		}
		assert(duk_get_top(ctx) == idxBase);
	}
}
}

// =====================================================================================================================
// boilerplate usercode implementations - these things wrap/cast/adapt stuff for your "real" methods
// ---------------------------------------------------------------------------------------------------------------------

#pragma region "script ChangeListener"
inline void peterlavalle::diskio::ChangeListener::fileChanged(scad40::duk_string& path)
{
	auto ptr = reinterpret_cast<scad40::duk_ptr<ChangeListener>*>(this);
	auto ctx = Host();

	assert(ctx == ptr->Host() && "SAN failed");
            assert(ctx == path.Host() && "Arg SAN failed");

	const auto base = duk_get_top(ctx);

	// stack -> .. base .. ;

	ptr->Push();
	// stack -> .. base .. ; [self] ;

#ifdef _DEBUG
	const auto is_object = duk_is_object(ctx, -1) ? true : false;
	const auto is_null = ptr->IsNull();

	duk_get_prop_string(ctx, -1, "fileChanged");
	// stack -> .. base .. ; [self] ; fileChanged() ;

	auto is_function = duk_is_function(ctx, -1) ? true : false;

	duk_pop(ctx);
	// stack -> .. base .. ; [self] ;
#endif

	duk_push_string(ctx, "fileChanged");
	// stack -> .. base .. ; [self] ; "fileChanged" ;

	path.Push();

#ifndef _DEBUG
	duk_call_prop(ctx, base, 1);
#else
	auto result = duk_pcall_prop(ctx, base, 1);
	if (DUK_EXEC_SUCCESS != result)
	{
		const char* message = duk_safe_to_string(ctx, -1);
		std::cerr << "Failed to call `peterlavalle.diskio/ChangeListener::fileChanged()` because\n\t" << message << std::endl;
		duk_error(ctx, 314, "Failed to call `peterlavalle.diskio/ChangeListener::fileChanged()` because\n\t%s", message);
	}
#endif
	// stack -> .. base .. ; ?result? ;

	duk_pop(ctx);
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
        // check def fileChanged(path: string): void
            duk_get_prop_string(ctx, idx, "fileChanged");
            // stack -> ... ; idx .. base .. ; ?fileChanged() ;
            if (duk_is_function(ctx, -1))
            {
                duk_pop(ctx);
            }
            else
            {
                duk_pop(ctx);
                return false;
            }
            // stack -> ... ; idx .. base .. ;

    // yeah - it's probably what we want
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

	const char* that = duk_to_string(ctx, -1);
	static const char* name = typeid(peterlavalle::diskio::Reading).name();

	const bool matches = strcmp(name, that) ? false : true;

	duk_pop(ctx);
	// stack -> ... ; ?[T]? ;

	return matches;
}

inline scad40::duk_native<peterlavalle::diskio::Reading> peterlavalle::diskio::Reading::New(duk_context* ctx)
{
	Reading* thisReading = (Reading*)duk_alloc(ctx, sizeof(Reading));

	const auto idxBase = duk_get_top(ctx);
	// stack -> ... ;

	duk_push_object(ctx);
	// stack -> ... ; [Reading] ;

	scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
		thisReading->~Reading();
		duk_free(ctx, thisReading);
		return 0;
	});
	// stack -> ... ; [Reading] ; ~Reading() ;

	duk_set_finalizer(ctx, idxBase);
	// stack -> ... ; [Reading] ;

	duk_push_string(ctx, typeid(peterlavalle::diskio::Reading).name());
	// stack -> ... ; [Reading] ; typeid(peterlavalle::diskio::Reading).name() ;

	duk_put_prop_string(ctx, idxBase, "\xFF" "typeid().name()");
	// stack -> ... ; [Reading] ;

	assert(peterlavalle::diskio::Reading::Is(ctx, -1));

	duk_push_pointer(ctx, thisReading);
	// stack -> ... ; [Reading] ; *Reading ;

	duk_put_prop_string(ctx, idxBase, "\xFF" "*");
	// stack -> ... ; [Reading] ;

	{
		// def read(): sint8
			scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

				auto result = thisReading->read();

				duk_push_int(ctx, result);
				return 1;

			});
			duk_put_prop_string(ctx, idxBase, "read");

		// def close(): void
			scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

				thisReading->close();

				return 0;
			});
			duk_put_prop_string(ctx, idxBase, "close");

		// def endOfFile(): bool
			scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

				auto result = thisReading->endOfFile();

				duk_push_boolean(ctx, result ? 1 : 0);
				return 1;

			});
			duk_put_prop_string(ctx, idxBase, "endOfFile");

		// var number: single
			// property name
				duk_push_string(ctx, "number");
			// getter
				scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 3, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
					auto result = thisReading->_number;
					duk_push_number(ctx, result);
					return 1;
				});
			// setter
				scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 3, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
					thisReading->_number = (float) duk_to_number(ctx, 0);
					return 0;
				});
			// assign
				duk_def_prop(ctx, idxBase, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_ENUMERABLE);

		// val path: string
			// property name
				duk_push_string(ctx, "path");
			// getter
				scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 3, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {
					auto result = thisReading->_path;
					result.Push();
					return 1;
				});
			// assign
				duk_def_prop(ctx, idxBase, DUK_DEFPROP_HAVE_GETTER);
	}


	// stack -> ... ; [Reading] ;
	*reinterpret_cast<duk_context**>(thisReading) = ctx;
	auto t = duk_get_top(ctx);
	new (thisReading)Reading();
	assert(t == duk_get_top(ctx));
	auto ret = scad40::duk_native<Reading>(ctx, -1);
	assert((t) == duk_get_top(ctx));
	// stack -> ... ; [Reading] ;
	duk_pop(ctx);

	assert(ret.operator->() == thisReading);
	assert(!ret.IsNull());
	assert(nullptr != ret.operator ->());

	return ret;
}

inline scad40::duk_native<peterlavalle::diskio::Reading> peterlavalle::diskio::Reading::To(duk_context* ctx, duk_idx_t idx)
{
	if (!peterlavalle::diskio::Reading::Is(ctx, idx))
	{
		duk_error(ctx, 314, "Tried to grab `%s` as a {Reading} (which it is not)", duk_to_string(ctx, idx));
	}
	return scad40::duk_native< peterlavalle::diskio::Reading >(ctx, idx);
}
#pragma endregion

#pragma region "global Disk"
inline peterlavalle::diskio::Disk& peterlavalle::diskio::Disk::get(duk_context* ctx)
{
	auto base = duk_get_top(ctx);

	// stack -> .... base .. ;

	scad40::env::lookup(ctx, "peterlavalle.diskio.Disk");
	// stack -> .... base .. ; [Disk] ;

	duk_get_prop_string(ctx, base, "\xFF" "*Disk");
	// stack -> .... base .. ; [Disk] ; Disk[*] ;

	auto ptrDisk = reinterpret_cast<peterlavalle::diskio::Disk*>(duk_to_pointer(ctx, -1));
	duk_pop_2(ctx);
	// stack -> .... base .. ;

	return *ptrDisk;
}
#pragma endregion
