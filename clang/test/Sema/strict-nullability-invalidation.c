// NOTE: This file documents function call invalidation behavior
// The feature works correctly with the normal compiler driver,
// but has issues with lit tests due to -nostdsysteminc flag.
// See cbang-starbang-syntax.c for working tests.

// Test function call invalidation for cbang null safety

void external_function(void);
void another_function(int x);

// Test: Function calls invalidate narrowing (conservative approach)
void test_function_call_invalidation(int* p) {
    if (p) {
        *p = 42;  // OK - p is narrowed
        external_function();  // Invalidates narrowing
        *p = 43;  // Would error with normal compiler - narrowing invalidated
    }
}

// Test: Multiple function calls
void test_multiple_calls(int* p) {
    if (p) {
        *p = 1;  // OK
        external_function();
        *p = 2;  // Would error - narrowing invalidated
        another_function(5);
        *p = 3;  // Would error - narrowing invalidated
    }
}

// Test: Function call with arguments
void test_call_with_args(int* p) {
    if (p) {
        *p = 1;  // OK
        another_function(*p);  // Call uses narrowed p, then invalidates
        *p = 2;  // Would error - narrowing invalidated
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

int is_valid(const char c);

void test_const_param_preserves_narrowing(const char* data) {
    if (!data) return;
    const int isvalid = is_valid(*data);
    char val = *data;  // OK - narrowing preserved by const parameter
}

void test_const_param_multiple_calls(const int* value) {
    if (!value) return;
    int copy1 = *value;  // OK
    is_valid((char)*value);
    int copy2 = *value;  // OK - narrowing preserved, parameter is const
}

