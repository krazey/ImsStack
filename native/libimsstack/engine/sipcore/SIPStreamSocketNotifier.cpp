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
#include "ServiceNetwork.h"
#include "ByteArray.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SIPRTConfigUtils.h"
#include "ISIPStreamSocketListener.h"
#include "SIPStreamSocket.h"
#include "SIPStreamSocketNotifier.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPStreamSocketNotifier::SIPStreamSocketNotifier(IN IMS_SINT32 nSlotId) :
        SIPSocket(nSlotId, SIPSocketAddress::SOCKET_TCP),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SIPStreamSocketNotifier::~SIPStreamSocketNotifier()
{
    IMS_TRACE_D("StreamSocketNotifier(D) :: (%s, %d)", SipDebug::GetIp(objSA.GetIPAddress()),
            objSA.GetPort(), 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL SIPSocket* SIPStreamSocketNotifier::Accept()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_CONNECTED)
    {
        return IMS_NULL;
    }

    ISocket* piNewSocket = piSocket->Accept();

    if (piNewSocket != IMS_NULL)
    {
        IMS_TRACE_I("StreamSocket will be created by remote end", 0, 0, 0);

        SIPStreamSocket* pStreamSocket = new SIPStreamSocket(GetSlotId(), piNewSocket);

        if (pStreamSocket == IMS_NULL)
        {
            piNewSocket->SetListener(IMS_NULL);
            piNewSocket->Close();
            NetworkService::GetNetworkService()->DestroySocket(piNewSocket);
            return IMS_NULL;
        }

        ApplyIpSec(piNewSocket);

        // Inherits the server's listener
        pStreamSocket->SetListener(piListener);

        return pStreamSocket;
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPStreamSocketNotifier::Create(IN CONST IPAddress& objIPA,
        IN IMS_UINT32 nPort /* = 0 */, IN IMS_BOOL bSecure /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (!SIPSocket::Create(objIPA, nPort, bSecure))
    {
        return IMS_FALSE;
    }

    if (GetState() != STATE_INITIALIZED)
    {
        return IMS_FALSE;
    }

    if (piSocket->Bind(objIPA, nPort) == ISocket::RESULT_ERROR)
    {
        return IMS_FALSE;
    }

    if (piSocket->Listen() == ISocket::RESULT_ERROR)
    {
        return IMS_FALSE;
    }

    objSA.SetPort(nPort);
    objSA.SetIPAddress(objIPA);

    SetState(STATE_CONNECTED);

    IMS_TRACE_I("StreamSocketNotifier(C) :: (%s, %d)",
            SIPRTConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId()) ? "xxx"
                                                                    : SipDebug::GetIp(objIPA),
            nPort, 0);

    ApplyIpSec();

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
void SIPStreamSocketNotifier::SetListener(IN ISIPStreamSocketListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPStreamSocketNotifier::Socket_OnConnectionReceived(IN ISocket* piSocket)
{
    //---------------------------------------------------------------------------------------------

    SIPSocket::Socket_OnConnectionReceived(piSocket);

    if (piListener != IMS_NULL)
    {
        piListener->StreamSocket_ConnectionReceived(this);

        // Accept & Close : A new connection MUST be extracted from the pending connection queue.
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPStreamSocketNotifier::Socket_OnClosed(
        IN ISocket* piSocket, IN IMS_SINT32 nReason /* = ISocket::CLOSE_REASON_UNKNOWN */)
{
    //---------------------------------------------------------------------------------------------

    SIPSocket::Socket_OnClosed(piSocket, nReason);
}
