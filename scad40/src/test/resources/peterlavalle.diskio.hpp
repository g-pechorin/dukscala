
#pragma once

#include <duktape.h>

#include <array>
#include <string>

#include <assert.h>
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

		friend class duk_object;

		template<typename T>
		static const char* type_string(void);

		object(duk_context*);
	protected:
		object(void);
	public:
		duk_context* Host(void) const;

		std::array<char, scad40_duk_ref_et_str__strlen + (sizeof(void*) * 2) + 1> KeyString(void) const;
	};

	class duk_object : public scad40::object
	{
		template<typename T>
		friend class duk_ref;

		friend struct duk_str;

		duk_object(duk_context*);
	public:
		~duk_object(void);

		bool is_null(void) const;
	};

	///
	/// holds a ref to a duktape object using magic
	template<typename T>
	class duk_ref : public scad40::duk_object
	{
		T* _ptr;
	public:
		duk_ref(duk_context*);

		duk_ref(const duk_ref<T>&);
		duk_ref<T>& operator= (const duk_ref<T>&);

		T* operator -> (void) { return _ptr; }
		const T* operator -> (void) const { return _ptr; }

		~duk_ref(void);
	};

	///
	/// holds a ref to a duktape string using magic
	struct duk_str : public scad40::duk_object
	{
		duk_str(duk_context* ctx);
		duk_str(duk_context* ctx, duk_idx_t);
		duk_str(duk_context* ctx, const char*);
		duk_str(duk_context* ctx, const std::string&);

		duk_str(const duk_str&);
		duk_str& operator = (const duk_str&);

		duk_str& operator = (const char*);
		duk_str& operator = (const std::string&);

		operator const char* (void) const;
	};

	template<typename T>
	duk_ref<T> create(duk_context* ctx);

	inline void assign(duk_context* ctx, const char* key)
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

	inline void lookup(duk_context* ctx, const char* binding)
	{
		size_t idx = 0, len = 0;

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

				duk_get_prop(ctx, -2);
				// stack -> .... base .. ; [outer host] ; ?[inner host]? ;

				if (duk_is_null_or_undefined(ctx, -1))
				{
					duk_pop_2(ctx);
					// stack -> .... base .. ;

					duk_push_undefined(ctx);
					// stack -> .... base .. ; <undefined> ;

					return;
				}

				// stack -> .... base .. ; [host] ;

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
};
#endif // ... okay - that's the end of predef

namespace peterlavalle {
namespace diskio {

	/// a script class
	struct ChangeListener : public scad40::duk_object
	{
		//
		//misc head stuff
		//

		/// the user's requested members
			void fileChanged (const scad40::duk_str& path);

		/// push this onto its stack
		void Push(void)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		/// create an instance of a scripted class that extends this class
		static scad40::duk_ref<ChangeListener> New(duk_context* ctx, const char* subclass)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		/// is whatever an instance of this class
		static bool Is(duk_context* ctx, duk_idx_t idx)
		{
			assert(false && "??? scad40 needs to provide this");
		}

		static scad40::duk_ref<ChangeListener> To(duk_context* ctx, duk_idx_t idx)
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
			scad40::duk_str _path;

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
			void foobar(const scad40::duk_str& text);
			scad40::duk_ref<Reading> open(const scad40::duk_str& path);
			scad40::duk_str _pwd;
			void subscribe(const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener);
			void unsubscribe(const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener);

		/// alternative const char* interfaces
			void foobar(const char* text)
			{
				scad40::duk_str text_(Host(), text);

				foobar(text_);
			}

		~Disk(void);

		/// locates the instance
		static Disk& get(duk_context*);
	};

	/// sets up the tables and calls to this VM
	inline void install(duk_context* ctx)
	{
		auto base = duk_get_top(ctx);

		// >> check for name collisions
		{
			// stack -> .... base .. ;

			scad40::lookup(ctx, "peterlavalle.diskio");
			// stack -> .... base .. ; ?? previous ?? ;

			if (!duk_is_undefined(ctx, -1))
			{
				duk_error(ctx, 314, "Can't redefine module `peterlavalle.diskio`");
				return;
			}
			// stack -> .... base .. ; <undefined> ;

			duk_pop(ctx);
			// stack -> .... base .. ;

			assert(duk_get_top(ctx) == base);
		};

		// >> bind lambdas for native class construction
		{
			// stack -> .... base .. ;

			duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

				duk_error(ctx, 314, "???");
				return -1;

			}, 0);
			// stack -> .... base .. ; class:Reading() ;

