
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











namespace underlay {

	/// a global class
	struct Log : public scad40::_object
	{
		/// the Log constructor
		/// ... the user must implement this
		Log(void);

		Log(const Log&) = delete;
		Log& operator = (const Log&) = delete;

		/// the user's requested members
		/// ... the user must implement these
			ThreeOut* _ioc;
			void warn (scad40::duk_string& message);
			void info (scad40::duk_string& message);
			void fail (scad40::duk_string& message);
			void note (scad40::duk_string& message);

		/// alternative const char* interfaces
			inline void warn (const char* message)
			{
				warn(
					scad40::duk_string(Host(), message)
				);
			}
			inline void info (const char* message)
			{
				info(
					scad40::duk_string(Host(), message)
				);
			}
			inline void fail (const char* message)
			{
				fail(
					scad40::duk_string(Host(), message)
				);
			}
			inline void note (const char* message)
			{
				note(
					scad40::duk_string(Host(), message)
				);
			}

		/// the Log destructor
		/// ... the user must implement this
		~Log(void);

		/// locates the singleton instance
		static Log& get(duk_context*);
	};

	/// a native class
	struct Pawn : public scad40::_object
	{
		/// the Pawn constructor
		/// ... the user must implement this
		Pawn(void);

		Pawn(const Pawn&) = delete;
		Pawn& operator= (const Pawn&) = delete;

		/// the user's requested members
		/// the user must implement these
			buzzbird<>::pawn* _ioc;
			std::function<void(void)> _onDed;
			void detach ();
			void notify (scad40::duk_string& message);

		/// alternative const char* interfaces
			inline void notify (const char* message)
			{
				notify(
					scad40::duk_string(Host(), message)
				);
			}

		/// the Pawn destructor
		/// the user must implement this
		~Pawn(void);

		/// queries if the passed index is a Pawn object
		static bool Is(duk_context* ctx, duk_idx_t idx);

		/// creates a new Pawn object and returns a magical pointer to it
		static scad40::duk_native<Pawn> New(duk_context* ctx);

		/// pulls the the passed index into a Pawn object
		/// ... if the passed index is not a Pawn object - behaviour is undefined
		static scad40::duk_native<Pawn> To(duk_context* ctx, duk_idx_t idx);
	};

	/// a native class
	struct Soul : public scad40::_object
	{
		/// the Soul constructor
		/// ... the user must implement this
		Soul(void);

		Soul(const Soul&) = delete;
		Soul& operator= (const Soul&) = delete;

		/// the user's requested members
		/// the user must implement these
			buzzbird<>::soul* _ioc;
			std::function<void(void)> _onDed;
			void remove ();

		/// alternative const char* interfaces

		/// the Soul destructor
		/// the user must implement this
		~Soul(void);

		/// queries if the passed index is a Soul object
		static bool Is(duk_context* ctx, duk_idx_t idx);

		/// creates a new Soul object and returns a magical pointer to it
		static scad40::duk_native<Soul> New(duk_context* ctx);

		/// pulls the the passed index into a Soul object
		/// ... if the passed index is not a Soul object - behaviour is undefined
		static scad40::duk_native<Soul> To(duk_context* ctx, duk_idx_t idx);
	};

	/// a global class
	struct Owner : public scad40::_object
	{
		/// the Owner constructor
		/// ... the user must implement this
		Owner(void);

		Owner(const Owner&) = delete;
		Owner& operator = (const Owner&) = delete;

		/// the user's requested members
		/// ... the user must implement these
			scad40::duk_native<underlay::Soul> ofPawn (scad40::duk_native<underlay::Pawn>& pawn);

		/// alternative const char* interfaces

		/// the Owner destructor
		/// ... the user must implement this
		~Owner(void);

		/// locates the singleton instance
		static Owner& get(duk_context*);
	};


	/// sets up the tables and calls to this VM
	inline void install(duk_context* ctx)
	{
		const auto idxBase = duk_get_top(ctx);

		// >> check for name collisions
		if (scad40::env::exists(ctx, "underlay"))
		{
			duk_error(ctx, 314, "Can't redefine module `underlay`");
			return;
		}

		// >> bind lambdas for native class construction
		{

			// stack -> .... base .. ;

			duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

				auto ptr = underlay::Pawn::New(ctx);

				ptr.Push();

				assert(underlay::Pawn::Is(ctx, -1) && "SAN failed");

				return 1;

			}, 0);
			// stack -> .... base .. ; class:Pawn() ;

			scad40::env::assign(ctx, "underlay.Pawn");
			// stack -> .... base .. ;

