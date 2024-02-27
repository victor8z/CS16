#include "xps.h"

int main() {
    char* line;
    while ((line = xps_readline()) != nullptr) { // Read lines until EOF.
        size_t length = xps_length(line);

        if (length <= 20) {
            // If the line is 20 characters or fewer, print it unchanged.
            xps_writeline(line);
        } else {
            // If the line is longer than 20 characters, print the truncated version.
            char* start_slice = xps_slice(line, 0, 10); // First 10 characters.
            char* end_slice = xps_slice(line, length - 7, length); // Last 7 characters.

            // Concatenate the slices with the ellipsis.
            char* ellipsis = xps_from_cstr("...");
            char* temp_concat = xps_concat(start_slice, ellipsis);
            char* final_concat = xps_concat(temp_concat, end_slice);

            // Print the truncated line.
            xps_writeline(final_concat);

            // Free the memory allocated for slices and temporary strings.
            xps_free(start_slice);
            xps_free(end_slice);
            xps_free(ellipsis);
            xps_free(temp_concat);
            xps_free(final_concat);
        }

        // Free the memory allocated for the line.
        xps_free(line);
    }

    return 0;
}
