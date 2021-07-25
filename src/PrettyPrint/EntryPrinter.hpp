#ifndef PP_ENTRY_PRINTER
#define PP_ENTRY_PRINTER

#include "../IntermediateRepresentation/Entry.hpp"

std::vector<std::string> icodeTableToStringArray(const std::vector<icode::Entry>& icodeTable);
std::string prettyPrintIcodeTable(const std::vector<icode::Entry>& icodeTable, int indentLevel);

#endif