			assert(duk_get_top(ctx) == idxBase);
		}
		{

			// stack -> .... base .. ;

			duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

				auto ptr = underlay::Soul::New(ctx);

				ptr.Push();

				assert(underlay::Soul::Is(ctx, -1) && "SAN failed");

				return 1;

			}, 0);
			// stack -> .... base .. ; class:Soul() ;

			scad40::env::assign(ctx, "underlay.Soul");
			// stack -> .... base .. ;

			assert(duk_get_top(ctx) == idxBase);
		}

		// >> allocate / in-place-new and store ALL global objects (including context pointers)
		{
			// stack -> .... base .. ;
			// underlay/Log
			{
				duk_push_object(ctx);
				// stack -> .... base .. ; [Log] ;

				underlay::Log* thisLog = (underlay::Log*) duk_alloc(ctx, sizeof(underlay::Log));
				duk_push_pointer(ctx, thisLog);
				// stack -> .... base .. ; [Log] ; *Log ;

				duk_put_prop_string(ctx, idxBase, "\xFF" "*Log");
				// stack -> .... base .. ; [Log] ;

				scad40::push_selfie<underlay::Log>(ctx, thisLog, 0, [](duk_context* ctx, underlay::Log* thisLog) -> duk_ret_t {

					assert(ctx == thisLog->Host());

					thisLog->~Log();
					duk_free(ctx, thisLog);
					// scad40::env::remove(ctx, "underlay.Log");
					// TODO ; find a way to actually DO THIS!?!?

					return 0;
				});
				// stack -> .... base .. ; [Log] ; ~Log() ;

				duk_set_finalizer(ctx, idxBase);
				// stack -> .... base .. ; [Log] ;

				// def warn(message: string): void
					scad40::push_selfie<underlay::Log>(ctx, thisLog, 1, [](duk_context* ctx, underlay::Log* thisLog) -> duk_ret_t {
						thisLog->warn(
							scad40::duk_string(ctx, 0)
						);
						return 0;
					});
					duk_put_prop_string(ctx, idxBase, "warn");

				// def info(message: string): void
					scad40::push_selfie<underlay::Log>(ctx, thisLog, 1, [](duk_context* ctx, underlay::Log* thisLog) -> duk_ret_t {
						thisLog->info(
							scad40::duk_string(ctx, 0)
						);
						return 0;
					});
					duk_put_prop_string(ctx, idxBase, "info");

				// def fail(message: string): void
					scad40::push_selfie<underlay::Log>(ctx, thisLog, 1, [](duk_context* ctx, underlay::Log* thisLog) -> duk_ret_t {
						thisLog->fail(
							scad40::duk_string(ctx, 0)
						);
						return 0;
					});
					duk_put_prop_string(ctx, idxBase, "fail");

				// def note(message: string): void
					scad40::push_selfie<underlay::Log>(ctx, thisLog, 1, [](duk_context* ctx, underlay::Log* thisLog) -> duk_ret_t {
						thisLog->note(
							scad40::duk_string(ctx, 0)
						);
						return 0;
					});
					duk_put_prop_string(ctx, idxBase, "note");

				assert(duk_get_top(ctx) == 1 + idxBase);

				// stack -> .... base .. ; [Log] ;

				scad40::env::assign(ctx, "underlay.Log");
				// stack -> .... base .. ;

				*reinterpret_cast<duk_context**>(thisLog) = ctx;
				new (thisLog) underlay::Log();

				assert(duk_get_top(ctx) == idxBase);
			}
			// underlay/Owner
			{
				duk_push_object(ctx);
				// stack -> .... base .. ; [Owner] ;

				underlay::Owner* thisOwner = (underlay::Owner*) duk_alloc(ctx, sizeof(underlay::Owner));
				duk_push_pointer(ctx, thisOwner);
				// stack -> .... base .. ; [Owner] ; *Owner ;

				duk_put_prop_string(ctx, idxBase, "\xFF" "*Owner");
				// stack -> .... base .. ; [Owner] ;

				scad40::push_selfie<underlay::Owner>(ctx, thisOwner, 0, [](duk_context* ctx, underlay::Owner* thisOwner) -> duk_ret_t {

					assert(ctx == thisOwner->Host());

					thisOwner->~Owner();
					duk_free(ctx, thisOwner);
					// scad40::env::remove(ctx, "underlay.Owner");
					// TODO ; find a way to actually DO THIS!?!?

					return 0;
				});
				// stack -> .... base .. ; [Owner] ; ~Owner() ;

				duk_set_finalizer(ctx, idxBase);
				// stack -> .... base .. ; [Owner] ;

				// def ofPawn(pawn: Pawn): Soul
					scad40::push_selfie<underlay::Owner>(ctx, thisOwner, 1, [](duk_context* ctx, underlay::Owner* thisOwner) -> duk_ret_t {
						auto result = thisOwner->ofPawn(
							scad40::duk_native<underlay::Pawn>(ctx, 0)
						);
						result.Push();
						return 1;
					});
					duk_put_prop_string(ctx, idxBase, "ofPawn");

				assert(duk_get_top(ctx) == 1 + idxBase);

				// stack -> .... base .. ; [Owner] ;

				scad40::env::assign(ctx, "underlay.Owner");
				// stack -> .... base .. ;

				*reinterpret_cast<duk_context**>(thisOwner) = ctx;
				new (thisOwner) underlay::Owner();

				assert(duk_get_top(ctx) == idxBase);
			}
		}
		assert(duk_get_top(ctx) == idxBase);
	}
}

