/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "Feature.h"
#include "SIPPrivate.h"
#include "SIPUtil.h"
#include "SipParameter.h"



PUBLIC
SIPParameter::SIPParameter()
    : strName(AString::ConstNull())
{
}

PUBLIC
SIPParameter::SIPParameter(IN CONST AString &strName_)
    : strName(strName_)
{
}

PUBLIC
SIPParameter::SIPParameter(IN CONST AString &strName_, IN CONST AString &strValue_)
    : strName(strName_)
{
    if (!strValue_.IsNULL())
    {
        objValues.AddElement(strValue_);
    }
}

PUBLIC
SIPParameter::SIPParameter(IN CONST AString &strName_, IN CONST AStringArray &objValues_)
    : strName(strName_)
    , objValues(objValues_)
{
}

PUBLIC
SIPParameter::SIPParameter(IN CONST SIPParameter &objRHS)
    : strName(objRHS.strName)
    , objValues(objRHS.objValues)
{
}

PUBLIC
SIPParameter::~SIPParameter()
{
}

/*

Remarks

*/
PUBLIC
SIPParameter& SIPParameter::operator=(IN CONST SIPParameter &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        strName = objRHS.strName;
        objValues = objRHS.objValues;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
void SIPParameter::AddValue(IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (strValue.IsNULL())
        return;

    // To prevent the duplicated parameter value
    if (objValues.Contains(strValue, IMS_FALSE))
        return;

    objValues.AddElement(strValue);
}

/*

Remarks

*/
PUBLIC
void SIPParameter::AddValues(IN CONST AString &strValues)
{
    //---------------------------------------------------------------------------------------------

    if (strValues.IsNULL())
        return;

    // Check if DQUOTE is present
    if (strValues.StartsWith(TextParser::CHAR_DQUOT))
    {
        IMS_SINT32 nLastDQUOTE = strValues.GetLastIndexOf(TextParser::CHAR_DQUOT);
        AString strTmpVal = strValues.GetSubStr(1, nLastDQUOTE - 1);

        if (strTmpVal.Contains(TextParser::CHAR_COMMA))
        {
            IMSList<AString> objTokens = strTmpVal.Split(TextParser::CHAR_COMMA);

            for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
            {
                const AString &strValue = objTokens.GetAt(i);

                // To prevent the duplicated parameter value
                if (objValues.Contains(strValue, IMS_FALSE))
                    continue;

                objValues.AddElement(strValue);
            }
        }
        else
        {
            // To prevent the duplicated parameter value
            if (objValues.Contains(strValues, IMS_FALSE))
                return;

            objValues.AddElement(strValues);
        }
    }
    else if (strValues.Contains(TextParser::CHAR_COMMA))
    {
        IMSList<AString> objTokens = strValues.Split(TextParser::CHAR_COMMA);

        for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
        {
            const AString &strValue = objTokens.GetAt(i);

            // To prevent the duplicated parameter value
            if (objValues.Contains(strValue, IMS_FALSE))
                continue;

            objValues.AddElement(strValue);
        }
    }
    else
    {
        // To prevent the duplicated parameter value
        if (objValues.Contains(strValues, IMS_FALSE))
            return;

        objValues.AddElement(strValues);
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPParameter::Create(IN CONST AString &strParameter)
{
    AString strValue;
    IMS_SINT32 nCount = strParameter.SplitF(TextParser::CHAR_EQUAL, strName, strValue);

    //---------------------------------------------------------------------------------------------

    if (nCount == 1)
    {
        // Name only parameter
        objValues.RemoveAllElements();
    }
    else if (nCount == 2)
    {
        // Extract the parameter values if present
        if (SetValues(strValue) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
    }
    else
    {
        strName = AString::ConstNull();
        objValues.RemoveAllElements();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPParameter::Equals(IN CONST SIPParameter *pParameter) const
{
    //---------------------------------------------------------------------------------------------

    if (pParameter == IMS_NULL)
        return IMS_FALSE;

    AString strValue = TextParser::DoPercentDecoding(strName);
    AString strOtherValue = TextParser::DoPercentDecoding(pParameter->strName);

    //4 Check boolean type parameter

    if (!strValue.EqualsIgnoreCase(strOtherValue))
    {
        return IMS_FALSE;
    }

    IMS_BOOL bFeatureTag = Feature::IsFeatureTag(strValue);

    for (IMS_SINT32 i = 0; i < objValues.GetCount(); ++i)
    {
        IMS_BOOL bFound = IMS_FALSE;

        strValue = TextParser::DoPercentDecoding(objValues.GetElementAt(i));

        IMS_BOOL bCaseSensitive = strValue.StartsWith(TextParser::CHAR_DQUOT);

        if (bFeatureTag)
        {
            strValue = TextParser::TrimDQUOT(strValue);
        }

        for (IMS_SINT32 j = 0; j < pParameter->objValues.GetCount(); ++j)
        {
            strOtherValue = TextParser::DoPercentDecoding(pParameter->objValues.GetElementAt(j));

            if (bFeatureTag)
            {
                strOtherValue = TextParser::TrimDQUOT(strOtherValue);
            }

            if (bCaseSensitive)
            {
                if (strValue.Equals(strOtherValue))
                {
                    bFound = IMS_TRUE;
                    break;
                }
            }
            else
            {
                if (strValue.EqualsIgnoreCase(strOtherValue))
                {
                    bFound = IMS_TRUE;
                    break;
                }
            }
        }

        if (!bFound)
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
const AString& SIPParameter::GetName() const
{
    //---------------------------------------------------------------------------------------------

    return strName;
}

/*

Remarks

*/
PUBLIC
const AString& SIPParameter::GetValue() const
{
    //---------------------------------------------------------------------------------------------

    if (objValues.IsEmpty())
        return AString::ConstNull();

    return objValues.GetFirstElement();
}

/*

Remarks

*/
PUBLIC
const AStringArray& SIPParameter::GetValues() const
{
    //---------------------------------------------------------------------------------------------

    return objValues;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPParameter::IsNameOnly() const
{
    //---------------------------------------------------------------------------------------------

    return (objValues.GetCount() == 0);
}

/*

Remarks

*/
PUBLIC
void SIPParameter::RemoveValue(IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_SINT32 i = 0; i < objValues.GetCount(); ++i)
    {
        const AString &strExValue = objValues.GetElementAt(i);

        if (strExValue.EqualsIgnoreCase(strValue))
        {
            objValues.RemoveElementAt(i);
            return;
        }
    }
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPParameter::SetValue(IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (strValue.IsEmpty())
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    objValues.RemoveAllElements();

    if (strValue.IsNULL())
    {
        SIPPrivate::SetLastError(SIPError::NO_ERROR);
        return IMS_SUCCESS;
    }

    objValues.AddElement(strValue);

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPParameter::SetValues(IN CONST AString &strValues)
{
    //---------------------------------------------------------------------------------------------

    if (strValues.IsEmpty())
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    objValues.RemoveAllElements();

    if (strValues.IsNULL())
    {
        SIPPrivate::SetLastError(SIPError::NO_ERROR);
        return IMS_SUCCESS;
    }

    // Check if DQUOTE is present
    if (strValues.StartsWith(TextParser::CHAR_DQUOT))
    {
        IMS_SINT32 nLastDQUOTE = strValues.GetLastIndexOf(TextParser::CHAR_DQUOT);
        AString strTmpVal = strValues.GetSubStr(1, nLastDQUOTE - 1);

        if (strTmpVal.Contains(TextParser::CHAR_COMMA))
        {
            objValues = strTmpVal.Split(TextParser::CHAR_COMMA);
        }
        else
        {
            objValues.AddElement(strValues);
        }
    }
    else if (strValues.Contains(TextParser::CHAR_COMMA))
    {
        objValues = strValues.Split(TextParser::CHAR_COMMA);
    }
    else
    {
        objValues.AddElement(strValues);
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
AString SIPParameter::ToString() const
{
    IMS_SINT32 nPValueCount = objValues.GetCount();

    //---------------------------------------------------------------------------------------------

    if (nPValueCount == 0)
    {
        return strName;
    }
    else if (nPValueCount == 1)
    {
        return strName + TextParser::CHAR_EQUAL + objValues.GetFirstElement();
    }
    else
    {
        AString strParameter(strName);

        strParameter += TextParser::CHAR_EQUAL;
        strParameter += TextParser::CHAR_DQUOT;

        if (!objValues.IsEmpty())
        {
            strParameter += objValues.GetElementAt(0);
        }

        for (IMS_SINT32 i = 1; i < objValues.GetCount(); ++i)
        {
            strParameter += TextParser::CHAR_COMMA;
            strParameter += objValues.GetElementAt(i);
        }

        strParameter += TextParser::CHAR_DQUOT;

        return strParameter;
    }
}
