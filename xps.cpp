
#include "xps.h"

size_t xps_length(const char* str) {
    size_t length = 0; // Initialize length as size_t for proper size handling.

    // Check for an empty string.
    if (str[0] == 0x00) {
        return 0; // The string is empty.
    }

    // Loop over the chunks in the string.
    while (true) {
        unsigned char chunkLength = *str & 0x0F; // Extract the length of the current chunk.

        length += chunkLength; // Add the length of this chunk to the total length.

        // If the chunk length is less than 15, it's either the end or a short chunk.
        if (chunkLength < 15) {
            break; // We've reached the end of the string.
        }

        // Move to the next chunk: skip over the current chunk's characters and the length byte.
        str += chunkLength + 1;

        // Check for the end of the string indicated by an empty chunk.
        if (*str == 0x00) {
            break; // The string ends with an empty chunk.
        }
    }
    
    return length; // Return the total length of the string.
}


char* xps_slice(const char* str, size_t start) {
    size_t str_length = xps_length(str);
    if (start >= str_length) {
        return new char[2]{0x00, 0x00}; // Empty string with length byte and null terminator.
    }

    size_t slice_length = str_length - start;
    // Calculate the number of chunks needed for the new slice.
    size_t num_chunks = (slice_length + 14) / 15;
    char* slice = new char[slice_length + num_chunks + 1](); // Initialize with zeros.

    if (slice == nullptr) {
        return nullptr; // Allocation failed.
    }

    size_t slice_index = 0;
    for (size_t i = 0; i < slice_length; ++i) {
        if (i % 15 == 0) { // At the start of a new chunk
            slice[slice_index++] = (slice_length - i > 15) ? 15 : (slice_length - i);
        }
        slice[slice_index++] = xps_getchar(str, start + i);
    }

    slice[slice_index] = 0x00; // Null-terminate the slice.
    return slice;
}

char* xps_slice(const char* str, size_t start, size_t stop) {
    size_t str_length = xps_length(str);
    if (start >= str_length || start >= stop) {
        return new char[2]{0x00, 0x00}; // Empty string with length byte and null terminator.
    }

    if (stop > str_length) {
        stop = str_length;
    }

    size_t slice_length = stop - start;
    // Calculate the number of chunks needed for the new slice.
    size_t num_chunks = (slice_length + 14) / 15;
    char* slice = new char[slice_length + num_chunks + 1](); // Initialize with zeros.

    if (slice == nullptr) {
        return nullptr; // Allocation failed.
    }

    size_t slice_index = 0;
    for (size_t i = 0; i < slice_length; ++i) {
        if (i % 15 == 0) { // At the start of a new chunk
            slice[slice_index++] = (slice_length - i > 15) ? 15 : (slice_length - i);
        }
        slice[slice_index++] = xps_getchar(str, start + i);
    }

    slice[slice_index] = 0x00; // Null-terminate the slice.
    return slice;
}

int xps_compare(const char* lhs, const char* rhs) {
    size_t lhs_length = xps_length(lhs);
    size_t rhs_length = xps_length(rhs);
    size_t min_length = (lhs_length < rhs_length) ? lhs_length : rhs_length;

    // Compare characters one by one using xps_getchar
    for (size_t i = 0; i < min_length; ++i) {
        char lhs_char = xps_getchar(lhs, i);
        char rhs_char = xps_getchar(rhs, i);

        if (lhs_char != rhs_char) {
            return lhs_char - rhs_char;
        }
    }

    // If all characters are the same but lengths are different, the shorter string is considered "less"
    if (lhs_length != rhs_length) {
        return (lhs_length < rhs_length) ? -1 : 1;
    }

    // If both strings are the same length and all characters are the same, they are equal
    return 0;
}

char* xps_concat(const char* lhs, const char* rhs) {
    size_t lhs_len = xps_length(lhs); // Get the length of lhs string.
    size_t rhs_len = xps_length(rhs); // Get the length of rhs string.
    size_t total_len = lhs_len + rhs_len; // Calculate total length.

    // Calculate how many chunks we need for the concatenated string.
    size_t total_chunks = (total_len + 14) / 15;
    size_t concat_len = total_len + total_chunks + 1; // +1 for the null terminator.

    // Allocate space for the concatenated string.
    char* concat_str = new char[concat_len](); // Initialize with zeros.

    // Check for allocation failure.
    if (!concat_str) {
        return nullptr;
    }

    size_t concat_index = 0; // Current position in concat_str.
    size_t chunk_len = 0; // Current chunk length.
    size_t chunk_index = 0; // Current chunk index.

    // Copy characters from lhs into concat_str.
    for (size_t i = 0; i < lhs_len; ++i) {
        if (chunk_len == 0) {
            chunk_index = concat_index; // Remember the position of the chunk length byte.
            concat_index++; // Skip the position of the chunk length byte, it will be filled later.
            chunk_len = 15; // Reset chunk length.
        }
        concat_str[concat_index++] = xps_getchar(lhs, i);
        chunk_len--;

        // When the end of a chunk is reached, go back and set the chunk length byte.
        if (chunk_len == 0 || i == lhs_len - 1) {
            concat_str[chunk_index] = (concat_index - chunk_index - 1); // Set the chunk length byte.
        }
    }

    // Copy characters from rhs into concat_str.
    for (size_t i = 0; i < rhs_len; ++i) {
        if (chunk_len == 0) {
            chunk_index = concat_index; // Remember the position of the chunk length byte.
            concat_index++; // Skip the position of the chunk length byte, it will be filled later.
            chunk_len = 15; // Reset chunk length.
        }
        concat_str[concat_index++] = xps_getchar(rhs, i);
        chunk_len--;

        // When the end of a chunk is reached, go back and set the chunk length byte.
        if (chunk_len == 0 || i == rhs_len - 1) {
            concat_str[chunk_index] = (concat_index - chunk_index - 1); // Set the chunk length byte.
        }
    }

    // Null-terminate the concatenated string.
    concat_str[concat_index] = '\0';

    return concat_str;
}