// =====================================================================================================================
// boilerplate usercode implementations - these things wrap/cast/adapt stuff for your "real" methods
// ---------------------------------------------------------------------------------------------------------------------

#pragma region "global Log"
inline underlay::Log& underlay::Log::get(duk_context* ctx)
{
	auto base = duk_get_top(ctx);

	// stack -> .... base .. ;

	scad40::env::lookup(ctx, "underlay.Log");
	// stack -> .... base .. ; [Log] ;

	duk_get_prop_string(ctx, base, "\xFF" "*Log");
	// stack -> .... base .. ; [Log] ; Log[*] ;

	auto ptrLog = reinterpret_cast<underlay::Log*>(duk_to_pointer(ctx, -1));
	duk_pop_2(ctx);
	// stack -> .... base .. ;

	return *ptrLog;
}
#pragma endregion

#pragma region "native Pawn"
inline bool underlay::Pawn::Is(duk_context* ctx, duk_idx_t idx)
{
	// stack -> ... ; ?[T]? ;
	if (!duk_is_object(ctx, idx))
	{
		return false;
	}
	duk_get_prop_string(ctx, idx, "\xFF" "typeid().name()");
	// stack -> ... ; ?[T]? ; ?"Pawn"? ;
	const char* that = duk_to_string(ctx, -1);
	static const char* name = typeid(underlay::Pawn).name();
	const bool matches = strcmp(name, that) ? false : true;
	duk_pop(ctx);
	// stack -> ... ; ?[T]? ;
	return matches;
}

inline scad40::duk_native<underlay::Pawn> underlay::Pawn::New(duk_context* ctx)
{
	Pawn* thisPawn = (Pawn*)duk_alloc(ctx, sizeof(Pawn));
	const auto idxBase = duk_get_top(ctx);
	// stack -> ... ;
	duk_push_object(ctx);
	// stack -> ... ; [Pawn] ;
	scad40::push_selfie< underlay::Pawn >(ctx, thisPawn, 0, [](duk_context* ctx, underlay::Pawn* thisPawn) -> duk_ret_t {
		thisPawn->~Pawn();
		duk_free(ctx, thisPawn);
		return 0;
	});
	// stack -> ... ; [Pawn] ; ~Pawn() ;
	duk_set_finalizer(ctx, idxBase);
	// stack -> ... ; [Pawn] ;
	duk_push_string(ctx, typeid(underlay::Pawn).name());
	// stack -> ... ; [Pawn] ; typeid(underlay::Pawn).name() ;
	duk_put_prop_string(ctx, idxBase, "\xFF" "typeid().name()");
	// stack -> ... ; [Pawn] ;
	assert(underlay::Pawn::Is(ctx, -1));
	duk_push_pointer(ctx, thisPawn);
	// stack -> ... ; [Pawn] ; *Pawn ;
	duk_put_prop_string(ctx, idxBase, "\xFF" "*");
	// stack -> ... ; [Pawn] ;
	{
		// def detach(): void
			scad40::push_selfie< underlay::Pawn >(ctx, thisPawn, 0, [](duk_context* ctx, underlay::Pawn* thisPawn) -> duk_ret_t {
				thisPawn->detach();
				return 0;
			});
			duk_put_prop_string(ctx, idxBase, "detach");

		// def notify(message: string): void
			scad40::push_selfie< underlay::Pawn >(ctx, thisPawn, 0, [](duk_context* ctx, underlay::Pawn* thisPawn) -> duk_ret_t {
				thisPawn->notify(scad40::duk_string(ctx, 0));
				return 0;
			});
			duk_put_prop_string(ctx, idxBase, "notify");

	}
	// stack -> ... ; [Pawn] ;
	*reinterpret_cast<duk_context**>(thisPawn) = ctx;
	auto t = duk_get_top(ctx);
	new (thisPawn)Pawn();
	assert(t == duk_get_top(ctx));
	auto ret = scad40::duk_native<Pawn>(ctx, -1);
	assert((t) == duk_get_top(ctx));
	// stack -> ... ; [Pawn] ;
	duk_pop(ctx);
	assert(ret.operator->() == thisPawn);
	assert(!ret.IsNull());
	assert(nullptr != ret.operator ->());
	return ret;
}

