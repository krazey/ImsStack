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
#include "conf/ConfigComment.h"

PUBLIC
ConfigComment::ConfigComment() {}

PUBLIC
ConfigComment::~ConfigComment() {}

PUBLIC
void ConfigComment::Add(IN const AString& strComment)
{
    objComments.AddElement(strComment);
}

PUBLIC
AString ConfigComment::ToString() const
{
    if (objComments.IsEmpty())
    {
        return AString::ConstNull();
    }

    AString strTmpVal;

    for (IMS_SINT32 i = 0; i < objComments.GetCount(); ++i)
    {
        strTmpVal.Append(objComments.GetElementAt(i));
        strTmpVal.Append(TextParser::STR_CRLF);
    }

    return strTmpVal;
}
