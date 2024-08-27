#include <iostream>
#include <string>
#include <cstring>
#include <assert.h>

bool match_once_or_more(const char*& text, const char*& pattern);

bool _match_once_or_more(const char*& text, char* sub_pattern, const char*& pattern);

bool match_zero_or_one(const char*& text, char* pattern);

bool match_symbol(const char*& text, const char*& pattern);

bool match_character_group(const char*& text, char*& pattern);

bool match_captured_pattern(const char*& text, const char*& pattern);

bool match_pattern(const std::string& input_line, const std::string& pattern);

int* create_int_array_backup(int* array, int array_size);

const char** create_char_array_backup(const char** array, int array_size);



const char** captured_patterns;   // Storing any captured patterns.
int num_captured_patterns;  // Length of the array.

const char** captured_pattern_stack; // Temporary stack for processing captured patterns
int captured_pattern_stack_len;

int* captured_indices_stack; // Temporary stack for processing captured patterns
int captured_indices_stack_len;


// Wanted to keep this kinda low level if that makes sense. Didn't use strings and such for the
// actual pattern matching, wanted to keep it as close to C as I could. (Just trynna learn)

// Also, did not clean it up whatsoever, got lazy once i finished the prototype and got the tests to pass
//  (tried but gave up and moved on at the first hurdle lol).


