#include "parser.hpp"

/*
    Hand-written recursive descent parser for uHLL.
    refer grammer.txt for grammer specification of the language.
*/

namespace parser
{
    rd_parser::rd_parser(lexer::lexical_analyser& lexer, const std::string& name, Console& consoleRef)
      : lex(lexer)
      , console(consoleRef)
      , ast(node::PROGRAM)
    {
        current_node = &ast;
        file_name = name;

        /* Start recursive descent parsing */
        /* Start with current node as PROGRAM */
        next();
        program();
    }

    void rd_parser::next()
    {
        /* Get next token from lexer's token queue */
        lex.get_token(symbol);
    }

    bool rd_parser::peek(token::tokenType type)
    {
        /* Peek into token queue without popping it */
        return type == lex.peek_token().getType();
    }

    bool rd_parser::dpeek(token::tokenType type)
    {
        /* Peek into token queue without popping it */
        return type == lex.dpeek_token().getType();
    }

    bool rd_parser::accept(token::tokenType type)
    {
        /* Check if symbol is equal to type passed as argument */
        return type == symbol.getType();
    }

    int rd_parser::get_operator_prec()
    {
        return symbol.getPrecedence();
    }

    void rd_parser::expect(token::tokenType type)
    {
        /* If symbol is not equal to type passed as argument,
            throw exception*/
        if (!accept(type))
            console.parseError(type, symbol);
    }

    void rd_parser::expect(token::tokenType* types, int ntypes)
    {
        /* If symbol is not equal to atleast one of the types
            passed as argument, throw exception */
        bool accepted = false;

        for (int i = 0; i < ntypes; i++)
            if (accept(types[i]))
                accepted = true;

        if (!accepted)
            console.parserErrorMultiple(types, ntypes, symbol);
    }

    void rd_parser::add_node(node::NodeType type, bool traverse, bool nexttoken)
    {
        /* Add child node to current node, if traverse iw true
            make current node the child node */
        /* Also fetch next symbol, if nexttoken is true */
        current_node->children.push_back(node::Node(type, symbol));

        if (traverse)
            current_node = &current_node->children.back();
        if (nexttoken)
            next();
    }

    void rd_parser::insert_node(node::NodeType type)
    {
        /* Adopts last child of current node
            and becomes the last child of current node */

        node::Node new_node(type, current_node->children.back().tok);

        new_node.children.push_back(current_node->children.back());

        current_node->children.pop_back();

        current_node->children.push_back(new_node);

        current_node = &current_node->children.back();
    }

    void rd_parser::insert_node_beg(node::NodeType type)
    {
        node::Node new_node(type, current_node->children.back().tok);

        std::swap(current_node->children, new_node.children);

        current_node->children.push_back(new_node);
    }

    void rd_parser::duplicate_node()
    {
        /* Takes all children of current node, moves
            it to a new duplicate of the current node. The duplicate
            node becomes the child of the current node */

        node::Node new_node(current_node->type, symbol);

        new_node.children = current_node->children;

        current_node->children.clear();

        current_node->children.push_back(new_node);

        next();
    }

    void rd_parser::push_node()
    {
        /* Push current node onto stack */
        node_stack.push_back(current_node);
    }

    void rd_parser::pop_node()
    {
        /* Pop node from stack, assign to current node */
        current_node = node_stack.back();
        node_stack.pop_back();
    }

    void rd_parser::program()
    {
        token::tokenType expected[] = { token::STRUCT, token::FUNCTION, token::ENUM,
                                        token::DEF,    token::VAR,      token::END_OF_FILE };

        while (accept(token::USE))
            use();

        while (accept(token::FROM))
            from();

        while (true)
        {
            expect(expected, 6);
            if (accept(token::STRUCT))
                structdef();
            else if (accept(token::FUNCTION))
                function();
            else if (accept(token::ENUM))
                enumdef();
            else if (accept(token::VAR))
                struct_varlist();
            else if (accept(token::DEF))
                def();
            else
                break;
        }

        expect(token::END_OF_FILE);
    }

