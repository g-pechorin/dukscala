
#ifndef PALBL__O
#define PALBL__O	'['
#endif

#ifndef PALBL__C
#define PALBL__C	']'
#endif


#ifndef PALBL_H
#define PALBL_H

#include <assert.h>
#include <stdint.h>

#ifdef __cplusplus
#include <string>
#include <vector>
#endif


typedef struct palbl_string_s{

	size_t _length;
	const char* _content;

#ifdef __cplusplus
	operator std::string(void)const { return std::string(_content, _length); }
	palbl_string_s(const char* source) : _length(strlen(source)), _content(source) {}
	palbl_string_s(void) : _length(0xCAFACAF), _content(nullptr){};


	palbl_string_s(size_t length, const char* source) : _length(length), _content(source) {}
#endif
} palbl_string_t;



void* palbl_load(
	const palbl_string_t document,
	const size_t offset,
	void* global,
	void* parent,
	void* (*callback_begin)(void* global, void* parent, const palbl_string_t tag),
	void* (*callback_assign)(void* global, void* parent, const palbl_string_t tag, void* self, const palbl_string_t key, const palbl_string_t val),
	void* (*callback_close)(void* global, void* parent, const palbl_string_t tag, void* self));


#endif

#ifdef PALBL_C

#define palbl_load__tagcmp(l, r)					((l)._length == (r)._length && 0 == strncmp((l)._content, (r)._content, (l)._length))
#define palbl_load__isws(c)							(' ' == (c) || '\t' == (c) || '\n' == (c) || '\r' == (c))
#define palbl_load__is_alphanum(c)					(('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z') || ('0' <= (c) && (c) <= '9'))
#define palbl_load__is_equal(document, offset, c)	((document)._content[(offset)] == (c))

void* palbl_load(
	const palbl_string_t document,
	const size_t offset,
	void* global,
	void* parent,
	void* (*callback_begin)(void* global, void* parent, const palbl_string_t tag),
	void* (*callback_assign)(void* global, void* parent, const palbl_string_t tag, void* self, const palbl_string_t key, const palbl_string_t val),
	void* (*callback_close)(void* global, void* parent, const palbl_string_t tag, void* self))
{
	size_t i, e;
	void* self;
	palbl_string_t tag, end;

	// TODO ; fail here
	if (offset >= document._length)
		return nullptr;

	// find the start of the next tag
	if (palbl_load__isws(document._content[offset]))
		return palbl_load(document, 1 + offset, global, parent, callback_begin, callback_assign, callback_close);

	// read the start tag
	assert(PALBL__O == document._content[offset]);
	for (i = 1; i < document._length && PALBL__C != document._content[offset + i] && !palbl_load__isws(document._content[offset + i]); ++i)
		assert(palbl_load__is_alphanum(document._content[offset + i]));
	tag._length = i - offset - 1;
	tag._content = document._content + 1;

	// start the object
	self = callback_begin(global, parent, tag);

	// move until the "next" thing
	while (palbl_load__isws(document._content[offset + i])) ++i;

	if (!palbl_load__is_equal(document, offset + i, PALBL__O) && palbl_load__is_equal(document, offset + i + 1, '/'))
	{
		assert(false && "I don't know");
	}
	else
	{
		e = i + 2;
		auto c = palbl_load__is_alphanum(document._content[offset + e]);
		assert(palbl_load__is_alphanum(document._content[offset + e]));
		while (palbl_load__is_alphanum(document._content[offset + e])) ++e;

		end._length = e - i - 2;
		end._content = document._content + i + 2;

		if (!palbl_load__tagcmp(tag, end))
			assert(false && "I don't know");
		else
		{
			// close the object
			return callback_close(global, parent, end, self);
		}
	}

	return nullptr;
}
#endif