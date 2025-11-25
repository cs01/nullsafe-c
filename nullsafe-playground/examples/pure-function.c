// Const functions preserve narrowing across calls
int is_valid(char c) __attribute__((const));

void example(const char* data) {
    if (!data) return;
    const int isvalid = is_valid(*data);
    char val = *data;  // OK - narrowing preserved by const function
}