			scad40::assign(ctx, "peterlavalle.diskio.Reading");
			// stack -> .... base .. ;

			assert(duk_get_top(ctx) == base);
		}

		// >> load up code to wrap scripts
		{
			// assert(false && "?? load up code to make script super-classes");
		}

		// >> allocate / in-place-new and store ALL global objects (including context pointers)
		{
			// stack -> .... base .. ;

			// peterlavalle/diskio/Disk
			{
				duk_push_object(ctx);
				// stack -> .... base .. ; [Disk] ;

				Disk* ptrDisk = (Disk*) duk_alloc(ctx, sizeof(Disk));
				duk_push_pointer(ctx, ptrDisk);
				// stack -> .... base .. ; [Disk] ; *Disk ;

				duk_put_prop_string(ctx, -2, "\xFF" "*Disk");
				// stack -> .... base .. ; [Disk] ;

				duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

					// stack -> ... ;

					duk_push_current_function(ctx);
					// stack -> ... ; ~Disk() ;

					duk_get_prop_string(ctx, -1, "\xFF" "*Disk");
					// stack -> ... ; ~Disk() ; *Disk ;

					Disk* thisDisk = (Disk*)duk_to_pointer(ctx, -1);

					thisDisk->~Disk();
					duk_free(ctx, thisDisk);

					return 0;

				}, 0);
				// stack -> .... base .. ; [Disk] ; ~Disk() ;

				duk_push_pointer(ctx, ptrDisk);
				// stack -> .... base .. ; [Disk] ; ~Disk() ; *Disk ;

				duk_put_prop_string(ctx, -2, "\xFF" "*Disk");
				// stack -> .... base .. ; [Disk] ; ~Disk() ;

				duk_set_finalizer(ctx, -2);
				// stack -> .... base .. ; [Disk] ;


				// def foobar(text: string): void
					duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

						// get this*
						duk_push_current_function(ctx);
						duk_get_prop_string(ctx, -1, "\xFF" "*Disk");
						Disk* thisDisk = (Disk*) duk_to_pointer(ctx, -1);
						duk_pop_2(ctx);

						// text: string
						scad40::duk_str text(ctx, 0);

						// make the call
						thisDisk->foobar(text);

						return 0;

					}, 1);
					duk_push_pointer(ctx, ptrDisk);
					duk_put_prop_string(ctx, -2, "\xFF" "*Disk");
					duk_put_prop_string(ctx, -2, "foobar");


				// def open(path: string): Reading
					duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

						assert(false && "???");
						return -1;

					}, 1);
					duk_put_prop_string(ctx, -2, "open");

				// var pwd: string
					duk_push_string(ctx, "pwd");
					duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

						// getter(key, ???, ???)
						assert(false && "???");
						return -1;

					}, 3);
					duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

						// setter(new val, key, ???)
						assert(false && "???");
						return -1;

					}, 3);
					duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_ENUMERABLE);

				// def subscribe(path: string, listener: ChangeListener)
					duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

						assert(false && "???");
						return -1;

					}, 2);
					duk_put_prop_string(ctx, -2, "subscribe");

				// def unsubscribe(path: string, listener: ChangeListener)
					duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

						assert(false && "???");
						return -1;

					}, 2);
					duk_put_prop_string(ctx, -2, "unsubscribe");

				assert(duk_get_top(ctx) == 1 + base);

				// stack -> .... base .. ; [Disk] ;

				scad40::assign((*reinterpret_cast<duk_context**>(ptrDisk) = ctx), "peterlavalle.diskio.Disk");
				// stack -> .... base .. ;

				new (ptrDisk)Disk();

				assert(duk_get_top(ctx) == base);
			}
		}
	}
}
}

#pragma region "Implementations"

inline scad40::duk_str::duk_str(duk_context* ctx) :
	scad40::duk_str(ctx, nullptr)
{
}

