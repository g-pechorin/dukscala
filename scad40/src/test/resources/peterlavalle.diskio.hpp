
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

		template<typename T>
		friend class duk_ref;

		friend struct duk_str;

		template<typename T>
		static const char* type_string(void);

		object(duk_context*);
	protected:
		object(void);
	public:
		duk_context* Host(void);

		std::array<char, scad40_duk_ref_et_str__strlen + (sizeof(void*) * 2) + 1> KeyString(void) const;
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

			if ('/' != key[idx + len])
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
			if ('/' != binding[idx + len])
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
		// stack -> .... base .. ; [host] ; "key" ;

		assert(false && "???");
	}
};
#endif // ... okay - that's the end of predef

namespace peterlavalle {
	namespace diskio {

		/// a script class
		struct ChangeListener : public scad40::object
		{
			//
			//misc head stuff
			//

			/// the user's requested members
			void fileChanged(const scad40::duk_str& path);

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
			int8_t read();
			void close();
			bool endOfFile();
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
			static bool Is(duk_context* ctx, duk_idx_t idx)
			{
				idx = idx < 0 ? duk_normalize_index(ctx, idx) : idx;

				assert(false && "??? scad40 needs to provide this");
			}

			/// pulls the the passed index into a Reading object
			/// ... if the passed index is not a Reading object - behaviour is undefined
			static scad40::duk_ref<Reading> To(duk_context* ctx, duk_idx_t idx)
			{
				idx = idx < 0 ? duk_normalize_index(ctx, idx) : idx;

				if (!Is(ctx, idx))
				{
					duk_error(ctx, 314, "Tried to grab `%s` as a {Reading} (which it is not)", duk_to_string(ctx, idx));
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
			Disk(const Disk&) = delete;
			Disk& operator = (const Disk&) = delete;

			/// the Disk constructor
			/// the user must implement this
			Disk(void);

			/// the user's requested members
			/// the user must implement these
			scad40::duk_ref<Reading> open(const scad40::duk_str& path);
			scad40::duk_str _pwd;
			void subscribe(const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener);
			void unsubscribe(const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener);

			/// locates the instance
			static Disk& get(duk_context*);
		};

		/// sets up the tables and calls to this VM
		void install(duk_context* ctx)
		{
			auto base = duk_get_top(ctx);

			// >> check for name collisions
			{
				// stack -> .... base .. ;

				scad40::lookup(ctx, "peterlavalle/diskio");
				// stack -> .... base .. ; ?? previous ?? ;

				if (!duk_is_undefined(ctx, -1))
				{
					duk_error(ctx, 314, "Can't redefine module `peterlavalle/diskio`");
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

				scad40::assign(ctx, "peterlavalle/diskio/Reading");
				// stack -> .... base .. ;

				assert(duk_get_top(ctx) == base);
			}

			// >> load up code to make script super-classes?
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

					Disk* ptrDisk = reinterpret_cast<Disk*>(duk_push_fixed_buffer(ctx, sizeof(Disk)));
					// stack -> .... base .. ; [Disk] ; Disk[*] ;

					duk_put_prop_string(ctx, -2, ptrDisk->KeyString().data());
					// stack -> .... base .. ; [Disk] ;

					{
						// def open(path: string): Reading
						{
							duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

								assert(false && "???");
								return -1;

							}, 1);
							duk_put_prop_string(ctx, -2, "open");
						}

						// var pwd: string
						{
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
						}

						// def subscribe(path: string, listener: ChangeListener)
						{
							duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

								assert(false && "???");
								return -1;

							}, 2);
							duk_put_prop_string(ctx, -2, "subscribe");
						}

						// def unsubscribe(path: string, listener: ChangeListener)
						{
							duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {

								assert(false && "???");
								return -1;

							}, 2);
							duk_put_prop_string(ctx, -2, "unsubscribe");
						}

						assert(duk_get_top(ctx) == 1 + base);
					}

					// stack -> .... base .. ; [Disk] ;

					scad40::assign((*reinterpret_cast<duk_context**>(ptrDisk) = ctx), "peterlavalle/diskio/Disk");
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

inline scad40::duk_str::duk_str(duk_context* ctx, const char* str) :
scad40::object(ctx)
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

inline scad40::object::object(duk_context* ctx) :
_ctx(ctx)
{
	assert(nullptr != _ctx);
}

inline scad40::object::object(void)
{
	assert(nullptr != _ctx);
}

inline duk_context* scad40::object::Host(void)
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
	char* ptr = (const char*)malloc(len + 1)

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