// RUN: %clang -fsyntax-only %s -Xclang -verify

// Test function call invalidation for cbang null safety

void external_function(void);
void another_function(int x);

// Test: Function calls invalidate narrowing (conservative approach)
void test_function_call_invalidation(int* p) {
    if (p) {
        *p = 42;  // OK - p is narrowed
        external_function();  // Invalidates narrowing
        *p = 43;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
    }
}

// Test: Multiple function calls
void test_multiple_calls(int* p) {
    if (p) {
        *p = 1;  // OK
        external_function();
        *p = 2;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
        another_function(5);
        *p = 3;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
    }
}

// Test: Function call with arguments
void test_call_with_args(int* p) {
    if (p) {
        *p = 1;  // OK
        another_function(*p);  // Call uses narrowed p, then invalidates
        *p = 2;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
    }
}

// Test: Narrowing persists without function calls
void test_no_invalidation_without_calls(int* p) {
    if (p) {
        *p = 1;  // OK
        int x = 5;
        x = x + 1;
        *p = 2;  // OK - no function call, so narrowing still valid
    }
}
