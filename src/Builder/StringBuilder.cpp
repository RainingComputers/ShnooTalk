#include "TypeDescriptionUtil.hpp"

#include "StringBuilder.hpp"

using namespace icode;

StringBuilder::StringBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder)
  : rootModule(rootModule)
  , opBuilder(opBuilder)
{
}

Operand StringBuilder::createStringOperand(const Token& str_token)
{
    std::string name = "_str" + str_token.getLineColString();
    rootModule.stringsData[name] = str_token.toUnescapedString();

    int char_count = str_token.toUnescapedString().length();
    size_t size = char_count * getDataTypeSize(UI8);
    Operand opr = opBuilder.createStringDataOperand(name, size);

    return opr;
}

Unit StringBuilder::createString(const Token& str_token)
{
    TypeDescription stringType = stringTypeFromToken(str_token);

    Operand opr = createStringOperand(str_token);

    return Unit(opr, stringType);
}