    void rd_parser::use()
    {
        push_node();

        add_node(node::USE, true);

        expect(token::IDENTIFIER);
        add_node(node::IDENTIFIER);

        while (accept(token::COMMA))
        {
            next();
            expect(token::IDENTIFIER);
            add_node(node::IDENTIFIER);
        }

        pop_node();
    }

    void rd_parser::from()
    {
        push_node();

        add_node(node::FROM, true);

        expect(token::IDENTIFIER);
        add_node(node::IDENTIFIER);

        expect(token::USE);
        use();

        pop_node();
    }

    void rd_parser::def()
    {
        push_node();

        add_node(node::DEF, true);

        expect(token::IDENTIFIER);
        add_node(node::IDENTIFIER);

        token::tokenType expected[] = { token::INT_LITERAL, token::FLOAT_LITERAL, token::CHAR_LITERAL };

        expect(expected, 3);

        add_node(node::LITERAL);

        pop_node();
    }

    void rd_parser::termvar(bool expr_subscript)
    {
        add_node(node::IDENTIFIER);

        while (accept(token::OPEN_SQAURE))
        {
            push_node();

            add_node(node::SUBSCRIPT, true);

            if (!expr_subscript)
            {
                expect(token::INT_LITERAL);
                add_node(node::LITERAL);
            }
            else
            {
                expression();
            }

            expect(token::CLOSE_SQUARE);
            next();

            pop_node();
        }
    }

    void rd_parser::exprvar()
    {
        termvar(true);

        push_node();

        while (accept(token::DOT) && !dpeek(token::LPAREN))
        {
            add_node(node::STRUCT_VAR);
            expect(token::IDENTIFIER);
            termvar(true);
        }

        pop_node();
    }

    void rd_parser::varname()
    {
        add_node(node::IDENTIFIER);

        expect(token::COLON);
        next();

        while (peek(token::DOUBLE_COLON))
        {
            add_node(node::MODULE);
            next();
        }

        expect(token::IDENTIFIER);
        termvar();
    }

    void rd_parser::initializerlist()
    {
        push_node();

        add_node(node::INITLIST, true);

        token::tokenType expected[] = { token::INT_LITERAL,   token::CHAR_LITERAL, token::HEX_LITERAL,
                                        token::FLOAT_LITERAL, token::STR_LITERAL,  token::BIN_LITERAL,
                                        token::OPEN_SQAURE,   token::IDENTIFIER,   token::MINUS,
                                        token::STR_LITERAL };

        expect(expected, 10);

        if (accept(token::OPEN_SQAURE))
            initializerlist();
        else if (accept(token::STR_LITERAL))
            add_node(node::STR_LITERAL);
        else
            expression();

        while (accept(token::COMMA))
        {
            next();

            expect(expected, 10);

            if (accept(token::OPEN_SQAURE))
                initializerlist();
            else if (accept(token::STR_LITERAL))
                add_node(node::STR_LITERAL);
            else
                expression();
        }

        expect(token::CLOSE_SQUARE);
        next();

        pop_node();
    }

    void rd_parser::varassign()
    {
        expect(token::IDENTIFIER);
        varname();

        if (accept(token::EQUAL))
        {
            next();

            token::tokenType expected[] = {
                token::INT_LITERAL, token::CHAR_LITERAL, token::HEX_LITERAL, token::FLOAT_LITERAL,
                token::STR_LITERAL, token::BIN_LITERAL,  token::IDENTIFIER,  token::OPEN_SQAURE,
                token::MINUS,       token::PLUS,         token::LPAREN,      token::SIZEOF,
            };

            expect(expected, 12);

            if (accept(token::OPEN_SQAURE))
                initializerlist();
            else if (accept(token::STR_LITERAL))
                add_node(node::STR_LITERAL);
            else
                expression();
        }
    }

    void rd_parser::varlist()
    {
        push_node();

        add_node(node::VAR, true);

        expect(token::IDENTIFIER);
        varassign();

        pop_node();

        while (accept(token::COMMA))
        {
            push_node();

            add_node(node::VAR, true);

            if (accept(token::VAR))
                next();

            expect(token::IDENTIFIER);
            varassign();

            pop_node();
        }
    }

