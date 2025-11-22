// RUN: %clang_cc1 -fsyntax-only -fblocks -Wnullable-to-nonnull-conversion %s -verify

// Test the *! (starbang) syntax for non-nullable pointers

#include <stdio.h>
typedef int*! nonnull_int_ptr;
typedef int* nullable_int_ptr;

void takes_nonnull(int*! p) {
    *p = 42;
}

void takes_nullable(int* p) {
    if (p) *p = 42;
}

int*! returns_nonnull(void) {
    static int x = 100;
    return &x;
}

int* returns_nullable(void) {
    return 0;
}

int foo(int* p1) {
    return *p1 + 1;
}

void test_basic_syntax(void) {
    int value = 10;
    int*! nonnull_ptr = NULL;
    int* nullable_ptr = &value;
    printf("%d %d", *nonnull_ptr, *nullable_ptr);
    takes_nonnull(&value);
    takes_nullable(&value);
}

void test_null_to_nonnull(void) {
    takes_nonnull(0); // expected-warning{{null passed to a callee that requires a non-null argument}}
}

void test_nullable_to_nonnull(void) {
    int* nullable = returns_nullable();
    takes_nonnull(nullable); // expected-warning{{implicit conversion from nullable pointer 'int *' to non-nullable pointer type 'int * _Nonnull'}}
}

void test_nonnull_to_nullable(void) {
    int*! nonnull = returns_nonnull();
    takes_nullable(nonnull);
}

void test_function_pointers(void) {
    void (*_Nonnull fp1)(int*!) = takes_nonnull;
    void (*fp2)(int*!) = takes_nonnull;
}

void test_typedef(void) {
    nonnull_int_ptr p1;
    nullable_int_ptr p2;

    int x = 42;
    p1 = &x;
    p2 = &x;
    p2 = 0;
}
