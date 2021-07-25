#include <iostream>

#include "FlatJSONPrinter.hpp"

FlatJSONPrinter::FlatJSONPrinter(int indentLevel, bool isOnSingleLine, bool beginOnSameLine)
{
    this->isFirstKeyValuePair = true;
    this->indentLevel = indentLevel;
    this->isOnSingleLine = isOnSingleLine;
    this->isFirstBegin = true;
    this->isBeginOnSameLine = beginOnSameLine;
    this->indentWidth = DEFAULT_INDENT_WIDTH;
}

void FlatJSONPrinter::setIndentWidth(int indentWidth)
{
    this->indentWidth = indentWidth;
}

void printKey(const std::string& key)
{
    std::cout << "\"" << key << "\": ";
}

void FlatJSONPrinter::indent(int indentLevel)
{
    std::cout << std::string(indentLevel * indentWidth, ' ');
}

void FlatJSONPrinter::singleLinePrintComma()
{
    /* Called BEFORE printing a key value pair */

    if (isFirstKeyValuePair)
        return;

    std::cout << ", ";
}

void FlatJSONPrinter::multiLinePrintComma()
{
    /* Called BEFORE printing a key value pair */

    if (isFirstKeyValuePair)
        std::cout << std::endl;
    else
        std::cout << "," << std::endl;

    indent(indentLevel + 1);
}

void FlatJSONPrinter::printCommaAndKey(const std::string& key)
{
    if (isOnSingleLine)
        singleLinePrintComma();
    else
        multiLinePrintComma();

    isFirstKeyValuePair = false;

    printKey(key);
}

void FlatJSONPrinter::printElementComma()
{
    if (isBeginOnSameLine)
    {
        std::cout << ", ";
    }
    else
    {
        std::cout << "," << std::endl;
        indent(indentLevel);
    }
}

void FlatJSONPrinter::begin()
{
    if (!isFirstBegin)
        printElementComma();

    std::cout << "{";

    isFirstBegin = false;
    isFirstKeyValuePair = true;
}

void FlatJSONPrinter::end()
{

    if (isOnSingleLine)
        std::cout << "}";
    else
    {
        std::cout << std::endl;
        indent(indentLevel);
        std::cout << "}";
    }
}

FlatJSONPrinter FlatJSONPrinter::beginNested(const std::string& key, bool nestedOnSingleLine)
{
    printCommaAndKey(key);
    FlatJSONPrinter nestedPrinter(indentLevel + 1, isOnSingleLine | nestedOnSingleLine);
    nestedPrinter.setIndentWidth(indentWidth);
    return nestedPrinter;
}

FlatJSONPrinter FlatJSONPrinter::beginArray(const std::string& key, bool elementOnSingleLine, bool arrayOnSameLine)
{
    printCommaAndKey(key);

    std::cout << "[";
    if (!isOnSingleLine)
    {
        std::cout << std::endl;
        indent(indentLevel + 2);
    }

    FlatJSONPrinter elemPrinter(indentLevel + 2,
                                isOnSingleLine | elementOnSingleLine,
                                isOnSingleLine | isBeginOnSameLine);
    elemPrinter.setIndentWidth(indentWidth);
    return elemPrinter;
}

void FlatJSONPrinter::endArray()
{
    if (isOnSingleLine)
    {
        std::cout << "]";
    }
    else
    {
        std::cout << std::endl;
        indent(indentLevel + 1);
        std::cout << "]";
    }
}

void FlatJSONPrinter::printString(const std::string& key, const std::string& val)
{
    printCommaAndKey(key);
    std::cout << "\"" << val << "\"";
}

void FlatJSONPrinter::printNumber(const std::string& key, float val)
{
    printCommaAndKey(key);
    std::cout << val;
}

void FlatJSONPrinter::printIntArray(const std::string& key, const std::vector<int>& val)
{
    printCommaAndKey(key);

    if (val.size() == 0)
    {
        std::cout << "[]";
        return;
    }

    size_t i = 0;
    std::cout << "[";

    for (i = 0; i < val.size() - 1; i++)
        std::cout << val[i] << ", ";

    std::cout << val[i] << "]";
}

void FlatJSONPrinter::printStringArray(const std::string& key, const std::vector<std::string>& val)
{
    printCommaAndKey(key);

    if (val.size() == 0)
    {
        std::cout << "[]";
        return;
    }

    size_t i = 0;
    std::cout << "[";

    for (i = 0; i < val.size() - 1; i++)
        std::cout << "\"" << val[i] << "\", ";

    std::cout << "\"" << val[i] << "\"]";
}

void FlatJSONPrinter::printLongStringArray(const std::string& key, const std::vector<std::string>& val)
{
    printCommaAndKey(key);

    if (val.size() == 0)
    {
        std::cout << "[]";
        return;
    }

    size_t i = 0;
    std::cout << "[\n";

    for (i = 0; i < val.size() - 1; i++)
    {
        indent(indentLevel + 2);
        std::cout << "\"" << val[i] << "\",\n";
    }

    std::cout << "\"" << val[i] << "\"\n]";
}