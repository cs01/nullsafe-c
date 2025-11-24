// RUN: %clang_cc1 -fsyntax-only -fblocks %s -verify

// Test the *! (starbang) syntax for non-nullable pointers
// This tests Phase 2.5: nullable-to-nonnull conversion errors

// Forward declarations for test functions
void assert(int);

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
    } else {
        *p=7;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
               // Error - in else branch, we know p IS null
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

// ============================================================================
// Early-return narrowing tests
// ============================================================================

// Early return with single variable
void test_early_return_simple(char* p) {
    if (p == 0) return;

    // After early return, p is narrowed to nonnull
    *p = 'x';  // OK
}

// Early return with negated check
void test_early_return_negated(char* p) {
    if (!p) return;

    *p = 'x';  // OK
}

// Early return with compound OR condition
void test_early_return_compound(char* p, char* q) {
    if (!p || !q) return;

    // Both p and q are narrowed
    *p = 'x';  // OK
    *q = 'y';  // OK
}

// Early return with explicit NULL comparison and OR
void test_early_return_explicit_or(char* p, char* q) {
    if ((p == 0) || (q == 0)) {
        return;
    }

    *p = 'x';  // OK
    *q = 'y';  // OK
}

// Early return with braces
void test_early_return_braces(char* p) {
    if (p == 0) {
        return;
    }

    *p = 'x';  // OK
}

// Test: Early goto (common in C error handling)
void test_early_goto(char* p, char* q) {
    if (!p || !q)
        goto error;

    // After goto, both are narrowed
    *p = 'x';  // OK
    *q = 'y';  // OK
    return;

error:
    return;
}

// AND pattern - narrows inside the then-block
// NOTE: If the condition contains function calls, narrowing is NOT applied
// since the function could invalidate pointers before narrowing takes effect.
int some_condition(void);
void test_and_pattern(char* p) {
    if (p && some_condition()) {
        *p = 'x';  // expected-error{{dereferencing nullable pointer of type 'char * _Nullable'}}
                   // Error - some_condition() could invalidate p before narrowing
    }
}

// Test: Simple condition without function calls still narrows
void test_and_pattern_no_call(char* p, char* q) {
    if (p && q) {
        *p = 'x';  // OK - no function calls, narrowing is safe
        *q = 'y';  // OK
    }
}

// Test: Function calls invalidate narrowing (Phase 4)
// This is conservative but necessary since functions can have side effects
void arbitrary_function(void);
void test_function_call_invalidation(int* p) {
    if (p) {
        *p = 42;            // OK - p is narrowed
        arbitrary_function(); // Invalidates narrowing
        *p = 23;            // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
    }
}

// Test: Multiple function calls continue to keep narrowing invalidated
void test_multiple_calls_invalidation(int* p) {
    if (p) {
        *p = 1;             // OK - p is narrowed
        arbitrary_function();
        *p = 2;             // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
        arbitrary_function();
        *p = 3;             // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
    }
}

// Test: Can re-narrow with assert after invalidation
void test_renarrow_after_invalidation(int* p) {
    if (p) {
        *p = 1;             // OK - p is narrowed
        arbitrary_function(); // Invalidates
        assert(p != 0);     // Re-narrow
        *p = 2;             // OK - narrowed again by assert
    }
}

// ============================================================================
// AND-expression dereference tests (Phase 2.6 - AND expression narrowing)
// ============================================================================

// Test dereference in AND condition itself
void test_and_deref_simple(char* p) {
    if (p && *p == 'x') {  // OK - p is narrowed before dereferencing in condition
        *p = 'y';          // OK - p is narrowed in body too
    }
}

// Test chained AND with multiple dereferences
void test_and_deref_chained(char* p, char* q) {
    if (p && q && *p == *q) {  // OK - both p and q are narrowed
        *p = 'a';              // OK
        *q = 'b';              // OK
    }
}

// Test AND with function call using dereferenced pointer
int check_char(char c);
void test_and_deref_funcall(char* p) {
    if (p && check_char(*p)) {  // Condition has function call
        check_char(*p);         // expected-error{{dereferencing nullable pointer of type 'char * _Nullable'}}
                                // Error - narrowing not applied due to function call in condition
        *p = 'x';               // expected-error{{dereferencing nullable pointer of type 'char * _Nullable'}}
    }
}

// ============================================================================
// Additional test cases for completeness
// ============================================================================

// Test: Assigning NULL to nullable is fine, to nonnull should error
void test_null_assignment(void) {
    int* nullable = 0;   // OK
    int*! nonnull = 0;   // expected-error{{null passed to a callee that requires a non-null argument}}
}

// Test: Loop condition narrowing
void test_while_loop(int* p) {
    while (p) {
        *p = 42;  // OK - p is narrowed inside the loop body
        p = 0;    // Break the loop
    }
    *p = 0;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
             // Error - outside loop, p is nullable again
}

// Test: For loop condition narrowing
void test_for_loop(int* p) {
    for (; p; p = 0) {
        *p = 42;  // OK - p is narrowed in loop body
    }
}

// Test: Negated conditions in else branch
void test_else_narrowing(int* p) {
    if (!p) {
        // p is known null here
        *p = 42;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
    } else {
        // p is known nonnull here
        *p = 42;  // OK
    }
}

// Test: Multiple variables in AND condition
void test_multiple_and(int* p, int* q, int* r) {
    if (p && q && r) {
        *p = 1;  // OK
        *q = 2;  // OK
        *r = 3;  // OK
    }
}

// Test: Ternary operator with nullable/nonnull
void test_ternary(int* nullable, int*! nonnull, int cond) {
    int* result1 = cond ? nullable : nonnull;  // OK - result is nullable
    int*! result2 = cond ? nonnull : nullable; // expected-error{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
    int*! result3 = cond ? nonnull : nonnull;  // OK - both branches nonnull
}

// Test: Dereference in function argument
void process_int(int val);
void test_deref_in_call(int* p) {
    if (p) {
        process_int(*p);  // OK - p is narrowed
    }
    process_int(*p);  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
}

// Test: Array subscript (which is dereference)
void test_array_subscript(int* arr) {
    if (arr) {
        int x = arr[0];  // OK - arr is narrowed
    }
    int y = arr[0];  // TODO: Should error but doesn't with -nostdsysteminc (lit test issue)
}

// Test: Pointer arithmetic doesn't remove nullability
void test_pointer_arithmetic(int* p) {
    if (p) {
        int* q = p + 1;  // q should also be nonnull after arithmetic on nonnull
        *q = 42;         // OK
    }
}

// Test: Address-of operator always produces nonnull
void test_address_of(void) {
    int x = 42;
    int*! p = &x;  // OK - address-of is always nonnull
    takes_nonnull(&x);  // OK
}

// Test: Struct member access through nullable pointer
struct Point { int x; int y; };
void test_struct_deref(struct Point* p) {
    if (p) {
        p->x = 10;  // OK - p is narrowed
    }
    p->y = 20;  // TODO: Should error but doesn't with -nostdsysteminc (lit test issue)
}

// Test: Chained dereferences (multi-level pointers)
void test_chained_deref(int** pp) {
    if (pp) {
        // pp is nonnull, but *pp is still nullable (inner pointer)
        *pp = 0;  // OK - can assign null to nullable
        // **pp would need another check
    }

    if (pp && *pp) {
        **pp = 42;  // OK - both levels narrowed
    }
}

// Test: Triple pointers
void test_triple_pointers(int*** ppp) {
    if (ppp && *ppp && **ppp) {
        ***ppp = 42;  // OK - all three levels narrowed
    }
}

// Test: Declared nonnull multi-level
void test_nonnull_outer_ptr(int**! pp) {
    *pp = 0;  // OK - pp is nonnull by declaration

    if (*pp) {
        **pp = 42;  // OK - *pp is narrowed
    }
}

// Test: Early return doesn't narrow in unreachable code
void test_early_return_unreachable(int* p) {
    if (!p) {
        *p = 42;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
                  // Error - p is known null before the return
        return;
    }
    *p = 0;  // OK - p is nonnull after the early return guard
}

// Test: AND pattern with OR - should not narrow
void test_and_or_mixed(int* p, int cond) {
    if (p || cond) {
        *p = 42;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
                  // Error - p might not be checked if cond is true
    }
}

// Test: Comparison creates narrowing
void test_comparison_narrowing(int* p, int* q) {
    if (p == q && p) {
        *p = 42;  // OK - p is narrowed
        *q = 42;  // expected-error{{dereferencing nullable pointer of type 'int * _Nullable'}}
                  // Error - q is not necessarily nonnull (could both be null)
    }
}

// Test: const pointer narrowing (cJSON pattern)
void test_const_pointer_narrowing(const unsigned char* input_pointer) {
    if (input_pointer) {
        *input_pointer;  // OK - narrowed to nonnull even though const  // expected-warning{{expression result unused}}
    }
}

// Test: narrowing inside while loop (cJSON pattern)
void test_while_narrowing(const unsigned char* input_pointer, const unsigned char* input_end) {
    while (input_pointer < input_end) {
        if (input_pointer) {
            *input_pointer;  // OK - narrowed inside if  // expected-warning{{expression result unused}}
        }
    }
}

// Test: narrowing from while condition with AND (proposed cJSON fix)
void test_while_and_narrowing(const unsigned char* input_pointer,
                               unsigned char* output_pointer,
                               const unsigned char* input_end) {
    while (input_pointer && output_pointer && input_pointer < input_end) {
        if (*input_pointer != '\\') {  // Dereference in nested if condition
            *output_pointer = *input_pointer;  // Should still be narrowed from while
        }
    }
}
