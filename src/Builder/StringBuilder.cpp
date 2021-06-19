#include "TypeDescriptionUtil.hpp"

#include "StringBuilder.hpp"

using namespace icode;

StringBuilder::StringBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder)
  : rootModule(rootModule)
  , opBuilder(opBuilder)
{
}

Operand StringBuilder::creatStringOperand(const Token& str_token, DataType dtype)
{
    std::string name = "_str" + str_token.getLineColString();
    rootModule.stringsData[name] = str_token.toUnescapedString();

    int char_count = str_token.toUnescapedString().length();
    size_t size = char_count * getDataTypeSize(dtype);
    Operand opr = opBuilder.createStringDataOperand(name, size);

    return opr;
}

Unit StringBuilder::createString(const Token& str_token)
{
    TypeDescription stringType = stringTypeFromToken(str_token);

    Operand opr = creatStringOperand(str_token, UI8);

    return Unit(opr, stringType);
}