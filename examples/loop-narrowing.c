// Loop conditions narrow the variable
void process_items(int* items, int count) {
    for (int i = 0; items && i < count; i++) {
        items[i] = 0;  // OK - items is non-null in loop
    }
}