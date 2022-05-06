/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "TextParser.h"
#include "conf/ConfigSectionData.h"

PUBLIC
ConfigSectionData::ConfigSectionData(IN const AString& strKey_) :
        strKey(strKey_),
        strValue(AString::ConstEmpty())
{
}

PUBLIC
ConfigSectionData::ConfigSectionData(IN const AString& strKey_, IN const AString& strValue_) :
        strKey(strKey_),
        strValue(strValue_)
{
}

PUBLIC
ConfigSectionData::~ConfigSectionData() {}

PUBLIC
void ConfigSectionData::AddComment(IN const AString& strComment)
{
    objComment.Add(strComment);
}

PUBLIC
const AString& ConfigSectionData::GetKey() const
{
    return strKey;
}

PUBLIC
const AString& ConfigSectionData::GetValue() const
{
    return strValue;
}

PUBLIC
void ConfigSectionData::SetValue(IN const AString& strValue)
{
    this->strValue = strValue;
}

PUBLIC
AString ConfigSectionData::ToString() const
{
    AString strTmpVal;

    // Append the comments
    strTmpVal = objComment.ToString();

    // key=value
    strTmpVal.Append(strKey);
    strTmpVal.Append(TextParser::CHAR_EQUAL);
    strTmpVal.Append(strValue);

    return strTmpVal;
}
