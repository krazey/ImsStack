/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_MESSAGE_HANDLER_H_
#define _SIP_MESSAGE_HANDLER_H_

#include "ISIPTransportListener.h"

class SIPTransactionState;

class SIPMessageHandler : public ISIPTransportListener
{
private:
    SIPMessageHandler();
    SIPMessageHandler(IN CONST SIPMessageHandler& objRHS);

public:
    ~SIPMessageHandler();

public:
    static SIPMessageHandler* GetInstance();

private:
    // ISIPTransportListener interface
    virtual void Transport_PacketReceived(IN IMS_SINT32 nSlotId, IN CONST ByteArray& objBuffer,
            IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd);

    void NotifyPacketReceived(IN IMS_SINT32 nSlotId, IN CONST ByteArray& objBuffer,
            IN SipMessage* pstMessage, IN IMS_SINT32 nProcessingResult);
    IMS_SINT32 NotifyRequest(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage,
            IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd);
    IMS_SINT32 NotifyResponse(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage,
            IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd);

    IMS_BOOL CheckIPSecValidityForRequest(IN IMS_SINT32 nSlotId, IN SIPTransactionState* pTState,
            IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd);
    IMS_BOOL CheckIPSecValidityForResponse(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage,
            IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd);
    IMS_BOOL IsIPSecSAMatched(IN IMS_SINT32 nSlotId, IN CONST SIPTransportAddress& objNearEnd,
            IN CONST SIPTransportAddress& objFarEnd);
    IMS_BOOL IsIPSecSAMatchedForUS(
            IN IMS_SINT32 nSlotId, IN CONST IPAddress& ojbIP, IN IMS_SINT32 nPort);
    IMS_BOOL IsSecuredMessage(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage);

    IMS_BOOL CheckRegContactValidity(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage);
};

#endif  // _SIP_MESSAGE_HANDLER_H_