    void rd_parser::constlist()
    {
        push_node();

        add_node(node::CONST, true);

        expect(token::IDENTIFIER);
        varassign();

        pop_node();

        while (accept(token::COMMA))
        {
            push_node();

            add_node(node::CONST, true);

            if (accept(token::CONST))
                next();

            expect(token::IDENTIFIER);
            varassign();

            pop_node();
        }
    }

    void rd_parser::struct_varlist()
    {
        push_node();

        add_node(node::VAR, true);

        expect(token::IDENTIFIER);
        varname();

        pop_node();

        while (accept(token::COMMA))
        {
            push_node();

            add_node(node::VAR, true);

            if (accept(token::VAR))
                next();

            expect(token::IDENTIFIER);
            varname();

            pop_node();
        }
    }

    void rd_parser::paramlist()
    {
        push_node();

        if (accept(token::MUTABLE))
            add_node(node::MUT_PARAM, true, true);
        else
            add_node(node::PARAM, true, false);

        expect(token::IDENTIFIER);
        varname();

        pop_node();

        while (accept(token::COMMA))
        {
            next();

            push_node();

            if (accept(token::MUTABLE))
                add_node(node::MUT_PARAM, true, true);
            else
                add_node(node::PARAM, true, false);

            expect(token::IDENTIFIER);
            varname();

            pop_node();
        }
    }

    void rd_parser::term_literal()
    {
        token::tokenType literals[] = { token::INT_LITERAL,
                                        token::CHAR_LITERAL,
                                        token::FLOAT_LITERAL,
                                        token::HEX_LITERAL,
                                        token::BIN_LITERAL };

        if (accept(token::MINUS) | accept(token::PLUS))
        {
            add_node(node::UNARY_OPR);
            expect(literals, 5);
        }
        else
            expect(literals, 5);

        add_node(node::LITERAL);
    }

    void rd_parser::structdef()
    {
        push_node();

        add_node(node::STRUCT, true);

        expect(token::IDENTIFIER);
        add_node(node::IDENTIFIER, true);

        expect(token::OPEN_BRACE);
        next();

        expect(token::VAR);
        struct_varlist();

        while (accept(token::VAR))
        {
            expect(token::VAR);
            struct_varlist();
        }

        expect(token::CLOSE_BRACE);
        next();

        pop_node();
    }

    void rd_parser::enumdef()
    {
        push_node();

        add_node(node::ENUM, true);

        expect(token::OPEN_SQAURE);
        next();

        expect(token::IDENTIFIER);
        add_node(node::IDENTIFIER);

        while (accept(token::COMMA))
        {
            next();

            expect(token::IDENTIFIER);
            add_node(node::IDENTIFIER);
        }

        expect(token::CLOSE_SQUARE);
        next();

        pop_node();
    }

    void rd_parser::struct_funccall()
    {
        while (accept(token::DOT))
        {
            next();

            if (peek(token::LPAREN) && accept(token::IDENTIFIER))
            {
                duplicate_node();

                push_node();

                insert_node(node::STRUCT_FUNCCALL);
                passparam();

                pop_node();
            }
            else
                break;
        }
    }

