/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090319  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "Protocol.h"
#include "ProtocolPermission.h"
#include "Connector.h"

__IMS_TRACE_TAG_IMS__;

/*

Remarks

*/
PUBLIC
IConnection* Connector::Open(IN const AString& strName)
{
    IMS_SINT32 nIndexOfColon = strName.GetIndexOf(TextParser::CHAR_COLON);

    if (nIndexOfColon == AString::NPOS)
    {
        IMS_TRACE_E(0, "Connection name is malformed: %s", strName.GetStr(), 0, 0);
        return IMS_NULL;
    }

    AString strURI;

    // Remove the display name and AQUOT if present
    IMS_SINT32 nIndexOfLAQUOT = strName.GetIndexOf(TextParser::CHAR_LAQUOT);

    if (nIndexOfLAQUOT != AString::NPOS)
    {
        IMS_SINT32 nIndexOfRAQUOT = strName.GetIndexOf(TextParser::CHAR_RAQUOT);

        strURI = strName.GetSubStr(nIndexOfLAQUOT + 1, nIndexOfRAQUOT - nIndexOfLAQUOT - 1);
    }
    else
    {
        strURI = strName;
    }

    strURI = strURI.Trim();

    // Tokenize the scheme field in the given name
    nIndexOfColon = strURI.GetIndexOf(TextParser::CHAR_COLON);

    AString strScheme = strURI.Left(nIndexOfColon).Trim();

    // Look up the protocol to determine if this URI scheme supports or not
    Protocol* pProtocol = ProtocolPermission::GetInstance()->Lookup(strScheme);

    if (pProtocol == IMS_NULL)
    {
        // ConnectionNotFoundException
        IMS_TRACE_E(0, "Protocol permissioni is not allowed: %s, uri: %s", strName.GetStr(),
                strURI.GetStr(), 0);
        return IMS_NULL;
    }

    return pProtocol->OpenPrim(strURI);
}

/*

Remarks

*/
PUBLIC
IConnection* Connector::Open(
        IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams)
{
    AString strTmpScheme = strScheme.Trim();

    // Look up the protocol to determine if this URI scheme supports or not
    Protocol* pProtocol = ProtocolPermission::GetInstance()->Lookup(strTmpScheme);

    if (pProtocol == IMS_NULL)
    {
        IMS_TRACE_E(0, "Protocol permissioni is not allowed: %s", strScheme.GetStr(), 0, 0);
        return IMS_NULL;
    }

    return pProtocol->OpenPrim(strTmpScheme, strTarget, strParams);
}
