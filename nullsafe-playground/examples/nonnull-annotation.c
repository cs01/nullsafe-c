void process(int* _Nonnull data) {
    *data = 42;
}

void example(int* _Nullable  x, int* _Nonnull y) {
    process(x);
    process(y);
}

void deref_twice_unsafe(int* _Nullable * _Nullable pp) {
    **pp = 42;
}

void deref_twice_safe(int* _Nullable * _Nullable pp) {
    if (pp && *pp) {
        **pp = 42;
    }
}

void deref_twice_partial(int* _Nullable * _Nullable pp) {
    if (pp) {
        **pp = 42;
    }
}