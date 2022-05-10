#ifndef INTERFACE_MTS_MESSAGE_H_
#define INTERFACE_MTS_MESSAGE_H_

#include "ByteArray.h"

class IPageMessage;

class IMtsMessage
{
public:
    virtual void SendMessage(IN IPageMessage* piPageMessage, IN const AString& strDestination,
            IN const IMS_UINT32 nSmsType, IN const ByteArray& objSMS) = 0;
    virtual void ReceiveMessage(IN IPageMessage* piPageMessage, IN const AString& strIMPU) = 0;
    virtual void Retry_MtsMessageInPending() = 0;
    virtual IMS_BOOL IsReceivedMessage() = 0;
    virtual AString& GetDestination() = 0;
    virtual IMS_SINT32 GetMessageReference() = 0;
    virtual IMS_BOOL IsProcessingMtsMessage() = 0;
    virtual void SetProcessingMtsMessage() = 0;
    virtual void ResetProcessingMtsMessage() = 0;
    virtual IPageMessage* GetPageMessage() = 0;
    virtual void TerminateMessage(IN IMS_BOOL bIs1xCallTerm) = 0;
    virtual void TerminateMessageEx(IN IMS_UINT32 nReason) = 0;
    virtual void SetSeqId(IN IMS_SINT32 nSeqId) = 0;
    virtual void PrintMsgInfo() = 0;
    virtual IMS_SINT32 GetMti() = 0;
};

#endif
