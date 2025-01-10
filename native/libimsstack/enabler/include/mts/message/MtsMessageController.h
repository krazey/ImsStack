/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MTS_MESSAGE_CONTROLLER_H_
#define MTS_MESSAGE_CONTROLLER_H_

#include "ByteArray.h"
#include "IPageMessageListener.h"
#include "ITimer.h"
#include "ImsActivityEx.h"
#include "MtsDef.h"
#include "helper/MtsTransactionTimerUpdateHelper.h"
#include "message/IMtsMessageController.h"
#include <functional>

class IMessage;
class IMtsContext;
class IMtsMessage;
class IPageMessage;
class IMtsErrorHandler;

class MtsMessageController :
        public ImsActivityEx,
        public IPageMessageListener,
        public IMtsMessageController,
        public ITimerListener
{
public:
    MtsMessageController(IN IMtsContext& objContext);
    virtual ~MtsMessageController();
    MtsMessageController(IN const MtsMessageController&) = delete;
    MtsMessageController& operator=(IN const MtsMessageController&) = delete;

    // IPageMessageListener
    void PageMessageDelivered(IN IPageMessage* piPageMessage) override;
    void PageMessageDeliveryFailed(IN IPageMessage* piPageMessage) override;

    // IMtsMessageController
    void NotifyMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency) override;
    void NotifyMtSms(IN IPageMessage* piPageMessage) override;
    void ClearAllMessages() override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

protected:
    IMS_BOOL OnMessage(IN ImsMessage& objMsg);

private:
    void DestroyMtsMessage();
    void Add(IN IMtsMessage* piMtsMessage);
    void Remove(IN const IMtsMessage* piMtsMessage);
    IMtsMessage* Search(IN const IPageMessage* piPageMessage);
    IMtsMessage* Search(IN const AString& strDestination, IN IMS_SINT32 nMti);
    IMtsMessage* Search(IN IMS_SINT32 nMessageReference,
            IN MtsTransactionType eMessageType = MtsTransactionType::MESSAGE_TYPE_RECEIVE);

    void ReceiveMtsMessage(IN IPageMessage* piPageMessage, IN IMS_BOOL bEmergency);
    IMS_RESULT SendMtsMessage(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency);
    void ReportMoStatus(
            IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId = -1);
    void ReportMtSms(IN SmsFormatType eSmsFormat, IN IMS_UINT32 nContentLength,
            IN const IMS_BYTE* pbyContent);

    IMS_BOOL ConstructSendMessage(IN IMessage* piMessage, IN const ByteArray& objContent,
            IN SmsFormatType eSmsFormat, IN IMS_BOOL bEmergency);
    IMS_BOOL FormDestinationByMti(IN SmsFormatType eSmsFormat, IN const ByteArray& objContent,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, OUT AString& strDestination);
    const ByteArray& ProcessReceivedMessage(
            IN IPageMessage* piPageMessage, IN IMtsMessage* piMtsMessage);
    void ReportTransmissionResult(
            IN IMS_SINT32 nResponse, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId = -1);
    IMS_RESULT RespondReceivedMessage(
            IN IPageMessage* piPageMessage, IN IMtsMessage* piMtsMessage, IN IMS_BOOL bAdded);
    void Retry_MtsMessageInPending(IN IMtsMessage* piMtsMessage);

    void CleanMtsMessage(IN IMtsMessage* piMtsMessage);
    void CleanMtsMessageWithRpMr(IN IMS_SINT32 nMrOfRp);
    void CleanMtsMessageWithInReplyTo(IN IPageMessage* piPageMessage);
    void CleanRetryContent();
    void TerminateAllMessages();
    void TerminateMessage(IN IMtsMessage* piMtsMessage);

    const AString& GetLastIpsmgwAddr();
    void SetLastIpsmgwAddr(IN const AString& strSmgwAddr);
    void SetLocationToMessage(IN IMessage* piMessage);

    ICoreService* GetICoreService(IN IMS_BOOL bEmergency) const;
    AString GetPreviousCallId(IN const IMtsMessage* piMtsMessage) const;
    static IMS_BOOL GetSmsgwFromReceivedMessage(
            IN const IPageMessage* piPageMessage, OUT AString& strSmsgw);
    static void GetUriFromHeaders(IN const AString& strFromHdr, OUT AString& strUri);
    IMS_BOOL IsDeliverMessage(IN IPageMessage* piPageMessage);
    static IMS_BOOL IsReceivedMessage(IN IMtsMessage* piMtsMessage);
    void SetMessageInfo(IN IPageMessage* piPageMessage, IN const ByteArray& objContent,
            IN SmsFormatType eSmsFormat, IN const AString& strDestination,
            IN MtsTransactionType eMessageType, OUT IMtsMessage* piMtsMessage);

    void StartRetryAfterTimer(IN IMS_SINT32 nRetryAfterValue);
    void StopRetryAfterTimer();
    IMS_RESULT ValidateInReplyToHeader(IN const IPageMessage& objPageMessage) const;

protected:
    ImsList<IMtsMessage*> m_objMsgList;

private:
    IMtsContext& m_objContext;
    IMS_BOOL m_bProcessingMsg;
    AString m_strLastRcvIpsmgwAddr;
    AString m_strPreviousCallId;
    IMtsErrorHandler* m_piMtsErrorHandler;
    ITimer* m_piRetryAfterTimer;
    std::function<void()> m_objRetryFunction;
    ByteArray* m_pRetryContent;
    MtsTransactionTimerUpdateHelper m_objTimerUpdateHelper;
};

#endif
