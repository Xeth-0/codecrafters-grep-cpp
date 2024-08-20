#include <iostream>
#include <string>
#include <cstring>


bool match_once_or_more(const char*& text, char* pattern);
bool match_zero_or_one(const char*& text, char* pattern);
bool match_symbol(const char*& text, const char*& pattern);
bool match_character_group(const char*& text, char*& pattern);
void capture_patterns(const char* pattern, int pattern_length);
bool match_pattern(const std::string& input_line, const std::string& pattern);
char** capture_pattern_group(const char*& pattern, int pattern_length, int& sub_pattern_length);


char** captured_patterns;   // Storing any captured patterns.
int num_captured_patterns;  // Length of the array.



// Wanted to keep this kinda low level if that makes sense. Didn't use strings and such for the
// actual pattern matching, wanted to keep it as close to C as I could. (Just trynna learn)



bool match_pattern(const std::string& input_line, const std::string& pattern) {
    // Starting point for the matcher. Handles any special patterns like character groups itself. Otherwise calls match_symbol.
    const char* text_ptr = input_line.c_str(); // Pointer to iterate over the input_line. (Wanted to avoid creating a new copy)
    const char* pattern_ptr = pattern.c_str(); // Pointer to iterate over the pattern.

    captured_patterns = (char**)malloc(pattern.length());
    num_captured_patterns = 0;

    // Capture any patterns in ().
    capture_patterns(pattern_ptr, pattern.length());

    // Special Matches
    if (pattern_ptr[0] == '^') { // Start of String Anchor
        const char* pattern = pattern_ptr + 1;
        const char* text = text_ptr;

        return match_symbol(text, pattern); // Avoids the loop below, so only matches at the start.
    }

    // Standard Match
    do {  // Iterate over the text and attempt to match the pattern.
        const char* text = text_ptr;
        const char* pattern = pattern_ptr;

        if (match_symbol(text, pattern)) {
            std::cout << "MATCH SUCCESS!!!!!!!!!!!" << std::endl;
            return true;
        }
    } while (*text_ptr++ != '\0');

    return false;
}

bool match_symbol(const char*& text, const char*& pattern) {
    // Recursive matcher. Match entire pattern over the remaining text. 
    // Need to iterate both the text and the pattern.

    // // Logging lines for debugging.
    // std::cout << std::endl;
    // std::cout << "Matching Symbol...";
    // std::cout << "|text: " << text;
    // std::cout << "|pattern: |" << pattern << std::endl;

    // Base cases
    if (pattern[0] == '\0' || pattern[0] == '|') {
        return true;
    }
    if (pattern[0] == '$' && (pattern[1] == '\0')) { // End of string anchor
        return *text == '\0';
    }
    if (pattern[0] == ')') { // End of the either/or matcher. Means we matched the second pattern. 
        return true;
    }
    if (text[0] == '\0') {
        return false;
    }
    // Matching the pattern to the different symbols
    if (pattern[0] == '\\' && isdigit(pattern[1])) {
        int pattern_num = (int)(pattern[1] - '0');
        if (pattern_num <= num_captured_patterns) {

            std::cout << "   Matching captured pattern(\\num)..." << std::endl;

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
            if (match_once_or_more(text, char_group_pattern)) {
                pattern = pattern_end + 2;
                return match_symbol(text, pattern);
            }
            return false;
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
            std::cout << "failed match" << std::endl;
            return false;
        }
    }
    if (pattern[0] == '(') { // Optional and/or captured matching.
        const char* text_start = text;
        const char* pattern_start = pattern;

        if (match_symbol(text, ++pattern)) {
            while (pattern[0] != ')') pattern++;
            return match_symbol(text, ++pattern);
        }

        // Reset
        text = text_start;
        pattern = pattern_start;

        while (pattern[0] != '|') {
            if (pattern[0] == ')') { // Not an optional pattern, it's a captured pattern
                return false; // It already failed to match. Since there is no alternative, return false
            }
            pattern++; // Iterate to the start of the second pattern since we didn't find a match.
        }
        return match_symbol(text, ++pattern);  // Try to match the second pattern.
    }
    if (pattern[0] == '\\' && pattern[1] == 'd') { // \d => digit (0-9)

        if (pattern[2] == '+') {
            char* p = (char*)malloc(3);
            strncpy(p, pattern, 2);
            p[2] = '\0';

            if (match_once_or_more(text, p)) {
                pattern += 3;
                return match_symbol(text, pattern);
            }
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
            char* p = (char*)malloc(3);
            strncpy(p, pattern, 2);
            p[2] = '\0';

            if (match_once_or_more(text, p)) {
                pattern += 3;
                return match_symbol(text, pattern);
            }
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
        char* p = (char*)malloc(2);
        strncpy(p, pattern, 1);
        p[1] = '\0';

        if (match_once_or_more(text, p)) {
            pattern += 2;
            return match_symbol(text, pattern);
        }
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
            if (negative_char_group) {
                return false; // Finding a match is a fail
            }
            text++;
            return true;

        }
        p++;
    } while (*p != ']');

    // Failed to find a match. Good if we're using a negative character group.
    if (negative_char_group) {
        text++;
        return true;
    }

    text = text_start; // reset
    return false;
}