inline scad40::duk_native<underlay::Pawn> underlay::Pawn::To(duk_context* ctx, duk_idx_t idx)
{
	if (!underlay::Pawn::Is(ctx, idx))
	{
		duk_error(ctx, 314, "Tried to grab `%s` as a {Pawn} (which it is not)", duk_to_string(ctx, idx));
	}
	return scad40::duk_native< underlay::Pawn >(ctx, idx);
}
#pragma endregion

#pragma region "native Soul"
inline bool underlay::Soul::Is(duk_context* ctx, duk_idx_t idx)
{
	// stack -> ... ; ?[T]? ;
	if (!duk_is_object(ctx, idx))
	{
		return false;
	}
	duk_get_prop_string(ctx, idx, "\xFF" "typeid().name()");
	// stack -> ... ; ?[T]? ; ?"Soul"? ;
	const char* that = duk_to_string(ctx, -1);
	static const char* name = typeid(underlay::Soul).name();
	const bool matches = strcmp(name, that) ? false : true;
	duk_pop(ctx);
	// stack -> ... ; ?[T]? ;
	return matches;
}

inline scad40::duk_native<underlay::Soul> underlay::Soul::New(duk_context* ctx)
{
	Soul* thisSoul = (Soul*)duk_alloc(ctx, sizeof(Soul));
	const auto idxBase = duk_get_top(ctx);
	// stack -> ... ;
	duk_push_object(ctx);
	// stack -> ... ; [Soul] ;
	scad40::push_selfie< underlay::Soul >(ctx, thisSoul, 0, [](duk_context* ctx, underlay::Soul* thisSoul) -> duk_ret_t {
		thisSoul->~Soul();
		duk_free(ctx, thisSoul);
		return 0;
	});
	// stack -> ... ; [Soul] ; ~Soul() ;
	duk_set_finalizer(ctx, idxBase);
	// stack -> ... ; [Soul] ;
	duk_push_string(ctx, typeid(underlay::Soul).name());
	// stack -> ... ; [Soul] ; typeid(underlay::Soul).name() ;
	duk_put_prop_string(ctx, idxBase, "\xFF" "typeid().name()");
	// stack -> ... ; [Soul] ;
	assert(underlay::Soul::Is(ctx, -1));
	duk_push_pointer(ctx, thisSoul);
	// stack -> ... ; [Soul] ; *Soul ;
	duk_put_prop_string(ctx, idxBase, "\xFF" "*");
	// stack -> ... ; [Soul] ;
	{
		// def remove(): void
			scad40::push_selfie< underlay::Soul >(ctx, thisSoul, 0, [](duk_context* ctx, underlay::Soul* thisSoul) -> duk_ret_t {
				thisSoul->remove();
				return 0;
			});
			duk_put_prop_string(ctx, idxBase, "remove");

	}
	// stack -> ... ; [Soul] ;
	*reinterpret_cast<duk_context**>(thisSoul) = ctx;
	auto t = duk_get_top(ctx);
	new (thisSoul)Soul();
	assert(t == duk_get_top(ctx));
	auto ret = scad40::duk_native<Soul>(ctx, -1);
	assert((t) == duk_get_top(ctx));
	// stack -> ... ; [Soul] ;
	duk_pop(ctx);
	assert(ret.operator->() == thisSoul);
	assert(!ret.IsNull());
	assert(nullptr != ret.operator ->());
	return ret;
}

inline scad40::duk_native<underlay::Soul> underlay::Soul::To(duk_context* ctx, duk_idx_t idx)
{
	if (!underlay::Soul::Is(ctx, idx))
	{
		duk_error(ctx, 314, "Tried to grab `%s` as a {Soul} (which it is not)", duk_to_string(ctx, idx));
	}
	return scad40::duk_native< underlay::Soul >(ctx, idx);
}
#pragma endregion

#pragma region "global Owner"
inline underlay::Owner& underlay::Owner::get(duk_context* ctx)
{
	auto base = duk_get_top(ctx);

	// stack -> .... base .. ;

	scad40::env::lookup(ctx, "underlay.Owner");
	// stack -> .... base .. ; [Owner] ;

	duk_get_prop_string(ctx, base, "\xFF" "*Owner");
	// stack -> .... base .. ; [Owner] ; Owner[*] ;

	auto ptrOwner = reinterpret_cast<underlay::Owner*>(duk_to_pointer(ctx, -1));
	duk_pop_2(ctx);
	// stack -> .... base .. ;

	return *ptrOwner;
}
#pragma endregion

