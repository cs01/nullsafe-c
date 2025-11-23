/*
 * ctype.h - Character classification and conversion (pure function annotations)
 * These functions are marked as const/pure since they don't modify state
 */

#ifndef _CBANG_CTYPE_H
#define _CBANG_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Character classification - all are const (no side effects) */
int isalnum(int c) __attribute__((const));
int isalpha(int c) __attribute__((const));
int isblank(int c) __attribute__((const));
int iscntrl(int c) __attribute__((const));
int isdigit(int c) __attribute__((const));
int isgraph(int c) __attribute__((const));
int islower(int c) __attribute__((const));
int isprint(int c) __attribute__((const));
int ispunct(int c) __attribute__((const));
int isspace(int c) __attribute__((const));
int isupper(int c) __attribute__((const));
int isxdigit(int c) __attribute__((const));

/* Character conversion - const (only uses argument) */
int tolower(int c) __attribute__((const));
int toupper(int c) __attribute__((const));

#ifdef __cplusplus
}
#endif

#endif /* _CBANG_CTYPE_H */
