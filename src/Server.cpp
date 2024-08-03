#include <iostream>
#include <string>

bool match_pattern(const std::string& input_line, const std::string& pattern) {
    if (pattern.length() == 1) { // Single letter(number) search. Not regex as that would require more than one character
        std::cout << pattern << std::endl;
        return input_line.find(pattern) != std::string::npos;
    }
    else if (pattern == "\\d") {
        // find_first_of() matches any of the characters specified in the args. This way
        // we are checking for any numbers within the input line.
        // It returns string::npos if it doesn't find any. It is used to indicate no matches.
        return input_line.find_first_of("0123456789") != std::string::npos;  // 1 if match, 0 if none.
    }
    else if (pattern == "\\w") {
        // return input_line.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos;  // 1 if match, 0 if none.
        for (const char &l : input_line) {
            if (std::isdigit(l) || std::isalpha(l)){
                return true;
            }
        }
        return false;
    }
    else {
        throw std::runtime_error("Unhandled pattern " + pattern);
    }
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

    std::cout << "AAAAAAAAA" << std::endl;
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
