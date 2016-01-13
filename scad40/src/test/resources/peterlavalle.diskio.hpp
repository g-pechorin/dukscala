
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

#include <assert.h>
#include <stdint.h>

#define scad40__pre_string ("\xFF" "scad40")
#define scad40__pre_strlen (7)

namespace scad40
{
	/// goofy wrapper to push member-like functions (so methods and accessors)
	/// ... should I call it "push_method?" or "push_member?"
	template<typename T>
	inline void push_selfie(duk_context* ctx, T* self, duk_idx_t nargs, duk_ret_t(*code)(duk_context*,T*));

	bool has_prop(duk_context* ctx, const duk_idx_t idx, const char* key);

	class object
	{
		/// magical pointer to the object's hosting context
		/// ... it will/should be set before construction
		duk_context* _ctx;

		friend class duk_obj;
		
		template<typename T>
		static const char* type_string(void);

	protected:
		object(duk_context*);
		object(void);

		// TODO ; move/merge copy operator and copy constructor from duk_str and duk_ref

		// TODO ; move/merge the destructors from duk_str and duk_ref
	public:
		duk_context* Host(void) const;

		std::array<char, scad40__pre_strlen + (sizeof(void*) * 2) + 1> KeyString(void) const;

		/// pushes (a reference to) the value onto the stack (or maybe null if the value is null)
		inline void Push(void) const
		{
			duk_push_global_stash(Host());
			duk_get_prop_string(Host(), -1, KeyString().data());
			duk_remove(Host(), -2);
		}

		// TODO ; move/merge the is_null function from duk_obj - and change it to IsNull()
	};

	class duk_obj : public scad40::object
	{
	protected:
		duk_obj(duk_context*);
	public:
		~duk_obj(void);

		bool is_null(void) const;
	};

	/// less sophiticated that the ref
	template<typename T>
	class duk_ptr : public scad40::object
	{
	public:
		duk_ptr(duk_context* ctx, duk_idx_t idx) : scad40::object(ctx)
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


	///
	/// holds a ref to a duktape object using magic
	template<typename T>
	class duk_ref : public scad40::duk_obj
	{
		T* _ptr;
	public:

		/// grab an instance from the stack. fails violently if types are wrong
		duk_ref(duk_context*, const duk_idx_t);

		duk_ref(const duk_ref<T>&);
		duk_ref<T>& operator= (const duk_ref<T>&);

		T* operator -> (void);
		const T* operator -> (void) const;

		~duk_ref(void);
	};

	///
	/// holds a ref to a duktape string using magic
	class duk_str : public scad40::duk_obj
	{
		const char* _str;
	public:
		/// create an instance and set it to point to the passed string
		duk_str(duk_context* ctx, const char* = nullptr);

		/// create an instance and set it to point to the passed string
		duk_str(duk_context* ctx, const std::string&);

		/// grab an instance from the stack. fails violently if types are wrong
		duk_str(duk_context* ctx, const duk_idx_t);

		duk_str(const duk_str&);
		duk_str& operator= (const duk_str&);

		duk_str& operator = (const char*);
		duk_str& operator = (const std::string&);

		duk_str& operator == (const char*) const;
		duk_str& operator == (const std::string&) const;
		duk_str& operator == (const duk_str&) const;

		operator const char* (void) const;

		~duk_str(void);
	};

	namespace env
	{
		/// puts whatever is on top of the stack somewhere into the global namespace
		/// happily splits up names with '.' in them
		/// creates whatever tables it needs to along the way
		/// throws an error if it finds a pre-existing value
		inline void assign(duk_context* ctx, const char* key);

		/// reads something from the global namespace and pushes it on the stack
		/// happily splits up names with '.' in them
		/// happily pushes undefined if there's no value there
		inline void lookup(duk_context* ctx, const char* binding);

		/// checks the global namespace for a non null_or_undefined value at path
		inline bool exists(duk_context* ctx, const char* path)
		{
			// stack -> .... base .. ;

			scad40::env::lookup(ctx, path);
			// stack -> .... base .. ; ??? ;

			const auto result = duk_is_null_or_undefined(ctx, -1) ? false : true;

			duk_pop(ctx);
			// stack -> .... base .. ;

			return result;
		}