    void rd_parser::term()
    {
        push_node();

        add_node(node::TERM, true, false);

        token::tokenType expected[] = { token::IDENTIFIER,    token::NOT,          token::LPAREN,
                                        token::INT_LITERAL,   token::CHAR_LITERAL, token::HEX_LITERAL,
                                        token::FLOAT_LITERAL, token::BIN_LITERAL,  token::MINUS,
                                        token::CONDN_NOT,     token::SIZEOF };

        expect(expected, 11);

        if (accept(token::SIZEOF))
        {
            push_node();

            add_node(node::SIZEOF, true);

            expect(token::LPAREN);
            next();

            while (peek(token::DOUBLE_COLON))
            {
                expect(token::IDENTIFIER);
                add_node(node::MODULE);
                next();
            }

            expect(token::IDENTIFIER);
            add_node(node::IDENTIFIER);

            expect(token::RPAREN);
            next();

            pop_node();
        }
        else if (accept(token::IDENTIFIER))
        {
            if (peek(token::LPAREN))
            {
                funccall();
            }
            else if (peek(token::CAST))
            {
                add_node(node::CAST, true);

                next();

                term();
            }
            else if (peek(token::DOUBLE_COLON))
            {
                while (peek(token::DOUBLE_COLON))
                {
                    add_node(node::MODULE);
                    next();
                }

                term();
            }
            else
            {
                exprvar();
                struct_funccall();
            }
        }
        else if (accept(token::LPAREN))
        {
            next();

            expression();

            expect(token::RPAREN);
            next();
        }
        else if (accept(token::NOT) || accept(token::MINUS) || accept(token::CONDN_NOT))
        {
            add_node(node::UNARY_OPR, true);

            term();
        }
        else
        {
            term_literal();
        }

        pop_node();
    }

    void rd_parser::expression(int min_precedence)
    {
        push_node();

        add_node(node::EXPRESSION, true, false);

        term();

        while (get_operator_prec() >= min_precedence)
        {
            int prec = get_operator_prec();

            insert_node_beg(node::EXPRESSION);

            add_node(node::OPERATOR);

            expression(prec + 1);
        }

        pop_node();
    }

    void rd_parser::assignment()
    {
        push_node();

        push_node();

        add_node(node::TERM, true, false);

        expect(token::IDENTIFIER);
        exprvar();

        if (accept(token::DOT) && dpeek(token::LPAREN) && peek(token::IDENTIFIER))
        {
            struct_funccall();
            pop_node();
        }
        else
        {
            pop_node();

            if (accept(token::EQUAL) && peek(token::STR_LITERAL))
            {
                insert_node(node::ASSIGNMENT_STR);

                next();
                add_node(node::STR_LITERAL);
            }
            else if (accept(token::EQUAL) && peek(token::OPEN_SQAURE))
            {
                insert_node(node::ASSIGNMENT_INITLIST);

                next();
                initializerlist();
            }
            else
            {
                insert_node(node::ASSIGNMENT);

                token::tokenType expected[] = { token::PLUS_EQUAL,     token::MINUS_EQUAL, token::DIVIDE_EQUAL,
                                                token::MULTIPLY_EQUAL, token::OR_EQUAL,    token::AND_EQUAL,
                                                token::XOR_EQUAL,      token::EQUAL };

                expect(expected, 8);
                add_node(node::ASSIGN_OPERATOR);

                expression();
            }
        }

        pop_node();
    }

    void rd_parser::print()
    {
        push_node();

        if (accept(token::PRINTLN))
            add_node(node::PRINTLN, true);
        else
            add_node(node::PRINT, true);

        expect(token::LPAREN);
        next();

        token::tokenType expected[] = { token::IDENTIFIER,   token::STR_LITERAL,   token::INT_LITERAL,
                                        token::CHAR_LITERAL, token::FLOAT_LITERAL, token::HEX_LITERAL,
                                        token::BIN_LITERAL,  token::LPAREN,        token::SIZEOF,
                                        token::NOT,          token::MINUS };
        expect(expected, 11);

        if (accept(token::STR_LITERAL))
            add_node(node::STR_LITERAL);
        else
            expression();

        while (accept(token::COMMA))
        {
            next();

            expect(expected, 11);

            if (accept(token::STR_LITERAL))
                add_node(node::STR_LITERAL);
            else
                expression();
        }

        expect(token::RPAREN);
        next();

        pop_node();
    }

