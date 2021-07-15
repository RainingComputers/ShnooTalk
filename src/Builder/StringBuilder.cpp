#include <algorithm>

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

std::string StringBuilder::createStringData(const Token& stringToken)
{
    /* Check if this string has already been defined, if yes return the key for that,
        else create a new key */

    std::string str = stringToken.toUnescapedString() + '\0';

    auto result = std::find_if(rootModule.stringsData.begin(),
                               rootModule.stringsData.end(),
                               [str](const auto& mapItem) { return mapItem.second == str; });

    if (result != rootModule.stringsData.end())
        return result->first;

    std::string key = "_str" + stringToken.getLineColString();
    rootModule.stringsData[key] = str;

    return key;
}

Operand StringBuilder::createStringOperand(const Token& stringToken)
{
    std::string key = createStringData(stringToken);

    int charCount = stringToken.toUnescapedString().length() + 1;
    size_t size = charCount * getDataTypeSize(UI8);
    Operand opr = opBuilder.createStringDataOperand(key, size);

    return opr;
}

Unit StringBuilder::createString(const Token& str_token)
{
    TypeDescription stringType = stringTypeFromToken(str_token);

    Operand opr = createStringOperand(str_token);

    return Unit(stringType, opr);
}
