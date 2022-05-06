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
#include "TextParser.h"
#include "Protocol.h"

PUBLIC
Protocol::Protocol() {}

PUBLIC VIRTUAL Protocol::~Protocol() {}

/*
 Creates and opens a Connection; SIPConnection, Service, and so on.

Remarks
 The creation of Connections is performed dynamically by looking up a protocol implementation class
 whose name is formed from the platform name (read from a system property) and the protocol name
 of the requested connection (extracted from the parameter string supplied by the application
 programmer).

 It throws the error as follows:
     ILLEGAL_ARGUMENT,
     CONNECTION_NOT_FOUND
*/
PUBLIC VIRTUAL IConnection* Protocol::OpenPrim(IN const AString& /* strName */)
{
    // throw exception
    return IMS_NULL;
}

/*
 Creates and opens a Connection; SIPConnection, Service, and so on.

Remarks
 The creation of Connections is performed dynamically by looking up a protocol implementation class
 whose name is formed from the platform name (read from a system property) and the protocol name
 of the requested connection (extracted from the parameter string supplied by the application
 programmer).

 It throws the error as follows:
     ILLEGAL_ARGUMENT,
     CONNECTION_NOT_FOUND
*/
PUBLIC VIRTUAL IConnection* Protocol::OpenPrim(IN const AString& /* strScheme */,
        IN const AString& /* strTarget */, IN const AString& /* strParams */)
{
    // throw exception
    return IMS_NULL;
}

/*
 Parses the connection string which consists of three parts: scheme, target, parameters.

Remarks

*/
PUBLIC GLOBAL void Protocol::ParseName(IN const AString& strName, OUT AString& strScheme,
        OUT AString& strTarget, OUT AString& strParams)
{
    IMS_SINT32 nPosColon;
    IMS_SINT32 nPosSemiColon;
    IMS_SINT32 nPosAt;

    AString strTmp = strName.Trim();

    // Get scheme information
    nPosColon = strTmp.GetIndexOf(TextParser::CHAR_COLON);
    strScheme = strTmp.GetSubStr(0, nPosColon);

    // Get target information
    nPosAt = strTmp.GetIndexOf(TextParser::CHAR_AT, nPosColon + 1);

    // Get the position of semi-colon to identify if the parameter is present or not.
    if (nPosAt == AString::NPOS)
        nPosSemiColon = strTmp.GetIndexOf(TextParser::CHAR_SEMICOLON, nPosColon + 1);
    else
        nPosSemiColon = strTmp.GetIndexOf(TextParser::CHAR_SEMICOLON, nPosAt + 1);

    // Check if the parameter exists or not
    if (nPosSemiColon == AString::NPOS)
    {
        // Get target
        strTarget = strTmp.GetSubStr(nPosColon + 1);
        strParams = AString::ConstNull();
    }
    else
    {
        // Get target
        strTarget = strTmp.GetSubStr(nPosColon + 1, nPosSemiColon - nPosColon - 1);
        // Get parameters information
        strParams = strTmp.GetSubStr(nPosSemiColon + 1);
    }
}
