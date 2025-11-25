// Multi-level pointers require careful handling
void deref_twice(int** pp) {
    **pp = 42;  // warning - *pp might be null!
}

void safe_deref(int** pp) {
    if (pp && *pp) {
        **pp = 42;  // OK
    }
}