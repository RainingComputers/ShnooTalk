#ifndef LEXER_HPP
#define LEXER_HPP

#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "log.hpp"
#include "token.hpp"

namespace lexer
{
    class lexical_analyser
    {
        std::vector<token::token> token_q;
        std::string file_name;
        size_t front_idx;

        void feed_line(std::string& line, int lineno);
        void feed(const std::string& name, std::ifstream& ifile);

      public:
        lexical_analyser(const std::string& name, std::ifstream& ifile);
        bool get_token(token::token& symbol);
        token::token peek_token();
        token::token dpeek_token();
    };
}

#endif