inline scad40::duk_object::~duk_object(void)
{
	assert(nullptr != Host());

	// stack -> ... ;

	duk_push_global_stash(Host());
	// stack -> ... ; [global stash] ;

	duk_del_prop_string(Host(), -1, KeyString().data());

	duk_pop(Host());
	// stack -> ... ;
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

inline scad40::duk_str::duk_str(duk_context* ctx, duk_idx_t idx) :
	scad40::duk_object(ctx)
{
	assert(duk_is_string(ctx, idx));

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

inline scad40::duk_str::duk_str(duk_context* ctx, const char* str) :
	scad40::duk_object(ctx)
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

inline scad40::duk_object::duk_object(duk_context* ctx) :
	scad40::object(ctx)
{
}

inline scad40::object::object(duk_context* ctx) :
	_ctx(ctx)
{
	assert(nullptr != _ctx);
}

inline scad40::object::object(void)
{
	assert(nullptr != _ctx);
}

inline duk_context* scad40::object::Host(void) const
{
	return _ctx;
}

inline std::array<char, scad40_duk_ref_et_str__strlen + (sizeof(void*) * 2) + 1> scad40::object::KeyString(void) const
{
	assert(strlen(scad40_duk_ref_et_str__string) == scad40_duk_ref_et_str__strlen);
	std::array<char, scad40_duk_ref_et_str__strlen + (sizeof(void*) * 2) + 1> result;

	strcpy(result.data(), scad40_duk_ref_et_str__string);
	size_t write = scad40_duk_ref_et_str__strlen;

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


#pragma region "duk_object implmentation"
inline bool scad40::duk_object::is_null(void) const
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
#pragma endregion

#pragma region "duk_ref implementation"
template<typename T>
inline scad40::duk_ref<T>::duk_ref(duk_context* ctx) :
	scad40::duk_object(ctx)
{
	assert(T::Is(ctx, -1));
	// stack -> ... ; [T] ;

	duk_get_prop_string(ctx, -1, "\xFF" "*");
	// stack -> ... ; [T] ; T* ;

	_ptr = (T*)duk_to_pointer(ctx, -1);

	duk_pop(ctx);
	// stack -> ... ; [T] ;

	duk_push_global_stash(ctx);
	// stack -> ... ; [T] ; [global stash] ;

	duk_insert(ctx, -2);
	// stack -> ... ; [global stash] ; [T] ;

	duk_put_prop_string(ctx, -2, KeyString().data());
	// stack -> ... ; [global stash] ;

	duk_pop(ctx);
	// stack -> ... ;
}

template<typename T>
inline scad40::duk_ref<T>::duk_ref(const scad40::duk_ref<T>& other) :
	scad40::duk_object(other.Host())
{
	// stack -> ... ;

	duk_push_global_stash(Host());
	// stack -> ... ; [global stash] ;

	duk_get_prop_string(Host(), -1, other.KeyString().data());
	// stack -> ... ; [global stash] ; other ;

	_ptr = duk_is_null_or_undefined(Host(), -1) ? nullptr : other._ptr;

	duk_put_prop_string(Host(), -2, KeyString().data());
	// stack -> ... ; [global stash] ;

	duk_pop(Host());
	// stack -> ... ;
}


template<typename T>
inline scad40::duk_ref<T>::~duk_ref(void)
{
	_ptr = reinterpret_cast<T*>(0xCACADAD);
}

#pragma endregion

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// usercode implmentation
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

	duk_push_pointer(ctx, thisReading);
	// stack -> ... ; [Reading] ; *Reading ;

	duk_put_prop_string(ctx, -2, thisReading->KeyString().data());
	// stack -> ... ; [Reading] ;

	duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

		// stack -> ... ; [Reading] ;

		duk_push_current_function(ctx);
		// stack -> ... ; [Reading] ; ~Reading() ;

		duk_get_prop_string(ctx, -1, "\xFF" "*");
		// stack -> ... ; [Reading] ; ~Reading() ; Reading* ;

		Reading* thisReading = (Reading*)duk_to_pointer(ctx, -1);

		thisReading->~Reading();

		duk_free(ctx, thisReading);

		return 0;

	}, 0);
	// stack -> ... ; [Reading] ; ~Reading() ;

	duk_push_pointer(ctx, thisReading);
	// stack -> ... ; [Reading] ; ~Reading() ; *Reading ;

	duk_put_prop_string(ctx, -2, "\xFF" "*");
	// stack -> ... ; [Reading] ; ~Reading() ;

	duk_set_finalizer(ctx, -2);
	// stack -> ... ; [Reading] ;

	duk_push_string(ctx, typeid(Reading).name());
	// stack -> ... ; ~Reading() ; "peterlavalle::diskio::Reading" ;

	duk_put_prop_string(ctx, -2, "\xFF" "typeid().name()");
	// stack -> ... ; ~Reading() ;

	duk_push_pointer(ctx, thisReading);
	// stack -> ... ; [Reading] ; *Reading ;

	duk_put_prop_string(ctx, -2, "\xFF" "*");
	// stack -> ... ; [Reading] ;

	{
		// def read(): sint8
		duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

			assert(false && "??? scad40 needs to provide this");
			return -1;

		}, 0);
		duk_push_pointer(ctx, thisReading);
		duk_put_prop_string(ctx, -2, "\xFF" "*Reading");
		duk_put_prop_string(ctx, -2, "read");

		// def close(): sint8
		duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

			assert(false && "??? scad40 needs to provide this");
			return -1;

		}, 0);
		duk_push_pointer(ctx, thisReading);
		duk_put_prop_string(ctx, -2, "\xFF" "*Reading");
		duk_put_prop_string(ctx, -2, "close");

		// def endOfFile(): bool
		duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

			assert(false && "??? scad40 needs to provide this");
			return -1;

		}, 0);
		duk_push_pointer(ctx, thisReading);
		duk_put_prop_string(ctx, -2, "\xFF" "*Reading");
		duk_put_prop_string(ctx, -2, "endOfFile");

		// var number: single
		duk_push_string(ctx, "number");
		duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {
			duk_push_current_function(ctx);
			duk_get_prop_string(ctx, -1, "\xFF" "*Reading");
			Reading* thisReading = (Reading*)duk_to_pointer(ctx, -1);
			duk_push_number(ctx, thisReading->_number);
			return 1;
		}, 3);
		duk_push_pointer(ctx, thisReading);
		duk_put_prop_string(ctx, -2, "\xFF" "*Reading");
		duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {
			duk_push_current_function(ctx);
			duk_get_prop_string(ctx, -1, "\xFF" "*Reading");
			Reading* thisReading = (Reading*)duk_to_pointer(ctx, -1);
			thisReading->_number = (float)duk_to_number(ctx, 0);
			return 0;
		}, 3);
		duk_push_pointer(ctx, thisReading);
		duk_put_prop_string(ctx, -2, "\xFF" "*Reading");
		duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_ENUMERABLE);

		// val path: string
		duk_push_string(ctx, "path");
		duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {
			duk_push_current_function(ctx);
			duk_get_prop_string(ctx, -1, "\xFF" "*Reading");
			Reading* thisReading = (Reading*)duk_to_pointer(ctx, -1);

			assert(false && "???");

			return 1;
		}, 3);
		duk_push_pointer(ctx, thisReading);
		duk_put_prop_string(ctx, -2, "\xFF" "*Reading");
		duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_SETTER);
	}


	// stack -> ... ; [Reading] ;
	*reinterpret_cast<duk_context**>(thisReading) = ctx;
	auto t = duk_get_top(ctx);
	new (thisReading)Reading();
	assert(t == duk_get_top(ctx));
	auto ret = scad40::duk_ref<Reading>(ctx);
	assert((t - 1) == duk_get_top(ctx));
	// stack -> ... ;

	assert(!ret.is_null());
	assert(nullptr != ret.operator ->());

	return ret;
}

inline scad40::duk_ref<peterlavalle::diskio::Reading> peterlavalle::diskio::Reading::To(duk_context* ctx, duk_idx_t idx)
{
	idx = idx < 0 ? duk_normalize_index(ctx, idx) : idx;

	if (!peterlavalle::diskio::Reading::Is(ctx, idx))
	{
		duk_error(ctx, 314, "Tried to grab `%s` as a {Reading} (which it is not)", duk_to_string(ctx, idx));
	}

	assert(false && "??? scad40 needs to provide this");
}
#pragma endregion

#pragma region "global Disk"
inline peterlavalle::diskio::Disk& peterlavalle::diskio::Disk::get(duk_context* ctx)
{
	// stack -> .... base .. ;

	scad40::lookup(ctx, "peterlavalle.diskio.Disk");
	// stack -> .... base .. ; [Disk] ;

	duk_get_prop_string(ctx, -1, "\xFF" "*Disk");
	// stack -> .... base .. ; [Disk] ; Disk[*] ;

	Disk* ptrDisk = reinterpret_cast<Disk*>(duk_to_pointer(ctx, -1));
	duk_pop_2(ctx);
	// stack -> .... base .. ;

	return *ptrDisk;
}
#pragma endregion
