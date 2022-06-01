/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110528  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipDebug.h"
#include "SipFactoryProxy.h"
#include "SipRtConfigUtils.h"
#include "SipTransportHelper.h"
#include "SipSocket.h"
#include "ISipKeepAliveHelperListener.h"
#include "SipKeepAliveHelper.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPKeepAliveHelper::SIPKeepAliveHelper(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        piListener(IMS_NULL)
{
    objSA_NearEnd.SetType(SIPSocketAddress::SOCKET_UDP);
    objSA_FarEnd.SetType(SIPSocketAddress::SOCKET_UDP);
}

PUBLIC VIRTUAL SIPKeepAliveHelper::~SIPKeepAliveHelper() {}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPKeepAliveHelper::Destroy()
{
    SIPSocket* pSocket = IMS_NULL;
    SIPTransportHelper* pTransportHelper =
            SIPFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());

    //---------------------------------------------------------------------------------------------

    if (objSA_NearEnd.GetType() == SIPSocketAddress::SOCKET_UDP)
    {
        pSocket = pTransportHelper->Open(objSA_NearEnd);
    }
    else
    {
        pSocket = pTransportHelper->OpenStreamSocket(objSA_NearEnd, objSA_FarEnd);
    }

    if (pSocket != IMS_NULL)
    {
        pSocket->SetKeepAliveListener(IMS_NULL);
    }

    delete this;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPKeepAliveHelper::SendPacket(IN CONST ByteArray& objPacket)
{
    //---------------------------------------------------------------------------------------------

    if (objSA_FarEnd.GetIPAddress().Equals(IPAddress::NONE) || (objSA_FarEnd.GetPort() <= 0))
    {
        IMS_TRACE_E(0, "IP address & port number MUST be specified to send the keep-alive packet",
                0, 0, 0);
        return IMS_FAILURE;
    }

    if (objPacket.GetLength() == 0)
    {
        IMS_TRACE_E(0, "No packet", 0, 0, 0);
        return IMS_FAILURE;
    }

    SIPSocket* pSocket = IMS_NULL;
    SIPTransportHelper* pTransportHelper =
            SIPFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());

    if (objSA_NearEnd.GetType() == SIPSocketAddress::SOCKET_UDP)
    {
        pSocket = pTransportHelper->Open(objSA_NearEnd);
    }
    else
    {
        pSocket = pTransportHelper->OpenStreamSocket(objSA_NearEnd, objSA_FarEnd);
    }

    if (pSocket == IMS_NULL)
    {
        IMS_TRACE_D("NearEnd - %s, %d, %d", SipDebug::GetIp(objSA_NearEnd.GetIPAddress()),
                objSA_NearEnd.GetPort(), objSA_NearEnd.GetType());
        // LOG_EXCLUDING_SERVER_INFO
        IMS_TRACE_D("FarEnd - %s, %d, %d",
                SIPRTConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                        ? "xxx"
                        : SipDebug::GetIp(objSA_FarEnd.GetIPAddress()),
                objSA_FarEnd.GetPort(), objSA_FarEnd.GetType());
        IMS_TRACE_E(0, "Finding the socket failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    pSocket->SetKeepAliveListener(this);

    if (pSocket->Send(objPacket.GetData(), objPacket.GetLength(), objSA_FarEnd.GetPort(),
                objSA_FarEnd.GetIPAddress()) < 0)
    {
        IMS_TRACE_E(0, "Sending the keep-alive packet failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPKeepAliveHelper::SetListener(IN ISipKeepAliveHelperListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPKeepAliveHelper::SetTransportTupleD(
        IN CONST IPAddress& objIP, IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    objSA_FarEnd.SetIPAddress(objIP);
    objSA_FarEnd.SetPort(nPort);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPKeepAliveHelper::SetTransportTupleS(IN CONST IPAddress& objIP,
        IN IMS_SINT32 nPort, IN IMS_SINT32 nProtocol /* = Sip::TRANSPORT_UDP */)
{
    //---------------------------------------------------------------------------------------------

    objSA_NearEnd.SetIPAddress(objIP);
    objSA_NearEnd.SetPort(nPort);

    if ((nProtocol == Sip::TRANSPORT_TCP) || (nProtocol == Sip::TRANSPORT_TLS))
    {
        objSA_NearEnd.SetType(SIPSocketAddress::SOCKET_TCP_CLIENT);
    }
    else
    {
        objSA_NearEnd.SetType(SIPSocketAddress::SOCKET_UDP);
    }

    objSA_FarEnd.SetType(objSA_NearEnd.GetType());
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPKeepAliveHelper::KeepAlive_PongReceived()
{
    //---------------------------------------------------------------------------------------------

    if (piListener != IMS_NULL)
    {
        piListener->KeepAliveHelper_PongReceived();
    }
}
