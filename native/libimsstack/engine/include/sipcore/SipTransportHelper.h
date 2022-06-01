/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_TRANSPORT_HELPER_H_
#define _SIP_TRANSPORT_HELPER_H_

// TCP active connection MUST be created at the start time of raw SIP message transmission.
#include "IMSMap.h"
#include "EngineActivity.h"
#include "ISipTransportHelper.h"
#include "ISipDatagramSocketListener.h"
#include "ISipStreamSocketListener.h"
#include "SipTransportAddress.h"
#include "SipSocketAddress.h"

class ISIPSocketListener;
class ISIPTransportListener;
class ISipLocalDnsQueryListener;

class SIPTransportHelper :
        public EngineActivity,
        public ISipTransportHelper,
        public ISIPStreamSocketListener,
        public ISIPDatagramSocketListener
{
public:
    SIPTransportHelper();
    virtual ~SIPTransportHelper();

private:
    SIPTransportHelper(IN CONST SIPTransportHelper& objRHS);
    SIPTransportHelper& operator=(IN CONST SIPTransportHelper& objRHS);

public:
    // EngineActivity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // Only SIPConnectionNotifier
    void Clear();
    SIPSocket* Create(IN CONST SIPSocketAddress& objSA);
    SIPSocket* CreateStreamSocket(
            IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd);
    void Destroy(IN SIPSocket*& pSocket, IN ISIPSocketListener* piListener);
    void DestroyStreamSocket(
            IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd);
    SIPSocket* Open(IN CONST SIPSocketAddress& objSA);
    SIPSocket* OpenStreamSocket(
            IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd);
    void SetListener(IN ISIPTransportListener* piListener);

    // MULTI_REG_TRANSPORT
    IMS_SINT32 AttachClientInitiatedConnection(IN SIPSocket* pSocket);
    void DetachClientInitiatedConnection(IN SIPSocket* pSocket);
    IMS_BOOL IsClientInitiatedConnection(IN SIPSocket* pSocket) const;

    // LOCAL_DNS_QUERY
    IMS_BOOL GetHostByName(IN CONST IPAddress& objLocalIP, IN CONST AString& strHostname,
            OUT IPAddress& objHostIP);

private:
    // ISipTransportHelper
    virtual void ApplyIpSecForServerSockets();
    virtual void DestroyAllSockets(
            IN IMS_SINT32 nMethod = 0, IN CONST IPAddress& objLocalIP = IPAddress::NONE);
    virtual void DestroyTcpSocket(IN CONST IPAddress& objSrcIP, IN IMS_UINT32 nSrcPort,
            IN CONST IPAddress& objDestIP, IN IMS_UINT32 nDestPort,
            IN IMS_BOOL bIsConnectionByPeer = IMS_FALSE);
    virtual void SetIpQos(IN SipRtConfig::IpQos* pIPQoS);
    virtual void SetKeepAlivePolicy(IN CONST IPAddress& objSrcIP, IN IMS_UINT32 nSrcPort,
            IN CONST IPAddress& objDestIP, IN IMS_UINT32 nDestPort,
            IN IMS_SINT32 nPolicy = (-1) /* default */);
    // LOCAL_DNS_QUERY
    virtual void SetLocalDnsQueryListener(IN ISipLocalDnsQueryListener* piListener);

    // ISIPDatagramSocketListener
    virtual void DatagramSocket_DataReceived(IN SIPSocket* pSocket, IN CONST ByteArray& objBuffer,
            IN CONST IPAddress& objIPA, IN IMS_SINT32 nPort);
    // ISIPStreamSocketListener
    virtual void StreamSocket_ConnectionReceived(IN SIPSocket* pSocket);
    virtual void StreamSocket_DataReceived(IN SIPSocket* pSocket, IN_OUT ByteArray& objBuffer);
    virtual void StreamSocket_KeepAliveExpired(IN SIPSocket* pSocket);
    virtual void StreamSocket_PassiveClosed(IN SIPSocket* pSocket);

    IMS_BOOL AttachSocket(IN SIPSocket* pSocket);
    IMS_BOOL IsSocketPresent(IN SIPSocket* pSocket) const;
    SIPSocket* LookupSocket(IN CONST SIPSocketAddress& objSA, IN IMS_BOOL bDetach = IMS_FALSE);
    SIPSocket* LookupSocket(IN CONST SIPSocket& objSocket, IN IMS_BOOL bDetach = IMS_FALSE);
    SIPSocket* LookupStreamSocket(IN CONST SIPSocketAddress& objSA);
    SIPSocket* LookupStreamSocket(
            IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd);

private:
    // Event for message processing
    enum
    {
        AMSG_PROCESS_MESSAGE = AMSG_USER,
        AMSG_DESTROY_ALL_SOCKETS
    };

    // Result values for the completeness of a raw SIP message (TCP only)
    enum
    {
        MESSAGE_ERROR = (-1),
        MESSAGE_COMPLETE = 0,
        MESSAGE_INCOMPLETE
    };

    // Result values for SipComp module
    enum
    {
        SIGCOMP_NO_COMP = (-3),
        SIGCOMP_INCOMPLETE = (-2),
        SIGCOMP_ERROR = (-1),
        SIGCOMP_SUCCESS = 0
    };

    class TransportBuffer
    {
        friend class SIPTransportHelper;

        ByteArray objData;
        // Destination information of a message (on the basis of sender)
        SIPTransportAddress objTA_NearEnd;
        // Source information of a message (on the basis of sender)
        SIPTransportAddress objTA_FarEnd;

        inline TransportBuffer() {}
        inline TransportBuffer(IN CONST TransportBuffer& objRHS) :
                objData(objRHS.objData),
                objTA_NearEnd(objRHS.objTA_NearEnd),
                objTA_FarEnd(objRHS.objTA_FarEnd)
        {
        }
        inline ~TransportBuffer() {}
        inline TransportBuffer& operator=(IN CONST TransportBuffer& objRHS)
        {
            if (this != &objRHS)
            {
                objData = objRHS.objData;
                objTA_NearEnd = objRHS.objTA_NearEnd;
                objTA_FarEnd = objRHS.objTA_FarEnd;
            }

            return (*this);
        }

        // DEBUG message
        void DisplayMessage(IN IMS_SINT32 nSlotId);
    };

    IMSList<SIPSocket*> objSockets;
    IMSList<TransportBuffer*> objBuffers;
    ISIPTransportListener* piListener;

    // MULTI_REG_TRANSPORT :: <Socket object's pointer, count>
    IMSMap<IMS_UINTP, IMS_SINT32> objClientInitiatedConnections;

    // LOCAL_DNS_QUERY
    // 1) Test purpose
    // 2) Using the application layer's DNS query result
    ISipLocalDnsQueryListener* piDnsQueryListener;
};

#endif  // _SIP_TRANSPORT_HELPER_H_
