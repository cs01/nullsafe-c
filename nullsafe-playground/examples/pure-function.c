// Pure functions preserve narrowing across calls
int is_valid(int* ptr) {
    return ptr != 0;  // Pure function
}

void example(int* data) {
    if (is_valid(data)) {
        *data = 42;  // OK - narrowing preserved
    }
}