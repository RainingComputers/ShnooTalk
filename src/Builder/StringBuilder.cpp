#include "TypeDescriptionUtil.hpp"

#include "StringBuilder.hpp"

using namespace icode;

StringBuilder::StringBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder)
  : rootModule(rootModule)
  , opBuilder(opBuilder)
{
}

TypeDescription StringBuilder::stringTypeFromToken(const Token& token)
{
    TypeDescription stringType = typeDescriptionFromDataType(icode::UI8);

    std::vector<int> dimensions;
    /* +1 for null char */
    dimensions.push_back(token.toUnescapedString().size() + 1);

    stringType = createArrayTypeDescription(stringType, dimensions, icode::STRING_LTRL_DIM);

    stringType.becomeString();

    stringType.moduleName = rootModule.name;

    return stringType;
}

Operand StringBuilder::createStringOperand(const Token& str_token)
{
    std::string name = "_str" + str_token.getLineColString();
    rootModule.stringsData[name] = str_token.toUnescapedString() + '\0';

    int char_count = str_token.toUnescapedString().length() + 1;
    size_t size = char_count * getDataTypeSize(UI8);
    Operand opr = opBuilder.createStringDataOperand(name, size);

    return opr;
}

Unit StringBuilder::createString(const Token& str_token)
{
    TypeDescription stringType = stringTypeFromToken(str_token);

    Operand opr = createStringOperand(str_token);

    return Unit(stringType, opr);
}
