/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20151229  hwangoo.park@             Created
    </table>

    Description
    This class provides the port information for SIP TCP/TLS transport protocol.
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceNetwork.h"
#include "ServiceSystemTime.h"
#include "SIPPortManager.h"

__IMS_TRACE_TAG_SIP__;



PRIVATE
SIPPortManager::SIPPortManager()
    : nPortC_Start(0)
    , nPortC_End(CLIENT_PORT_END)
    , nNextPortC(0)
{
}

PRIVATE
SIPPortManager::~SIPPortManager()
{
}

PUBLIC
void SIPPortManager::Clear()
{
    //---------------------------------------------------------------------------------------------

    nPortC_Start = 0;
    nPortC_End = CLIENT_PORT_END;

    nNextPortC = 0;
}

PUBLIC
IMS_SINT32 SIPPortManager::GetPortC(IN CONST IPAddress &objIP) const
{
    //---------------------------------------------------------------------------------------------

    return SelectNextPortC(objIP);
}

PUBLIC
IMS_BOOL SIPPortManager::IsPortCProvisioned() const
{
    //---------------------------------------------------------------------------------------------

    return (nPortC_Start >= CLIENT_PORT_MIN)
            && (nPortC_End <= CLIENT_PORT_MAX);
}

PUBLIC
void SIPPortManager::SetPortC(IN IMS_SINT32 nPortStart, IN IMS_SINT32 nPortEnd)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("SetPortC :: Range (%d-%d)", nPortStart, nPortEnd, 0);

    nPortC_Start = nPortStart;
    nPortC_End = nPortEnd;

    if ((nPortC_End <= 0) || (nPortC_End > CLIENT_PORT_MAX))
    {
        nPortC_End = CLIENT_PORT_END;
    }
    else if (nPortC_Start >= nPortC_End)
    {
        nPortC_Start = 0;
    }

    // Select a starting random port number
    IMS_SINT32 nStartingPort
            = static_cast<IMS_SINT32>(IMS_SYS_GetSRandom0()) % (nPortC_End - nPortC_Start);

    nStartingPort += nPortC_Start;

    SetNextPortC(nStartingPort);
}

PUBLIC GLOBAL
SIPPortManager* SIPPortManager::GetInstance()
{
    static SIPPortManager *pPortManager = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pPortManager == IMS_NULL)
    {
        pPortManager = new SIPPortManager();
    }

    return pPortManager;
}

PRIVATE
IMS_SINT32 SIPPortManager::GetNextPortC() const
{
    //---------------------------------------------------------------------------------------------

    return nNextPortC;
}

PRIVATE
IMS_BOOL SIPPortManager::IsPortAvailable(IN CONST IPAddress &objIP, IN IMS_SINT32 nPort) const
{
    NetworkService *pNetworkService = NetworkService::GetNetworkService();

    //---------------------------------------------------------------------------------------------

    if (pNetworkService == IMS_NULL)
    {
        return IMS_TRUE;
    }

    return pNetworkService->CheckIpAndPortAvailability(objIP, nPort, ISocket::TYPE_STREAM);
}

PRIVATE
IMS_SINT32 SIPPortManager::SelectNextPortC(IN CONST IPAddress &objIP) const
{
    IMS_SINT32 nSelectedPort = 0;
    IMS_SINT32 nCurrentPort = GetNextPortC();

    //---------------------------------------------------------------------------------------------

    if (nCurrentPort <= 0)
    {
        for (IMS_SINT32 i = (nPortC_Start + 1); i < nPortC_End; ++i)
        {
            if (IsPortAvailable(objIP, i))
            {
                nSelectedPort = i;
                break;
            }
        }
    }
    else if (nCurrentPort < nPortC_End)
    {
        for (IMS_SINT32 i = nCurrentPort; i < nPortC_End; ++i)
        {
            if (IsPortAvailable(objIP, i))
            {
                nSelectedPort = i;
                break;
            }
        }
    }

    if (nSelectedPort != 0)
    {
        SetNextPortC(nSelectedPort + 1);
    }

    return nSelectedPort;
}

PRIVATE
void SIPPortManager::SetNextPortC(IN IMS_SINT32 nPort) const
{
    //---------------------------------------------------------------------------------------------

    if (nNextPortC != nPort)
    {
        // Round-robin
        if (nPort == nPortC_End)
        {
            nPort = (nPortC_Start + 1);
        }
        else if (nPort == nPortC_Start)
        {
            nPort = (nPortC_Start + 1);
        }

        IMS_TRACE_D("SetNextPortC :: %d >> %d", nNextPortC, nPort, 0);

        nNextPortC = nPort;
    }
}