int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cout << "Logs from your program will appear here" << std::endl;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    std::string pattern = argv[2];

    if (flag != "-E") {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }

    std::string input_line;
    std::getline(std::cin, input_line);

    try {
        if (match_pattern(input_line, pattern)) {
            std::cout << "Pattern matched: " << input_line << " " << pattern << std::endl;
            return 0;
        }
        else {
            std::cout << "Pattern not matched: " << input_line << " " << pattern << std::endl;
            return 1;
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool match_pattern(const std::string& input_line, const std::string& pattern_string) {
    // Starting point for the matcher. Handles any special patterns like character groups itself. Otherwise calls match_symbol.
    const char* text_ptr = input_line.c_str(); // Pointer to iterate over the input_line. (Wanted to avoid creating a new copy)
    const char* pattern_ptr = pattern_string.c_str(); // Pointer to iterate over the pattern.

    // Special Matches
    if (pattern_ptr[0] == '^') { // Start of String Anchor
        const char* pattern = pattern_ptr + 1;
        const char* text = text_ptr;

        captured_patterns = (const char**)malloc(sizeof(char*) * pattern_string.length());
        num_captured_patterns = 0;

        captured_pattern_stack = (const char**)malloc(sizeof(char*) * pattern_string.length());
        captured_pattern_stack_len = 0;

        captured_indices_stack = (int*)malloc(sizeof(int) * pattern_string.length());

        // Avoids the loop below, so only matches at the start.
        const bool result = match_symbol(text, pattern);

        free(captured_patterns);
        free(captured_pattern_stack);
        free(captured_indices_stack);

        return result;
    }

    // Standard Match
    do {  // Iterate over the text and attempt to match the pattern.
        const char* text = text_ptr;
        const char* pattern = pattern_ptr;

        captured_patterns = (const char**)malloc(pattern_string.length());
        num_captured_patterns = 0;

        captured_pattern_stack = (const char**)malloc(pattern_string.length());
        captured_pattern_stack_len = 0;

        captured_indices_stack = (int*)malloc(sizeof(int) * pattern_string.length());


        if (match_symbol(text, pattern)) {
            std::cout << "MATCH SUCCESS!!!!!!!!!!!" << std::endl;

            free(captured_pattern_stack);
            free(captured_patterns);

            return true;
        }

        free(captured_pattern_stack);
        free(captured_patterns);

    } while (*text_ptr++ != '\0');

    return false;
}

bool match_symbol(const char*& text, const char*& pattern) {
    // // // Logging lines for debugging.
    // std::cout << std::endl;
    // std::cout << "Matching Symbol...";
    // std::cout << "|text: " << text;
    // std::cout << "|pattern: |" << pattern << std::endl;

    // Base cases
    if (pattern[0] == '\0') {
        return true;
    }
    if (pattern[0] == '$' && (pattern[1] == '\0')) { // End of string anchor
        return *text == '\0';
    }
    if (pattern[0] == '|') {
        while (pattern[0] != ')') pattern++;
        return match_symbol(text, pattern);
    }
    if (pattern[0] == ')') { // End of the either/or matcher. Means we matched the second pattern. 
        // Backup for the global variables in case we need to backtrack...
        //              yes this is shit :(
        const int _num_captured_patterns = num_captured_patterns;
        const int _captured_indices_stack_len = captured_indices_stack_len;
        const int _captured_patterns_stack_len = captured_pattern_stack_len;

        int* _captured_indices_stack = create_int_array_backup(captured_indices_stack, _captured_patterns_stack_len);
        const char** _captured_pattern_stack = create_char_array_backup(captured_pattern_stack, captured_pattern_stack_len);
        // End of the shit(or shitter ig) parts.

        const int captured_text_index = captured_indices_stack[captured_pattern_stack_len - 1];

        const char* captured_text_start = captured_pattern_stack[captured_pattern_stack_len - 1];
        const int captured_text_length = text - captured_text_start;

        char* captured_text = (char*)malloc(captured_text_length + 1);
        strncpy(captured_text, captured_text_start, captured_text_length);
        captured_text[captured_text_length] = '\0';

        captured_patterns[captured_text_index] = captured_text;

        captured_pattern_stack_len--;


        if (match_symbol(text, ++pattern)) {
            return true;
        }
        else {
            // Reset global vars.
            captured_indices_stack_len = _captured_indices_stack_len;
            captured_pattern_stack_len = _captured_patterns_stack_len;

            captured_pattern_stack = _captured_pattern_stack;
            captured_indices_stack = _captured_indices_stack;

            return false;
        }
    }
    if (text[0] == '\0') {
        return false;
    }
    if (pattern[0] == '(') { // Optional and/or captured matching.
        // return match_captured_pattern(text, pattern);

        const char* text_start = text;
        const char* pattern_start = pattern;

        // This will be the n'th captured pattern, n => number of captured patterns so far. 
        captured_pattern_stack[captured_pattern_stack_len] = text_start;
        captured_indices_stack[captured_pattern_stack_len] = num_captured_patterns; // will be the index we store it at.

        captured_pattern_stack_len++;
        num_captured_patterns++;


        if (match_symbol(text, ++pattern)) {
            return true;
        };

        pattern = pattern_start;

        num_captured_patterns--;
        captured_pattern_stack_len--;

        while (pattern[0] != '\0' && pattern[0] != '|') pattern++;

        if (*pattern == '|') {
            captured_pattern_stack[captured_pattern_stack_len] = text_start;
            captured_indices_stack[captured_pattern_stack_len] = num_captured_patterns;

            num_captured_patterns++;
            captured_pattern_stack_len++;
            if (match_symbol(text, ++pattern)) {
                return true;
            }
        }

        num_captured_patterns--;
        captured_pattern_stack_len--;
        pattern = pattern_start;
        return false;

    }
    // Matching the pattern to the different symbols
    if (pattern[0] == '\\' && isdigit(pattern[1])) {
        int pattern_num = (int)(pattern[1] - '0');
        if (pattern_num <= num_captured_patterns) {
            const char* captured_pattern = captured_patterns[pattern_num - 1];
            const char* text_start = text;

            if (match_symbol(text, captured_pattern)) {  // Iterate over the text, and continue matching
                pattern += 2;
                return match_symbol(text, pattern);
            }
        }
    }
    if (pattern[0] == '[') { // Character Groups
        const char* pattern_end = pattern;
        while (pattern_end[0] != ']') pattern_end++;

        char* char_group_pattern = (char*)malloc(pattern_end - pattern + 2);
        strncpy(char_group_pattern, pattern, pattern_end - pattern + 1);
        char_group_pattern[pattern_end - pattern + 2] = '\0';


        if (pattern_end[1] == '+') { // Match one or more.
            return match_once_or_more(text, pattern);
        }
        else if (pattern_end[1] == '?') { // Match zero or more.
            match_character_group(text, char_group_pattern);
            pattern = pattern_end + 2;
            return match_symbol(text, pattern);
        }
        else { // Normal char group matching.
            if (match_character_group(text, char_group_pattern)) {
                pattern = pattern_end + 1;
                return match_symbol(text, pattern);
            }
        }
        return false;
    }
    if (pattern[0] == '\\' && pattern[1] == 'd') { // \d => digit (0-9k)

        if (pattern[2] == '+') {
            return match_once_or_more(text, pattern);
        }
        else if (pattern[2] == '?') {
            char* p = (char*)malloc(3);
            strncpy(p, pattern, 2);
            p[2] = '\0';

            match_zero_or_one(text, p);
            pattern += 3;
            return match_symbol(text, pattern);
        }
        else if (std::isdigit(text[0])) {
            std::cout << "Match: digit. Proceeding..." << std::endl;
            pattern += 2;
            return match_symbol(++text, pattern); // (pattern + 2) because the pattern here is 2 chars(\d).
        }
        return false;
    }
    if (pattern[0] == '\\' && pattern[1] == 'w') { // \w => alphanumeric
        if (pattern[2] == '+') {
            return match_once_or_more(text, pattern);
        }
        else if (pattern[2] == '?') {
            char* p = (char*)malloc(3);
            strncpy(p, pattern, 2);
            p[2] = '\0';

            match_zero_or_one(text, p);
            pattern += 3;
            return match_symbol(text, pattern);
        }
        else if (std::isdigit(text[0]) || std::isalpha(text[0])) {
            std::cout << "Match: alphanumeric. Proceeding..." << std::endl;
            pattern += 2;
            return match_symbol(++text, pattern);
        }
        return false;
    }
    if (pattern[1] == '+') { // Match one or more times
        return match_once_or_more(text, pattern);
    }
    if (pattern[1] == '?') { // Match zero or more times
        char* p = (char*)malloc(2);
        strncpy(p, pattern, 1);
        p[1] = '\0';

        match_zero_or_one(text, p);
        pattern += 2;
        return match_symbol(text, pattern);
    }
    if (*text != '\0' && (*pattern == '.' || *pattern == *text)) { // . => exact matches
        return match_symbol(++text, ++pattern);
    }
    return false;
}

bool match_character_group(const char*& text, char*& pattern) {
    const char* text_start = text;

    bool negative_char_group = false;
    pattern++; // Start of pattern(skip the [).

    if (pattern[0] == '^') {
        negative_char_group = true;
        pattern++;
    }

    const char* p = pattern;
    do {
        if (*text == *p) {  // Found a match
            if (negative_char_group)
                return false; // Finding a match is a fail
            text++;
            return true;
        }
        p++;
    } while (*p != ']');

    if (negative_char_group) { // Failed to find a match. Good if we're using a negative character group.
        text++;
        return true;
    }

    text = text_start; // Reset
    return false;
}


bool match_zero_or_one(const char*& text, char* pattern) {
    const char* p = pattern;
    const char* text_start = text;

    if (!match_symbol(text, p))
        text = text_start;
    return true;
}


bool match_once_or_more(const char*& text, const char*& pattern) {
    const char* pattern_start = pattern;
    const char* text_start = text;

    const char* sub_pattern_end = pattern;
    while (*sub_pattern_end != '+') sub_pattern_end++;

    char* sub_pattern = (char*)malloc(sub_pattern_end - pattern_start);
    strncpy(sub_pattern, pattern, sub_pattern_end - pattern);
    sub_pattern[sub_pattern_end - pattern] = '\0';

    return _match_once_or_more(text, sub_pattern, ++sub_pattern_end);
}


bool _match_once_or_more(const char*& text, char* sub_pattern, const char*& pattern) {
    const char* pattern_start = pattern;
    const char* text_start = text;

    const char* _sub_pattern = sub_pattern;

    if (match_symbol(text, _sub_pattern)) {
        const char* text1 = text;
        const char* text2 = text;

        const char* pattern1 = pattern;
        const char* pattern2 = pattern;

        // try to match more of the sub-pattern || continue trying to match the pattern.
        if (_match_once_or_more(text1, sub_pattern, pattern1) || match_symbol(text2, pattern2)) return true;
    }

    pattern = pattern_start;
    text = text_start;

    return false;
}


bool match_captured_pattern(const char*& text, const char*& pattern) {
    const char* text_start = text;
    const char* pattern_start = pattern;
    const char* pattern_end = pattern + 1;

    int captured_pattern_index = num_captured_patterns;
    num_captured_patterns++;

    pattern_end++;
    if (match_symbol(text, ++pattern)) { // Will terminate at the ')'
        if (match_symbol(text, ++pattern)) {
            while (pattern[1] != ')') {
                pattern++;
            }

            // Replacing the captured pattern with the text it matched with. 
            //  (Ahh, the magic of recursive calls. The subpatterns should already
            //   be  matched and replaced with their text by this point).
            int captured_text_length = text - text_start;
            char* captured_text = (char*)malloc(captured_text_length + 1);
            strncpy(captured_text, text_start, captured_text_length);
            captured_text[captured_text_length] = '\0';

            captured_patterns[captured_pattern_index] = captured_text;
            return match_symbol(text, ++pattern);
        }
    }
    return false;
}

const char** create_char_array_backup(const char** array, int array_size) {
    const char** array_backup = (const char**)malloc(sizeof(array));

    for (int i = 0; i < array_size; i++) {
        // * Not gonna allocate any memory for the strings, as it's assuming they already have their own, 
        // *  and we're going to replace the pointers in the array instead of changing the char* contents.

        array_backup[i] = array[i];
    }

    return array_backup;

}

int* create_int_array_backup(int* array, int array_size) {
    int* array_backup = (int*)malloc(sizeof(array));

    for (int i = 0; i < array_size; i++) {
        // * Not gonna allocate any memory for the strings, as it's assuming they already have their own, 
        // *  and we're going to replace the pointers in the array instead of changing the char* contents.

        array_backup[i] = array[i];
    }

    return array_backup;
}
