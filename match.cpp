#include "xps.h"

// Helper function to check if a given segment matches the beginning of the text.
bool segmentMatches(const char* segment, const char* text) {
    for (size_t i = 0; i < xps_length(segment); ++i) {
        if (xps_getchar(segment, i) != xps_getchar(text, i)) {
            return false;
        }
    }
    return true;
}

bool matchesWildcard(const char* pattern, const char* text) {
    // The current segment we're matching.
    char* segment = nullptr;
    
    // Start and end pointers for the current segment within the pattern.
    const char* segment_start = pattern;
    const char* segment_end = pattern;

    // Are we at the start of a new segment?
    bool new_segment = true;

    while (*segment_end != '\0') {
        if (*segment_end == '*' || *(segment_end + 1) == '\0') {
            // If it's not the first character and it's either a '*' or the end of the pattern.
            if (new_segment && *segment_end == '*') {
                segment_start = segment_end + 1; // Move to the character after '*'.
            } else {
                // Slice the segment out of the pattern.
                segment = xps_slice(segment_start, 0, *segment_end == '*' ? segment_end - segment_start : segment_end - segment_start + 1);
                new_segment = true;
                
                // Loop through text to find the segment.
                while (*text != '\0') {
                    if (segmentMatches(segment, text)) {
                        // Move text pointer to the end of the matching segment.
                        text += xps_length(segment);
                        break;
                    }
                    ++text;
                }
                
                // Clean up the segment.
                xps_free(segment);
                
                // If we've reached the end of the text before matching the current segment.
                if (*text == '\0' && *segment_end != '\0') {
                    return false;
                }
            }
        } else {
            new_segment = false;
        }
        ++segment_end;
    }

    // If pattern ends with '*', we have a match regardless of the remaining text.
    // Otherwise, we must reach the end of both pattern and text together for a match.
    return *segment_end == '*' || *text == '\0';
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        char* message = xps_from_cstr("USAGE: match [pattern]\n");
        xps_write(message);
        xps_free(message);
        return 1;
    }

    char* pattern = xps_from_cstr(argv[1]);
    char* line;

    while ((line = xps_readline()) != nullptr) {
        if (xps_find(pattern, "*", 0) != XPS_NPOS) { // Check if pattern contains a '*'
            if (matchesWildcard(pattern, line)) {
                char* message = xps_from_cstr("Match!\n");
                xps_write(message);
                xps_free(message);
            } else {
                char* message = xps_from_cstr("No match.\n");
                xps_write(message);
                xps_free(message);
            }
        } else {
            // If there's no wildcard, use substring matching.
            if (xps_find(line, pattern, 0) != XPS_NPOS) {
                char* message = xps_from_cstr("Match!\n");
                xps_write(message);
                xps_free(message);
            } else {
                char* message = xps_from_cstr("No match.\n");
                xps_write(message);
                xps_free(message);
            }
        }
        xps_free(line);
    }

    xps_free(pattern);
    return 0;
}