size_t xps_find(const char* str, const char* pattern) {
    size_t pattern_length = xps_length(pattern);
    size_t str_len = xps_length(str);

    // Edge case: empty pattern should always match at 0
    if (pattern_length == 0) {
        return 0;
    }

    // If the pattern is longer than the string, or the string is empty, return not found.
    if (pattern_length > str_len || str_len == 0) {
        return XPS_NPOS;
    }

    // Iterate through the string and compare substrings with the pattern.
    for (size_t index = 0; index <= str_len - pattern_length; ++index) {
        bool match = true;
        for (size_t p = 0; p < pattern_length; ++p) {
            if (xps_getchar(str, index + p) != xps_getchar(pattern, p)) {
                match = false;
                break;
            }
        }
        if (match) {
            return index;
        }
    }

    return XPS_NPOS;
}

size_t xps_find(const char* str, const char* pattern, size_t start) {
    size_t pattern_length = xps_length(pattern);
    size_t str_len = xps_length(str);

    // Edge case: empty pattern should always match at start
    if (pattern_length == 0) {
        return start;
    }

    // If the start index is beyond the string length, or the pattern is longer than the string, return not found.
    if (start >= str_len || pattern_length > str_len - start) {
        return XPS_NPOS;
    }

    // Iterate through the string starting at 'start' and compare substrings with the pattern.
    for (size_t index = start; index <= str_len - pattern_length; ++index) {
        bool match = true;
        for (size_t p = 0; p < pattern_length; ++p) {
            if (xps_getchar(str, index + p) != xps_getchar(pattern, p)) {
                match = false;
                break;
            }
        }
        if (match) {
            return index;
        }
    }

    return XPS_NPOS;
}


char xps_getchar(const char* str, size_t index) {
    while (*str != 0x00) { // Continue until an empty chunk is encountered.
        unsigned char chunkLength = *str & 0x0F; // Get the number of characters in the current chunk.

        // If the index is within the range of the current chunk, return the character.
        if (index < chunkLength) {
            return *(str + 1 + index); // The character is offset by 1 byte from the length indicator.
        }

        // Adjust the index for the next chunk.
        index -= chunkLength;

        // Move the pointer to the start of the next chunk.
        str += chunkLength + 1;

        // If the next chunk is the end of the string, break early.
        if (*str == 0x00) {
            break;
        }
    }

    // If the function hasn't returned yet, the index was out of bounds.
    return '\0'; // This could be an error character or a sentinel value indicating an invalid index.
}

void xps_setchar(char* str, size_t index, char c) {
    while (*str != 0x00) { // Continue until the end of the string
        unsigned char chunkLength = *str & 0x0F; // Get the length of the current chunk

        // Check if the index is within this chunk
        if (index < chunkLength) {
            *(str + 1 + index) = c; // Set the character at the specified index
            return; // Character set successfully, exit the function
        }
        
        // Deduct the length of this chunk from the index and move to the next chunk
        index -= chunkLength;
        str += 1 + chunkLength; // Move past the length byte and the characters in this chunk

        // If the next chunk length is 0, this was the last chunk
        if (*str == 0x00) {
            // If index is not within the string, handle as an error or simply return without doing anything
            return;
        }
    }
}


void xps_free(char* str) {
    delete[] str; 
}

char* xps_from_cstr(const char* cstr) {
    size_t len = 0;
    while (cstr[len] != '\0') len++; // Manually calculate the length of cstr
    
    size_t xp_len = len / 15 + (len % 15 ? 1 : 0) + 1; // Calculate the required number of chunks, including the empty chunk
    char* xp_str = new char[xp_len * 16]; // Allocate memory for the XP string using new

    char* xp_ptr = xp_str;
    while (*cstr != '\0') {
        size_t chunk_len = 0;
        *xp_ptr = 0; // Initialize the length byte
        ++xp_ptr;
        while (*cstr != '\0' && chunk_len < 15) {
            *xp_ptr++ = *cstr++;
            ++chunk_len;
        }
        *(xp_ptr - chunk_len - 1) = chunk_len; // Set the length byte
    }
    *xp_ptr = 0; // Add an empty chunk to mark the end
    return xp_str;
}