    void rd_parser::statement()
    {
        if (accept(token::IDENTIFIER))
        {
            if (peek(token::LPAREN))
            {
                funccall();
            }
            else if (peek(token::DOUBLE_COLON))
            {
                push_node();

                add_node(node::MODULE, true);
                next();
                funccall();

                pop_node();
            }
            else
            {
                assignment();
            }
        }
        else if (accept(token::VAR))
        {
            varlist();
        }
        else if (accept(token::CONST))
        {
            constlist();
        }
        else if (accept(token::IF))
        {
            push_node();

            add_node(node::IF, true, false);

            push_node();

            add_node(node::IF, true);

            expression();

            block();

            pop_node();

            while (accept(token::ELSEIF))
            {
                push_node();

                add_node(node::ELSEIF, true);

                expression();

                block();

                pop_node();
            }

            if (accept(token::ELSE))
            {
                push_node();

                add_node(node::ELSE, true);

                block();

                pop_node();
            }

            pop_node();
        }
        else if (accept(token::WHILE))
        {
            push_node();

            add_node(node::WHILE, true);

            expression();

            block();

            pop_node();
        }
        else if (accept(token::FOR))
        {
            push_node();

            add_node(node::FOR, true);

            if (accept(token::LPAREN))
                next();

            if (accept(token::VAR))
                varlist();
            else
                assignment();
            expect(token::SEMICOLON);
            next();

            expression();
            expect(token::SEMICOLON);
            next();

            expect(token::IDENTIFIER);
            assignment();

            if (accept(token::RPAREN))
                next();

            block();

            pop_node();
        }
        else if (accept(token::BREAK))
        {
            add_node(node::BREAK);
        }
        else if (accept(token::CONTINUE))
        {
            add_node(node::CONTINUE);
        }
        else if (accept(token::RETURN))
        {
            push_node();

            add_node(node::RETURN, true);

            if (accept(token::VOID))
                next();
            else
                expression();

            pop_node();
        }
        else if (accept(token::PRINTLN) || accept(token::PRINT))
        {
            print();
        }
        else if (accept(token::INPUT))
        {
            push_node();

            add_node(node::INPUT, true);

            expect(token::LPAREN);
            next();

            expect(token::IDENTIFIER);
            term();

            expect(token::RPAREN);
            next();

            pop_node();
        }
        else if (accept(token::EXIT))
        {
            add_node(node::EXIT);
        }
    }

    void rd_parser::block()
    {
        push_node();

        add_node(node::BLOCK, true, false);

        token::tokenType expected[] = { token::CLOSE_BRACE, token::VAR,    token::CONST, token::IF,
                                        token::WHILE,       token::FOR,    token::BREAK, token::CONTINUE,
                                        token::IDENTIFIER,  token::RETURN, token::PRINT, token::PRINTLN,
                                        token::INPUT,       token::EXIT };

        if (accept(token::OPEN_BRACE))
        {
            next();

            while (!accept(token::CLOSE_BRACE))
            {
                expect(expected, 14);
                statement();
            }

            expect(token::CLOSE_BRACE);
            next();
        }
        else
            statement();

        pop_node();
    }

    void rd_parser::function()
    {
        push_node();

        add_node(node::FUNCTION, true);

        expect(token::IDENTIFIER);
        add_node(node::IDENTIFIER);

        expect(token::LPAREN);
        next();

        if (accept(token::IDENTIFIER) || accept(token::MUTABLE))
            paramlist();

        expect(token::RPAREN);
        next();

        if (accept(token::RIGHT_ARROW))
        {
            push_node();

            next();

            expect(token::IDENTIFIER);
            termvar();

            pop_node();
        }

        block();

        pop_node();
    }

    void rd_parser::passparam()
    {
        next();

        if (!accept(token::RPAREN))
        {
            if (accept(token::STR_LITERAL))
                add_node(node::STR_LITERAL);
            else
                expression();

            while (accept(token::COMMA))
            {
                next();

                if (accept(token::STR_LITERAL))
                    add_node(node::STR_LITERAL);
                else
                    expression();
            }
        }

        expect(token::RPAREN);
        next();
    }

    void rd_parser::funccall()
    {
        push_node();

        add_node(node::FUNCCALL, true, true);

        expect(token::LPAREN);
        passparam();

        pop_node();
    }

} // namespace parser
