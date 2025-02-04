#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <string>

struct CommandData {
    std::string chord;
    int fret;
    int duration;
};

CommandData parseCommand(const std::string& input);

#endif // PROTOCOL_PARSER_H
