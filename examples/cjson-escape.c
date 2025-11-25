// From cJSON.c:1019-1027 (print_string_ptr function)
// https://github.com/DaveGamble/cJSON/blob/c859b25da0/cJSON.c#L1019-L1027
// Null-safety warning: dereferencing nullable pointer at line 1022
#define NULL ((void*)0)

typedef unsigned char * output_t;

// Simplified version of actual cJSON code with null-safety warning
void escape_json_string(const unsigned char *input, output_t output_pointer) {
    *output_pointer++ = '"';

    // Line 1022: warning on *input_pointer dereference
    for (const unsigned char *input_pointer = input;
         *input_pointer != '\0';  // warning: input_pointer might be NULL!
         (void)input_pointer++, output_pointer++) {

        if ((*input_pointer > 31) && (*input_pointer != '"')) {
            *output_pointer = *input_pointer;  // warnings here too
        } else {
            *output_pointer++ = '\\';
            *output_pointer = *input_pointer;
        }
    }
    *output_pointer = '"';
}

void example(void) {
    unsigned char buf[100];
    escape_json_string(NULL, buf);  // Crash!
}