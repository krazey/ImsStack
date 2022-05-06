/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100415  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "TextParser.h"
#include "Replaces.h"

PUBLIC
Replaces::Replaces() :
        strCallId(AString::ConstNull()),
        strFromTag(AString::ConstNull()),
        strToTag(AString::ConstNull()),
        bIsEarlyOnly(IMS_FALSE),
        pDialog(IMS_NULL)
{
}

PUBLIC
Replaces::Replaces(IN CONST AString& strCallId_, IN CONST AString& strLocalTag_,
        IN CONST AString& strRemoteTag_, IN IMS_BOOL bIsEarlyOnly_ /* = IMS_FALSE */) :
        strCallId(strCallId_),
        strFromTag(strLocalTag_),
        strToTag(strRemoteTag_),
        bIsEarlyOnly(bIsEarlyOnly_)
{
    pDialog = new Dialog(strCallId, strFromTag, strToTag);
}

PUBLIC
Replaces::Replaces(IN CONST Replaces& objRHS) :
        strCallId(objRHS.strCallId),
        strFromTag(objRHS.strFromTag),
        strToTag(objRHS.strToTag),
        bIsEarlyOnly(objRHS.bIsEarlyOnly),
        pDialog(IMS_NULL)
{
    if (objRHS.pDialog != IMS_NULL)
    {
        if (strToTag.Equals(objRHS.pDialog->GetLocalTag()))
            pDialog = new Dialog(strCallId, strToTag, strFromTag);
        else
            pDialog = new Dialog(strCallId, strFromTag, strToTag);
    }
}

PUBLIC
Replaces::~Replaces()
{
    if (pDialog != IMS_NULL)
    {
        delete pDialog;
        pDialog = IMS_NULL;
    }
}

PUBLIC
Replaces& Replaces::operator=(IN CONST Replaces& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        strCallId = objRHS.strCallId;
        strFromTag = objRHS.strFromTag;
        strToTag = objRHS.strToTag;
        bIsEarlyOnly = objRHS.bIsEarlyOnly;

        if (objRHS.pDialog != IMS_NULL)
        {
            if (pDialog != IMS_NULL)
            {
                delete pDialog;
                pDialog = IMS_NULL;
            }

            if (strToTag.Equals(objRHS.pDialog->GetLocalTag()))
                pDialog = new Dialog(strCallId, strToTag, strFromTag);
            else
                pDialog = new Dialog(strCallId, strFromTag, strToTag);
        }
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Replaces::Create(IN CONST AString& strReplacesHeader, IN IMS_BOOL bUAS /* = IMS_TRUE */)
{
    AString strReplaces = TextParser::DoPercentDecoding(strReplacesHeader);
    IMSList<AString> objTokens = strReplaces.Split(TextParser::CHAR_SEMICOLON);

    //---------------------------------------------------------------------------------------------

    if (objTokens.GetSize() < 3)
    {
        return IMS_FALSE;
    }

    // call-id
    strCallId = objTokens.GetAt(0);

    // to-tag & from-tag
    for (IMS_UINT32 i = 1; i < objTokens.GetSize(); ++i)
    {
        const AString& strTemp = objTokens.GetAt(i);
        IMS_SINT32 nIndex = strTemp.GetIndexOf(TextParser::CHAR_EQUAL);
        AString strName = strTemp.GetSubStr(0, nIndex);

        if (strName.EqualsIgnoreCase("to-tag"))
        {
            strToTag = strTemp.GetSubStr(nIndex + 1);
        }
        else if (strName.EqualsIgnoreCase("from-tag"))
        {
            strFromTag = strTemp.GetSubStr(nIndex + 1);
        }
        else if (strName.EqualsIgnoreCase("early-only"))
        {
            bIsEarlyOnly = IMS_TRUE;
        }
    }

    if (strCallId.IsNULL() || strToTag.IsNULL() || strFromTag.IsNULL())
    {
        return IMS_FALSE;
    }

    if (pDialog != IMS_NULL)
    {
        delete pDialog;
        pDialog = IMS_NULL;
    }

    if (bUAS)
        pDialog = new Dialog(strCallId, strToTag, strFromTag);
    else
        pDialog = new Dialog(strCallId, strFromTag, strToTag);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Replaces::Equals(IN CONST Replaces* pOther) const
{
    //---------------------------------------------------------------------------------------------

    if (pOther == IMS_NULL)
        return IMS_FALSE;

    if (!strCallId.Equals(pOther->strCallId))
        return IMS_FALSE;

    if (!strFromTag.Equals(pOther->strFromTag))
        return IMS_FALSE;

    if (!strToTag.Equals(pOther->strToTag))
        return IMS_FALSE;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
const AString& Replaces::GetCallId() const
{
    //---------------------------------------------------------------------------------------------

    return strCallId;
}

/*

Remarks

*/
PUBLIC
const AString& Replaces::GetFromTag() const
{
    //---------------------------------------------------------------------------------------------

    return strFromTag;
}

/*

Remarks

*/
PUBLIC
const AString& Replaces::GetToTag() const
{
    //---------------------------------------------------------------------------------------------

    return strToTag;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Replaces::IsEarlyOnly() const
{
    //---------------------------------------------------------------------------------------------

    return bIsEarlyOnly;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Replaces::IsSameDialog(IN CONST Replaces* pOther) const
{
    //---------------------------------------------------------------------------------------------

    if (pOther == IMS_NULL)
        return IMS_FALSE;

    if ((pDialog == IMS_NULL) || (pOther->pDialog == IMS_NULL))
        return IMS_FALSE;

    return pDialog->Equals(pOther->pDialog);
}

/*

Remarks

*/
PUBLIC
AString Replaces::ToString(IN IMS_BOOL bPercentEncoding) const
{
    AString strReplaces;

    //---------------------------------------------------------------------------------------------

    if (bPercentEncoding)
    {
        // Check if '@' is present in Call-ID header
        IMS_SINT32 nIndex = strCallId.GetIndexOf(TextParser::CHAR_AT);

        if (nIndex == AString::NPOS)
        {
            strReplaces.Append(strCallId);
        }
        else
        {
            AString strUserPart = strCallId.GetSubStr(0, nIndex);
            AString strDomainPart = strCallId.GetSubStr(nIndex + 1);

            strReplaces.Append(strUserPart);
            strReplaces.Append("%40");
            strReplaces.Append(strDomainPart);
        }

        strReplaces.Append("%3Bfrom-tag%3D");
        strReplaces.Append(strFromTag);
        strReplaces.Append("%3Bto-tag%3D");
        strReplaces.Append(strToTag);
    }
    else
    {
        strReplaces.Append(strCallId);
        strReplaces.Append(";from-tag=");
        strReplaces.Append(strFromTag);
        strReplaces.Append(";to-tag=");
        strReplaces.Append(strToTag);
    }

    if (bIsEarlyOnly)
    {
        strReplaces.Append(";early-only");
    }

    return strReplaces;
}
