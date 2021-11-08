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
    void printModuleDescription(const icode::ModuleDescription& module_desc, bool jsonIR);
    void printModuleDescriptionIcodeOnly(const icode::ModuleDescription& moduleDescription);
}

#endif