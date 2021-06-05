#ifndef LEXER_HPP
#define LEXER_HPP

#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "Console/Console.hpp"
#include "Token/Token.hpp"

namespace lexer
{
    class lexical_analyser
    {
        std::vector<Token> token_q;
        std::string file_name;
        size_t front_idx;
        Console& console;

        void feed_line(std::string& line, int lineno);
        void feed(std::ifstream& ifile);

      public:
        lexical_analyser(std::ifstream& ifile, Console& consoleRef);
        bool get_token(Token& symbol);
        Token peek_token();
        Token dpeek_token();
    };
}

#endif