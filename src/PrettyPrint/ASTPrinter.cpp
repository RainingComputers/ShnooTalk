#include <iostream>

#include "Strings.hpp"
#include "JSON/FlatJSONPrinter.hpp"

#include "ASTPrinter.hpp"

namespace pp
{
    void printToken(const Token& token)
    {
        std::cout << "string=\"" << token.toString() << "\", "
                  << "tokenType=" << tokenTypeToString[token.getType()] << ", line=" << token.getLineNo()
                  << ", col=" << token.getColumn();
    }

    void printNodeInfo(const Node& node)
    {
        std::cout << "> nodeType=" << nodeTypeToString[node.type] << ", ";
        printToken(node.tok);
        std::cout << std::endl;
    }

    void printNode(const Node& node, int indent)
    {
        static std::vector<bool> isLastChild;

        printNodeInfo(node);

        isLastChild.push_back(false);

        for (int i = 0; i < (int)node.children.size(); i++)
        {
            for (int j = 0; j <= indent; j++)
            {
                std::cout << "  ";

                if (j == indent)
                    if (i == (int)node.children.size() - 1)
                        std::cout << "└───";
                    else
                        std::cout << "├───";
                else if (isLastChild[j])
                    std::cout << "    ";
                else
                    std::cout << "│   ";
            }

            if (i == (int)node.children.size() - 1)
                isLastChild[indent] = true;

            printNode(node.children[i], indent + 1);
        }

        isLastChild.pop_back();
    }

    std::string escapeString(const Token& token)
    {
        std::string escapedString;

        for (char c : token.toString())
            if (c == '\\')
                escapedString += "\\\\";
            else if (c == '\"')
                escapedString += "\\\"";
            else
                escapedString += c;

        return escapedString;
    }

    void printTokenJson(const Token& token, FlatJSONPrinter& jsonp)
    {
        jsonp.begin();
        jsonp.printString("string", escapeString(token));
        jsonp.printString("tokenType", tokenTypeToString[token.getType()]);
        jsonp.printNumber("line", token.getLineNo());
        jsonp.printNumber("col", token.getColumn());
        jsonp.end();
    }

    void printNodeJson(const Node& node, FlatJSONPrinter& jsonp)
    {
        jsonp.begin();

        FlatJSONPrinter tokenPrinter = jsonp.beginNested("token", true);
        printTokenJson(node.tok, tokenPrinter);

        FlatJSONPrinter childPrinter = jsonp.beginArray("children");

        for (auto child : node.children)
            printNodeJson(child, childPrinter);

        jsonp.endArray();

        jsonp.end();
    }

    void printJSONAST(const Node& ast)
    {
        FlatJSONPrinter jsonp(0, false);
        jsonp.setIndentWidth(2);
        printNodeJson(ast, jsonp);
    }
}
