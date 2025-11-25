// From Redis src/sds.h:113-118 (sdssetlen function)
// https://github.com/redis/redis/blob/c8e1e833cdab/src/sds.h#L113-L118
// Null-safety warning: dereferencing nullable pointer at line 118

#define NULL ((void*)0)
#define SDS_TYPE_5 0
#define SDS_TYPE_BITS 3

typedef char *sds;

// Actual code from Redis that triggers null-safety warning
static inline void sdssetlen(sds s, unsigned long newlen) {
    unsigned char *fp = ((unsigned char*)s)-1;  // warning: s might be NULL!
    *fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);  // Dereferences fp
}

void example(void) {
    sds str = NULL;
    sdssetlen(str, 5);  // Crash! Pointer arithmetic on NULL
}