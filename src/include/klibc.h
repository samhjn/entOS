#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)

typedef __builtin_va_list va_list;

void xprintf (const char* fmt, ...);
void xsprintf (char* buff, const char* fmt, ...);

#define kprintf xprintf
#define sprintf xsprintf

void*  	memchr (const void*, int, size_t) ;
int  memcmp (const void*, const void*, size_t) ;
//void*  memcpy (void*, const void*, size_t);
//void* memset (void*, int, size_t);
inline void * memset(void * s, char c,size_t count);

#define memset(a,b,c) __builtin_memset(a,b,c);
#define memcpy(a,b,c) __builtin_memcpy(a,b,c);

