/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091126  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "SIPPrivate.h"
#include "SIPManager.h"
#include "SIPConnectionNotifier.h"
#include "SIPConnectionNotifierImpl.h"
#include "SIPClientConnection.h"
#include "SIPClientConnectionImpl.h"
#include "SIPProtocol.h"

__IMS_TRACE_TAG_SIP__;

PRIVATE
SIPProtocol::SIPProtocol() :
        Protocol()
{
}

PUBLIC VIRTUAL SIPProtocol::~SIPProtocol() {}

/*
 Returns a singleton object of SIP protocol.

Remarks

*/
PUBLIC GLOBAL SIPProtocol* SIPProtocol::GetInstance()
{
    static SIPProtocol* pSIP = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pSIP == IMS_NULL)
    {
        pSIP = new SIPProtocol();
    }

    return pSIP;
}

/*
 Creates and opens a SIP Connection.

Remarks
 It throws the error as follows:
     ILLEGAL_ARGUMENT,
     CONNECTION_NOT_FOUND
*/
PRIVATE VIRTUAL IConnection* SIPProtocol::OpenPrim(IN CONST AString& strName)
{
    AString strScheme;
    AString strTarget;
    AString strParams;

    //---------------------------------------------------------------------------------------------

    Protocol::ParseName(strName, strScheme, strTarget, strParams);

    return OpenPrim(strScheme, strTarget, strParams);
}

/*
 Creates and opens a SIP Connection.

Remarks
 It throws the error as follows:
     ILLEGAL_ARGUMENT,
     CONNECTION_NOT_FOUND
*/
PRIVATE VIRTUAL IConnection* SIPProtocol::OpenPrim(
        IN CONST AString& strScheme, IN CONST AString& strTarget, IN CONST AString& strParams)
{
    IMS_SINT32 nScheme;

    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (strScheme.EqualsIgnoreCase(Sip::STR_SIP))
        nScheme = Sip::URI_SCHEME_SIP;
    else
        nScheme = Sip::URI_SCHEME_SIPS;

    // Check if it is SIP connection notifier or not
    if (strTarget.IsEmpty())
    {
        // SIP transaction notifier : dedicated mode (Port will be selected by the system)
        return CreateConnectionNotifier(nScheme, 0, strParams);
    }
    else if (strTarget.Equals(TextParser::CHAR_ASTERISK))
    {
        // SIP transaction notifier : shared mode
        return CreateConnectionNotifier(nScheme, 0, strParams, IMS_TRUE);
    }
    else
    {
        IMS_BOOL bOK = IMS_FALSE;
        IMS_SINT32 nPort = strTarget.ToInt32(&bOK);

        if (bOK == IMS_TRUE)
        {
            // SIP transaction notifier : dedicated mode
            return CreateConnectionNotifier(nScheme, nPort, strParams);
        }
        else
        {
            AString strURI = strScheme + TextParser::CHAR_COLON + strTarget;

            if (strParams.GetLength() > 0)
            {
                strURI.Prepend(TextParser::CHAR_LAQUOT);

                strURI += TextParser::CHAR_SEMICOLON + strParams;

                strURI.Append(TextParser::CHAR_RAQUOT);
            }

            // SIP client transaction
            SIPClientConnection* pSCC = new SIPClientConnection(strURI);

            if (pSCC == IMS_NULL)
            {
                SIPPrivate::SetLastError(SipError::NO_MEMORY);

                IMS_TRACE_E(0, "Allocating SCC failed", 0, 0, 0);
                return IMS_NULL;
            }

            SIPClientConnectionImpl* pSCCImpl = new SIPClientConnectionImpl(pSCC);

            if (pSCCImpl == IMS_NULL)
            {
                delete pSCC;
                SIPPrivate::SetLastError(SipError::NO_MEMORY);

                IMS_TRACE_E(0, "Allocating SCCImpl failed", 0, 0, 0);
                return IMS_NULL;
            }

            return pSCCImpl;
        }
    }
}

PRIVATE
IConnection* SIPProtocol::CreateConnectionNotifier(IN IMS_SINT32 nScheme, IN IMS_SINT32 nPort,
        IN CONST AString& strParams, IN IMS_BOOL bSharedMode /* = IMS_FALSE */)
{
    SIPConnectionNotifier* pSCN = new SIPConnectionNotifier(nScheme, nPort, strParams, bSharedMode);

    //---------------------------------------------------------------------------------------------

    if (pSCN == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Allocating SCN failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (SIPPrivate::GetLastError() != SipError::NO_ERROR)
    {
        delete pSCN;

        IMS_TRACE_E(0, "Parsing the parameters for SCN failed", 0, 0, 0);
        return IMS_NULL;
    }

    SIPConnectionNotifierImpl* pSCNImpl = new SIPConnectionNotifierImpl(pSCN);

    if (pSCNImpl == IMS_NULL)
    {
        delete pSCN;
        SIPPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Allocating SCNImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!SIPManager::GetInstance()->AttachConnectionNotifier(pSCN))
    {
        delete pSCNImpl;

        IMS_TRACE_E(0, "Attaching SCN failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pSCNImpl;
}
