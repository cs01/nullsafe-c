// _Nonnull annotations guarantee non-null
void process(_Nonnull int* data) {
    *data = 42;  // OK - data is guaranteed non-null
}

void example(int* _Nullable x, int* _Nonnull y) {
    process(x);  // warning - passing nullable to nonnull
    process(y);  // OK
}