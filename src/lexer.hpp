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
        std::vector<token::Token> token_q;
        std::string file_name;
        size_t front_idx;
        Console& console;

        void feed_line(std::string& line, int lineno);
        void feed(const std::string& name, std::ifstream& ifile);

      public:
        lexical_analyser(const std::string& name, std::ifstream& ifile, Console& consoleRef);
        bool get_token(token::Token& symbol);
        token::Token peek_token();
        token::Token dpeek_token();
    };
}

#endif