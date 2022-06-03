#ifndef MTS_MESSAGE_H_
#define MTS_MESSAGE_H_

#include "message/IMtsMessage.h"
#include "IPageMessageListener.h"
#include "IMessage.h"
#include "message/MtsMessageController.h"
#include "base/IMessageMediator.h"

class MtsMessage final : public IMtsMessage, public IPageMessageListener, public IMessageMediator
{
public:
    MtsMessage(IN IMS_SINT32 nSlotId, IN MtsMessageController* pMtsMessageController,
            IN IMS_BOOL bIsSmsEServiceType);
    ~MtsMessage();

    // IMtsMessage
    void SendMessage(IN IPageMessage* piPageMessage, IN const AString& strDestination,
            IN const IMS_UINT32 nSmsType, IN const ByteArray& objSms) override;
    void ReceiveMessage(IN IPageMessage* piPageMessage, IN const AString& strImpu) override;
    void Retry_MtsMessageInPending() override;
    IMS_BOOL IsReceivedMessage() override;
    AString& GetDestination() override;
    IMS_SINT32 GetMessageReference() override;
    IMS_BOOL IsProcessingMtsMessage() override;
    void SetProcessingMtsMessage() override;
    void ResetProcessingMtsMessage() override;
    IPageMessage* GetPageMessage() override;
    void TerminateMessage(IN IMS_BOOL bIs1xCallTerm) override;
    void TerminateMessageEx(IN IMS_UINT32 nReason) override;
    void SetSeqId(IN IMS_SINT32 nSeqId) override;
    void PrintMsgInfo() override;
    IMS_SINT32 GetMti() override;

    // IPageMessageListener
    void PageMessageDelivered(IN IPageMessage* piPageMessage) override;
    void PageMessageDeliveryFailed(IN IPageMessage* piPageMessage) override;

    // IMessageMediator
    IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nMessage) override;

    IMS_BOOL ConstructSendMessage(
            IN IMessage* piMessage, IN const ByteArray& objSms, IN const IMS_UINT32 nSmsType);
    AString GetPreviousCallId(IN const ByteArray& objSms);
    void SetSendMsgInfo(IN const ByteArray& objSms, IN const IMS_UINT32 nSmsType);
    IMS_BOOL HandleDeliveryResponse(IN IMessage* piMessage);
    void DeliveryFailed_PageMessageNull();
    void DeliveryFailed_TimerF();
    void DeliveryFailed_MessageNull();
    IMS_BOOL Result_ReceiveMessage(
            IN IPageMessage* piPageMessage, IMS_UINT32 nMtResult, IMS_BOOL bAdded);
    IMS_BOOL Processing_ReceiveMessage(
            IN IPageMessage* piPageMessage, IN const AString& strImpu, OUT ByteArray& objSms);
    IMS_SINT32 GetSlotId();
    void CleanMtsMessagewithReportResponse(
            IN IMS_UINT32 nResponse, IN IMS_BOOL bSendToAos = IMS_FALSE, IN IMS_UINT32 nType = 0);
    void CleanMtsMessage();
    void CleanOperatorMtsMessage();

protected:
    void SetDestination(IN const AString& strDestination);
    IMS_UINT32 GetContentType() const;
    void GetUserPartFromUris(IN const AString& strUri, OUT AString& strUserPart) const;
    IMS_BOOL GetSmsgwFromReceivedMessage(
            IN const IPageMessage* piPageMessage, OUT AString& strSmsgw);
    void GetUriFromHeaders(IN const AString& strFromHdr, OUT AString& strUri) const;
    IMS_SINT32 GetRetryAfterValue(IN IMessage* piMessage);

private:
    void ReportTransmissionResultToMessageController(
            IN IMS_UINT32 nResponse, IN IMS_UINT32 nSmsType);
    void SetTrmInfo(IN IMS_SINT32 nSlotId, IN IMS_BOOL bSmsState);

public:
    enum
    {
        // mdn length for to header comparison according to verizon sms over ims.
        MDNLENGTHFORCOMPARISON = 4,
    };

protected:
    IPageMessage* m_piPageMessage;
    AString m_strDestination;
    IMS_BOOL m_bIsSmsEServiceType;

    // SMS Msg Info
    IMS_UINT32 m_nSmsFormat;
    IMS_SINT32 m_nMrOfRp;
    IMS_SINT32 m_nSmsTrxType;
    IMS_SINT32 m_nMti;
    IMS_SINT32 m_nSmSize;

    IMS_SINT32 m_nSeqId;
    AString m_strImpu;
    IMS_UINT32 m_nSlotId;
    MtsMessageController* m_pMtsMessageController;
};

#endif
