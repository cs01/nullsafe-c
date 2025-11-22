// RUN: %clang_cc1 -fsyntax-only -fblocks %s -verify

// Test the *! (starbang) syntax for non-nullable pointers
// This tests Phase 2.5: nullable-to-nonnull conversion errors

typedef int*! nonnull_int_ptr;
typedef int* nullable_int_ptr;

void takes_nonnull(int*! p) {
    *p = 42;  // Safe - p is nonnull
}

void takes_nullable(int* p) {
    if (p) {// Need null check for nullable
        *p = 42;
        // TODO: Invalidate narrowing after function calls that could modify the pointer
        // (see Phase 4 - narrowing invalidation)
        *p = 23;
    }
}

int*! returns_nonnull(void) {
    static int x = 100;
    return &x;
}

int* returns_nullable(void) {
    return 0;  // OK - nullable can be null
}

void test_basic_syntax(void) {
    int value = 10;
    int*! nonnull_ptr = &value;  // OK - address-of is always nonnull
    int* nullable_ptr = &value;  // OK
    takes_nonnull(&value);       // OK
    takes_nullable(&value);      // OK
}

void test_null_to_nonnull(void) {
    takes_nonnull(0); // expected-error{{null passed to a callee that requires a non-null argument}}
}

void test_nullable_to_nonnull(void) {
    int* nullable = returns_nullable();
    takes_nonnull(nullable); // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
}

void test_nonnull_to_nullable(void) {
    int*! nonnull = returns_nonnull();
    takes_nullable(nonnull);  // OK - implicit upcast
}

void test_assignment(void) {
    int value = 42;
    int*! nonnull;
    int* nullable = 0;

    nonnull = &value;     // OK
    nonnull = nullable;   // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
    nullable = nonnull;   // OK - implicit upcast
}

void test_function_pointers(void) {
    void (*_Nonnull fp1)(int*!) = takes_nonnull;  // OK
    void (*fp2)(int*!) = takes_nonnull;           // OK
}

void test_typedef(void) {
    nonnull_int_ptr p1;
    nullable_int_ptr p2;

    int x = 42;
    p1 = &x;  // OK
    p2 = &x;  // OK
    p2 = 0;   // OK - nullable can be null

    p1 = p2;  // expected-error{{implicit conversion from nullable pointer 'nullable_int_ptr _Nullable' (aka 'int *') to non-nullable pointer type 'nonnull_int_ptr' (aka 'int *')}}
}

void test_return_types(void) {
    int*! nonnull = returns_nullable();  // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
    int* nullable = returns_nonnull();   // OK - implicit upcast
}

// Test that defaults work correctly
void test_defaults(int* implicitly_nullable, int*! explicitly_nonnull) {
    takes_nullable(implicitly_nullable);   // OK
    takes_nonnull(implicitly_nullable);    // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
    takes_nullable(explicitly_nonnull);    // OK - implicit upcast
    takes_nonnull(explicitly_nonnull);     // OK
}

// Test multiple levels of indirection work at least syntactically
void test_multi_level(int** nullable_ptr_to_nullable,
                     int**! nonnull_ptr_to_nullable) {
    // Just test that these parse correctly
    // Full multi-level pointer support is Phase 3+
}

// ============================================================================
// Phase 3 Tests: Flow-sensitive type narrowing (IMPLEMENTED!)
// ============================================================================

// Phase 3 - null checks narrow nullable to nonnull
void test_flow_narrowing_basic(int* p) {
    if (p) {
        takes_nonnull(p);  // OK - p is narrowed to nonnull after null check
    }
}

// Phase 3 - explicit null comparison narrows type
void test_flow_narrowing_explicit(int* p) {
    if (p != 0) {
        takes_nonnull(p);  // OK - p is narrowed to nonnull after null check
    }
}

// This should always error - no null check
void test_flow_no_check(int* p) {
    takes_nonnull(p);  // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
}

// Phase 3 - type reverts to nullable after if block
void test_flow_after_if(int* p) {
    if (p) {
        takes_nonnull(p);  // OK - narrowed within if block
    }
    takes_nonnull(p);  // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
                      // Error - nullable again after if
}

// Phase 3 - else branch still nullable
void test_flow_else(int* p) {
    if (p) {
        takes_nonnull(p);  // OK - narrowed in then-branch
    } else {
        takes_nonnull(p);  // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
                          // Error - p is null here!
    }
}

// Test dereference narrowing
void test_flow_dereference(int* p) {
    if (p) {
        *p = 42;  // OK - p is narrowed to nonnull after null check
    }
}

void test_flow_dereference_no_check(int* p) {
    *p = 42;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
}
