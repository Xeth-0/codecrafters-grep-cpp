#include <iostream>
#include <string>

bool match_symbol(const char* text, const char* pattern);
bool match_pattern(const std::string& input_line, const std::string& pattern);
bool match_character_group(const char* text, const char* pattern, bool negative_char_group);


bool match_pattern(const std::string& input_line, const std::string& pattern) {
    // Starting point for the matcher. Handles any special patterns like character groups itself. Otherwise calls match_symbol.
    const char* text_ptr = input_line.c_str(); // Pointer to iterate over the input_line. (Wanted to avoid creating a new copy)
    const char* pattern_ptr = pattern.c_str(); // Pointer to iterate over the pattern.

    // Special matches
    // Character Groups
    if (pattern_ptr[0] == '[') {
        if (pattern_ptr[1] != '^' && match_character_group(text_ptr, pattern_ptr + 1, false)) // positive character group
            return 1;
        if (pattern_ptr[1] == '^' && match_character_group(text_ptr, pattern_ptr + 1, true))  // negative character group
            return 1;

        return 0;   // Failed to match.
    }
    if (pattern_ptr[0] == '^') { // Start of String Anchor
        return match_symbol(text_ptr, pattern_ptr + 1); // Avoids the loop below, so only matches at the start.
    }

    // Iterate over the text and attempt to match the pattern.
    do {
        if (match_symbol(text_ptr, pattern_ptr)) return 1;
    } while (*text_ptr++ != '\0');

    return 0;
}


bool match_symbol(const char* text, const char* pattern) {
    // Recursive matcher. Match entire pattern over the remaining text. 
    // Need to iterate both the text and the pattern.

    // Logging lines for debugging.
    std::cout << "Matching Symbol..." << std::endl;
    std::cout << "text: " << text << std::endl;
    std::cout << "pattern: " << pattern << std::endl;
    std::cout << std::endl;

    // Base case
    if (pattern[0] == '\0' || pattern[0] == '|') {
        std::cout << "End of pattern." << std::endl;
        std::cout << (pattern[0] == '|') ? ("    (Optional path)\n") : "";   // End of the optional path
        return 1;
    }
    // Matching the pattern to the different symbols
    if (pattern[0] == '(') {
        if (match_symbol(text, pattern + 1)) {
            while (pattern[0] != ')') pattern++; // Iterate to the end of the either/or pattern since we already found a match.
            return match_symbol(text, pattern + 1);  // Continue.
        }
        while (pattern[0] != '|') pattern++; // Iterate to the end of the second pattern since we didn't find a match.
        return match_symbol(text, pattern + 1);  // Try to match the second pattern.
    }
    if (pattern[0] == ')') { // End of the either/or matcher. Means we matched the second pattern. 
        return match_symbol(text, pattern + 1);  // Continue.
    }
    if (pattern[0] == '\\' && pattern[1] == 'd') { // \d => digit (0-9)
        if (std::isdigit(text[0])) {
            std::cout << "Match: digit. Proceeding..." << std::endl;
            return match_symbol(text + 1, pattern + 2); // (pattern + 2) because the pattern here is 2 chars(\d).
        }
    }
    if (pattern[0] == '\\' && pattern[1] == 'w') { // \w => alphanumeric
        if (std::isdigit(text[0]) || std::isalpha(text[0])) {
            std::cout << "Match: alphanumeric. Proceeding..." << std::endl;
            return match_symbol(text + 1, pattern + 2);
        }
    }
    if (pattern[1] == '+') { // Match one or more times
        std::cout << "Matching +..." << std::endl;
        std::cout << "Pattern: " << pattern[0] << pattern[1] << std::endl;
        std::cout << *text << std::endl;
        std::cout << std::endl;

        if (pattern[0] == text[0]) { // First match
            while (pattern[0] == text[0]) { // Match the rest(if any) as well.
                text++;
            }
            return match_symbol(text, pattern + 2);
        }
    }
    if (pattern[1] == '?') { // Match zero or more times
        std::cout << "Matching +..." << std::endl;
        std::cout << "Pattern: " << pattern[0] << pattern[1] << std::endl;
        std::cout << *text << std::endl;
        std::cout << std::endl;

        while (pattern[0] == text[0]) text++; // Match all (if any).
        return match_symbol(text, pattern + 2);
    }
    if (*text != '\0' && (*pattern == '.' || *pattern == *text)) { // . => exact matches
        std::cout << "Match: exact match. Proceeding..." << std::endl;
        return match_symbol(text + 1, pattern + 1);
    }
    if (pattern[0] == '$' && (pattern[1] == '\0')) { // End of string anchor
        return *text == '\0';
    }
    return 0;
}

bool match_character_group(const char* text, const char* pattern, bool negative_char_group) {
    while (*text != '\0') {
        const char* p = pattern;
        do {
            std::cout << "text pattern : result" << std::endl << *text << *p << " : " << (*text == *p) << std::endl << std::endl;
            if (*text == *p) {  // Found a match
                if (negative_char_group) return 0; // Finding a match is a fail
                else return 1;
            }
            p++;
        } while (*p != ']');
        text++;
    }
    if (negative_char_group) return 1;
    else return 0;
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
            std::cout << "Pattern matched" << std::endl;
            return 0;
        }
        else {
            std::cout << "Pattern not matched" << std::endl;
            return 1;
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
