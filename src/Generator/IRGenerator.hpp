#ifndef GENERATORIR_IR_GENERATOR
#define GENERATORIR_IR_GENERATOR

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "../Node/Node.hpp"
#include "GeneratorContext.hpp"

namespace generator
{
    Node generateAST(Console& console);

    void generateIR(Console& console,
                    const std::string& moduleName,
                    icode::StringModulesMap& modulesMap,
                    monomorphizer::StringGenericASTMap& genericsMap);
}

#endif