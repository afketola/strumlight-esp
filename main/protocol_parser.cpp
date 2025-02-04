#include "protocol_parser.h"
#include <cstdlib>

// Parses command string: CMD:Cmaj;FRET:3;DUR:1000
CommandData parseCommand(const std::string& input) {
    CommandData cmd = {"", 0, 0};
    size_t start = 0;

    while (start < input.length()) {
        size_t end = input.find(';', start);
        if (end == std::string::npos) end = input.length();
        std::string token = input.substr(start, end - start);
        size_t sep = token.find(':');
        
        if (sep != std::string::npos) {
            std::string key = token.substr(0, sep);
            std::string value = token.substr(sep + 1);

            if (key == "CMD") cmd.chord = value;
            else if (key == "FRET") cmd.fret = std::atoi(value.c_str());
            else if (key == "DUR") cmd.duration = std::atoi(value.c_str());
        }
        start = end + 1;
    }

    return cmd;
}
