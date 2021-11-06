#ifndef PP_FLAT_JSON_PRINTER
#define PP_FLAT_JSON_PRINTER

#include <map>
#include <string>
#include <vector>

#define DEFAULT_INDENT_WIDTH 4

class FlatJSONPrinter
{
    bool isFirstKeyValuePair;
    bool isOnSingleLine;
    bool isBeginOnSameLine;
    bool isFirstBegin;
    int indentLevel;
    int indentWidth;

    void indent(int indentLevel);
    void singleLinePrintComma();
    void multiLinePrintComma();
    void printCommaAndKey(const std::string& key);
    void printElementComma();

public:
    FlatJSONPrinter(int indentLevel = 0, bool onSingleLine = false, bool beginOnSameLine = false);
    void setIndentWidth(int indentWidth);
    void begin();
    void end();
    FlatJSONPrinter beginNested(const std::string& key, bool nestedOnSingleLine = false);
    FlatJSONPrinter beginArray(const std::string& key,
                               bool elementOnSingleLine = false,
                               bool arrayOnSingleLine = false);
    void endArray();
    void printString(const std::string& key, const std::string& val);
    void printNumber(const std::string& key, double val);
    void printIntArray(const std::string& key, const std::vector<int>& val);
    void printStringArray(const std::string& key, const std::vector<std::string>& val);
};

#endif
