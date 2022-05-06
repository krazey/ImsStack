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
#include "conf/ConfigSection.h"

PUBLIC
ConfigSection::ConfigSection() :
        strSectionName(AString::ConstNull())
{
}

PUBLIC
ConfigSection::~ConfigSection()
{
    if (!objSectionData.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSectionData.GetSize(); ++i)
        {
            ConfigSectionData* pData = objSectionData.GetAt(i);

            if (pData != IMS_NULL)
            {
                delete pData;
            }
        }
    }
}

PUBLIC
void ConfigSection::AddComment(IN const AString& strComment)
{
    objComment.Add(strComment);
}

PUBLIC
const AString& ConfigSection::GetName() const
{
    return strSectionName;
}

PUBLIC
void ConfigSection::GetKeys(OUT AStringArray& objKeys) const
{
    for (IMS_UINT32 i = 0; i < objSectionData.GetSize(); ++i)
    {
        const ConfigSectionData* pSectionData = objSectionData.GetAt(i);

        objKeys.AddElement(pSectionData->GetKey());
    }
}

PUBLIC
const AString& ConfigSection::GetValue(IN const IMS_CHAR* pszKey) const
{
    if (objSectionData.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < objSectionData.GetSize(); ++i)
    {
        const ConfigSectionData* pData = objSectionData.GetAt(i);

        if (pData->GetKey().EqualsIgnoreCase(pszKey))
        {
            return pData->GetValue();
        }
    }

    return AString::ConstNull();
}

PUBLIC
IMS_BOOL ConfigSection::SetValue(IN const IMS_CHAR* pszKey, IN const AString& strValue)
{
    if (strValue.IsNULL())
    {
        return IMS_FALSE;
    }

    if (objSectionData.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objSectionData.GetSize(); ++i)
    {
        ConfigSectionData* pData = objSectionData.GetAt(i);

        if (pData->GetKey().EqualsIgnoreCase(pszKey))
        {
            pData->SetValue(strValue);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
AString ConfigSection::ToString() const
{
    AString strTmpVal;

    // Append the comment
    strTmpVal = objComment.ToString();

    // Section: [SectionName]
    strTmpVal.Append(TextParser::CHAR_LSBRACKET);
    strTmpVal.Append(strSectionName);
    strTmpVal.Append(TextParser::CHAR_RSBRACKET);
    strTmpVal.Append(TextParser::STR_CRLF);

    // Section parameters
    for (IMS_UINT32 i = 0; i < objSectionData.GetSize(); ++i)
    {
        const ConfigSectionData* pData = objSectionData.GetAt(i);

        strTmpVal.Append(pData->ToString());
        strTmpVal.Append(TextParser::STR_CRLF);
    }

    return strTmpVal;
}

PRIVATE
IMS_BOOL ConfigSection::AddSectionData(IN const AString& strKeyValue)
{
    IMS_SINT32 nStartIndex = strKeyValue.GetIndexOf(TextParser::CHAR_EQUAL);

    if (nStartIndex == AString::NPOS)
    {
        return IMS_FALSE;
    }

    AString strKey = strKeyValue.GetSubStr(0, nStartIndex).Trim();
    AString strValue = strKeyValue.GetSubStr(nStartIndex + 1).Trim();

    ConfigSectionData* pData = new ConfigSectionData(strKey, strValue);

    if (pData == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objSectionData.Append(pData))
    {
        delete pData;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
ConfigSectionData* ConfigSection::GetLastElement() const
{
    if (objSectionData.IsEmpty())
    {
        return IMS_NULL;
    }

    return objSectionData.GetAt(objSectionData.GetSize() - 1);
}

PRIVATE
void ConfigSection::SetName(IN const AString& strSectName)
{
    this->strSectionName = strSectName;
}