bool match_once_or_more(const char*& text, char* pattern) {
    // we want the text to iterate over (in the caller) during this, not the pattern

    bool flag = false;
    const char* p = pattern;
    const char* text_start = text;
    if (match_symbol(text, p)) {
        flag = true;
        return match_once_or_more(text, pattern) || flag;
    }

    text = text_start;
    return flag;
}

bool match_zero_or_one(const char*& text, char* pattern) {
    const char* p = pattern;
    const char* text_start = text;

    if (!match_symbol(text, p)) text = text_start;
    return true;
}


void capture_patterns(const char* pattern, int pattern_length) {
    // Need to do this first, this way we can do one iteration to capture all the patterns and worry about matching later.
    // Also avoids the need to recapture the patterns.

    // The wrapper for the recursive call basically.

    while (pattern[0] != '\0') {
        const char* pattern_start = pattern;
        if (pattern[0] == '(') { // Start of the capture pattern signifier.
            int sub_pattern_length = 0;
            char** new_patterns = capture_pattern_group(pattern, pattern_length, sub_pattern_length); // will capture any sub-patterns

            for (int i = 0; i < sub_pattern_length; i++) {
                captured_patterns[num_captured_patterns] = new_patterns[i];
                num_captured_patterns++;
            }
        }
        pattern++;
        pattern_length--;
    }
    return;
}

char** capture_pattern_group(const char*& pattern, int pattern_length, int& sub_pattern_length) {
    char** pattern_group = (char**)malloc(pattern_length);
    const char* pattern_start = pattern;
    bool isOptionalPattern = false;

    while (*pattern != '\0') {
        if (*pattern == '|') {
            isOptionalPattern = true;
        }
        if (*pattern == ')') { // End of the pattern.
            char* captured_pattern = (char*)malloc(pattern - pattern_start + 2);
            strncpy(captured_pattern, pattern_start, pattern - pattern_start + 1);
            captured_pattern[pattern - pattern_start + 1] = '\0';

            pattern_group[0] = captured_pattern;

            sub_pattern_length++;
            return pattern_group;
        }
        if ((pattern - pattern_start != 0) && *pattern == '(') {
            // nested pattern. recursive call here.
            int sub_sub_pattern_length = 0;
            char** sub_patterns = capture_pattern_group(pattern, pattern_length, sub_sub_pattern_length);

            for (int i = 0; i < sub_sub_pattern_length; i++) {
                pattern_group[1 + i + sub_pattern_length] = sub_patterns[i];
            }
            sub_pattern_length += sub_sub_pattern_length;
        }
        pattern++;
    }
    return pattern_group;
}


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