		/// drops the value at key from the global namespace
		/// leaves containers et-al in place
		/// throws an error iff nothing exists there
		bool remove(duk_context* ctx, const char* path);
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
	struct Reading : public scad40::object
	{
		/// the Reading constructor
		/// ... the user must implement this
		Reading(void);

		Reading(const Reading&);
		Reading& operator = (const Reading&);

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

		/// pushes this object onto its hosting stack
		void Push(void)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		/// queries if the passed index is a Reading object
		static bool Is(duk_context* ctx, duk_idx_t idx);

		/// creates a new Reading object and returns a magical pointer to it
		static scad40::duk_ref<Reading> New(duk_context* ctx);

		/// pulls the the passed index into a Reading object
		/// ... if the passed index is not a Reading object - behaviour is undefined
		static scad40::duk_ref<Reading> To(duk_context* ctx, duk_idx_t idx);
	};

	/// a global class
	struct Disk : public scad40::object
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

		/// magic method to load/store dynamic fields
		template<typename T>
		T& Stash(const pal_adler32::obj&);

		/// overload. assumes typeid(T).name() is acceptable
		template<typename T> T& Stash(void) { const static pal_adler32::obj hash = typeid(T).name(); return Stash<T>(hash); }

		/// locates the instance
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

		// >> load up code to wrap scripts
		{
			// assert(false && "?? load up code to make script super-classes");
			// assert(false && "... is there anything to do here? is this redundant?");
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

					thisDisk->~Disk();
					duk_free(ctx, thisDisk);

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

#pragma region "scad40"
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
#pragma endregion

#pragma region "duk_str"
inline scad40::duk_str::duk_str(duk_context* ctx, const char* str) :
	scad40::duk_obj(ctx)
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
	scad40::duk_obj(ctx)
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

inline scad40::duk_str::duk_str(const scad40::duk_str& other) :
	scad40::duk_obj(other.Host())
{
	duk_push_global_stash(Host());
	duk_get_prop_string(Host(), -1, other.KeyString().data());
	duk_put_prop_string(Host(), -2, KeyString().data());
	duk_pop(Host());
	_str = other._str;
}

inline scad40::duk_str& scad40::duk_str::operator=(const scad40::duk_str& other)
{
	assert(Host() == other.Host());

	duk_push_global_stash(Host());
	duk_get_prop_string(Host(), -1, other.KeyString().data());
	duk_put_prop_string(Host(), -2, KeyString().data());
	duk_pop(Host());

	_str = other._str;

	return *this;
}

inline scad40::duk_str::operator const char* (void) const
{
	assert(nullptr != Host());

	// stack -> ... ;

	duk_push_global_stash(Host());
	// stack -> ... ; [global stash] ;

	duk_get_prop_string(Host(), -1, KeyString().data());
	// stack -> ... ; [global stash] ; "val" ;

	auto result = duk_to_string(Host(), -1);

	duk_pop_2(Host());
	// stack -> ... ;

	return result;
}

inline scad40::duk_str::~duk_str(void)
{
	if (Host())
	{
		duk_push_global_stash(Host());
		duk_del_prop_string(Host(), -1, KeyString().data());
		duk_pop(Host());

		_str = reinterpret_cast<char*>(0xCAFEBABA);
	}
}
#pragma endregion

#pragma region "duk_obj"
inline scad40::duk_obj::duk_obj(duk_context* ctx) :
	scad40::object(ctx)
{
	assert(is_null());
}

inline bool scad40::duk_obj::is_null(void) const
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

inline scad40::duk_obj::~duk_obj(void)
{
	assert(nullptr != Host());

	// stack -> ... ;

	duk_push_global_stash(Host());
	// stack -> ... ; [global stash] ;

	duk_del_prop_string(Host(), -1, KeyString().data());

	duk_pop(Host());
	// stack -> ... ;
}
#pragma endregion

#pragma region "duk_ref"
template<typename T>
inline scad40::duk_ref<T>::duk_ref(duk_context* ctx, const duk_idx_t idx) :
	scad40::duk_obj(ctx)
{
	assert(T::Is(ctx, idx));
	// stack -> ... ; [T] ; ... ;

	duk_get_prop_string(ctx, idx, "\xFF" "*");
	// stack -> ... ; [T] ; ... ; T* ;

	_ptr = (T*)duk_to_pointer(ctx, -1);

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

template<typename T>
inline scad40::duk_ref<T>::duk_ref(const scad40::duk_ref<T>& other) :
	scad40::duk_obj(other.Host())
{
	// stack -> ... ;

	duk_push_global_stash(Host());
	// stack -> ... ; [global stash] ;

	duk_get_prop_string(Host(), -1, other.KeyString().data());
	// stack -> ... ; [global stash] ; other ;

	duk_put_prop_string(Host(), -2, KeyString().data());
	// stack -> ... ; [global stash] ;

	duk_pop(Host());
	// stack -> ... ;

	_ptr = duk_is_null_or_undefined(Host(), -1) ? nullptr : other._ptr;
}

template<typename T>
inline T* scad40::duk_ref<T>::operator -> (void)
{
	return _ptr;
}

template<typename T>
inline const T* scad40::duk_ref<T>::operator -> (void) const
{
	return _ptr;
}

template<typename T>
inline scad40::duk_ref<T>::~duk_ref(void)
{
	_ptr = reinterpret_cast<T*>(0xCACADAD);
}
#pragma endregion

#pragma region "object"
inline scad40::object::object(void)
{
	assert(nullptr != _ctx);
}

inline scad40::object::object(duk_context* ctx) :
	_ctx(ctx)
{
	assert(nullptr != _ctx);
}

inline duk_context* scad40::object::Host(void) const
{
	return _ctx;
}

inline std::array<char, scad40__pre_strlen + (sizeof(void*) * 2) + 1> scad40::object::KeyString(void) const
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

template<typename T>
inline const char* scad40::object::type_string(void)
{
	/// there is an insanely small chance of a leak
	static char* _ptr = nullptr;
	if (nullptr != _ptr)
	{
		return _ptr;
	}

	const char* str = typeid(T).name();
	size_t len = strlen(str);
	char* ptr = (const char*)malloc(len + 1);

	size_t src = 0, out = 0;

	while (str[src])
	{
		const auto c = str[src++];
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || ('_' == c))
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
#endif // ... okay - that's the end of predef


// =====================================================================================================================
// boilerplate usercode implementations - these things wrap/cast/adapt stuff for your "real" methods
// ---------------------------------------------------------------------------------------------------------------------
#pragma region "script ChangeListener"
inline void peterlavalle::diskio::ChangeListener::fileChanged(const scad40::duk_str& path)
{
	auto ptr = reinterpret_cast<scad40::duk_ptr<ChangeListener>*>(this);

	assert(Host() == ptr->Host() && "SAN check failed");

	const auto base = duk_get_top(Host());

	// stack -> .. base .. ;

	ptr->Push();
	// stack -> .. base .. ; [self] ;

	duk_get_prop_string(Host(), -1, "fileChanged");
	// stack -> .. base .. ; [self] ; fileChanged() ;

	duk_insert(Host(), -2);
	// stack -> .. base .. ; fileChanged() ; [self] ;

	assert(Host() == path.Host());
	path.Push();
	// stack -> .. base .. ; fileChanged() ; [self] ; "path" ;

	duk_call_method(Host(), 1);
	// stack -> .. base .. ; ?result? ;

	duk_pop(Host());
	return;
}

inline scad40::duk_ptr<peterlavalle::diskio::ChangeListener> peterlavalle::diskio::ChangeListener::New(duk_context* ctx, const char* subclass)
{
	assert(false && "??? scad40 needs to provide this");
	duk_error(ctx, 314, "??? scad40 needs to provide this");
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

	duk_push_string(ctx, typeid( peterlavalle::diskio::Reading ).name());
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
			duk_put_prop_string( ctx, -2, "read" );

		// def close(): void
			scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

				assert(false && "??? scad40 needs to provide this");
				return -1;

			});
			duk_put_prop_string( ctx, -2, "close" );

		// def endOfFile(): bool
			scad40::push_selfie< peterlavalle::diskio::Reading >(ctx, thisReading, 0, [](duk_context* ctx, peterlavalle::diskio::Reading* thisReading) -> duk_ret_t {

				assert(false && "??? scad40 needs to provide this");
				return -1;

			});
			duk_put_prop_string( ctx, -2, "endOfFile" );

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

	assert(!ret.is_null());
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