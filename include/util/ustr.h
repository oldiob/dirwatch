#ifndef UTIL_USTR_H
#define UTIL_USTR_H


typedef const char ustr;

struct ustr {
	struct ustr *next;
	u64 hash;
	char str[];
};


extern ustr *ustr_mk(const char *str);
extern ustr *ustr_mkn(const char *str, usize len);

extern usize ustr_clear();

static inline u64 ustr_h(const ustr *str)
{
	return (container_of(str, struct ustr, str))->hash;
}


#endif
