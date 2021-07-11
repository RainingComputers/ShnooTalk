#include "SetupLLVM.hpp"

#include "ModuleContext.hpp"

using namespace llvm;

ModuleContext::ModuleContext(icode::ModuleDescription& moduleDescription,
                             icode::StringModulesMap& modulesMap,
                             Console& console)
  : moduleDescription(moduleDescription)
  , modulesMap(modulesMap)
  , console(console)
{
    context = std::make_unique<LLVMContext>();
    LLVMModule = std::make_unique<Module>(moduleDescription.name, *context);
    builder = std::make_unique<IRBuilder<>>(*context);
}

void ModuleContext::clear()
{
    symbolNamePointersMap.clear();
    operandValueMap.clear();
    params.clear();
}