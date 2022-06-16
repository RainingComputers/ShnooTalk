#ifndef PP_IR_PRINTER
#define PP_IR_PRINTER

#include <fstream>
#include <iostream>
#include <string>

#include "../IntermediateRepresentation/All.hpp"
#include "../Node/Node.hpp"
#include "../Token/Token.hpp"
#include "Strings.hpp"

namespace pp
{
    void printModule(const icode::ModuleDescription& module_desc, bool jsonIR);
    void printModulesMap(const icode::StringModulesMap& modulesMap, bool jsonIR);
    void printModuleIcodeOnly(const icode::ModuleDescription& moduleDescription, bool jsonIR);
    void printModulesMapIcodeOnly(const icode::StringModulesMap& modulesMap, bool jsonIR);
}

#endif