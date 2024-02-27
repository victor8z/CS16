#include "xps.h"

// Helper function to create a new XPS string by replacing all occurrences of 'old_pattern' with 'new_pattern' in 'str'.
char* replace_in_xps_string(const char* str, const char* old_pattern, const char* new_pattern) {
    size_t str_len = xps_length(str);
    size_t old_len = xps_length(old_pattern);
    // Removed unused variable 'new_len'
    char* result = xps_from_cstr(""); // Start with an empty XPS string.

    size_t i = 0;
    while (i < str_len) {
        // Check if 'old_pattern' is found starting at position 'i'.
        bool match_found = true;
        if (i + old_len <= str_len) { // Ensure there are enough characters left in 'str' for a match.
            for (size_t j = 0; j < old_len; ++j) {
                if (xps_getchar(str, i + j) != xps_getchar(old_pattern, j)) {
                    match_found = false;
                    break;
                }
            }
        } else {
            match_found = false;
        }

        if (match_found) {
            // If 'old_pattern' is found, concatenate 'new_pattern' to 'result'.
            char* temp = xps_concat(result, new_pattern);
            xps_free(result);
            result = temp;
            i += old_len; // Skip past 'old_pattern' in 'str'.
        } else {
            // If 'old_pattern' is not found, concatenate the current character to 'result'.
            char current_char[2] = {xps_getchar(str, i), '\0'}; // Create a temporary C-string.
            char* temp_char_xps = xps_from_cstr(current_char); // Convert it to an XPS string.
            char* temp_result = xps_concat(result, temp_char_xps);
            xps_free(result);
            xps_free(temp_char_xps);
            result = temp_result;
            ++i; // Move to the next character.
        }
    }

    return result; // Return the new XPS string with replacements.
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        char* message = xps_from_cstr("USAGE: replace [old] [new]\n");
        xps_write(message);
        xps_free(message);
        return 1;
    }

    char* line;
    while ((line = xps_readline()) != nullptr) {
        char* old_pattern_xps = xps_from_cstr(argv[1]);
        char* new_pattern_xps = xps_from_cstr(argv[2]);
        char* replaced_line = replace_in_xps_string(line, old_pattern_xps, new_pattern_xps);

        xps_writeline(replaced_line);

        xps_free(old_pattern_xps);
        xps_free(new_pattern_xps);
        xps_free(replaced_line);
        xps_free(line);
    }

    return 0;
}
