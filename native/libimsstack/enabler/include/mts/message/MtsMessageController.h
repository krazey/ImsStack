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

#include "IMtsServiceListener.h"
#include "IPageMessageListener.h"
#include "ImsActivityEx.h"
#include "MtsDef.h"
#include "message/IMtsErrorHandlerListener.h"

class IMessage;
class IMtsMessage;
class IMtsService;
class IPageMessage;
class MtsDynamicLoader;
class IMtsErrorHandler;

class MtsMessageController final :
        public ImsActivityEx,
        public IPageMessageListener,
        public IMtsServiceListener,
        public IMtsErrorHandlerListener
{
public:
    MtsMessageController(IN IMS_SINT32 nSlotId, IN IMtsService* piMtsService,
            IN MtsDynamicLoader* pMtsDynamicLoader);
    ~MtsMessageController();

    // IPageMessageListener
    void PageMessageDelivered(IN IPageMessage* piPageMessage) override;
    void PageMessageDeliveryFailed(IN IPageMessage* piPageMessage) override;

    // IMtsServiceListener
    void NotifyMoSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency) override;
    void NotifyMtSms(IN IPageMessage* piPageMessage) override;
    void OnDisconnected() override;
    void OnSuspended() override;

    // IMtsErrorHandlerListener
    void NotifyControlAos(IMS_UINT32 nCommand) override;

private:
    // ImsActivityEx
    IMS_BOOL OnMessage(IN IMSMSG& objMSG);

    void DestroyMtsMessage();
    void Add(IN IMtsMessage* piMtsMessage);
    void Remove(IN IMtsMessage* piMtsMessage);
    IMtsMessage* Search(IN const AString& strDestination);
    IMtsMessage* Search(IN IPageMessage* piPageMessage);
    IMtsMessage* Search(IN const AString& strDestination, IN IMS_SINT32 nMti);
    IMtsMessage* Search(IN IMS_SINT32 nMessageReference,
            IN MtsTransactionType eMessageType = MtsTransactionType::MESSAGE_TYPE_RECEIVE);

    void ReceiveMtsMessage(IN IPageMessage* piPageMessage, IN IMS_BOOL bEmergency);
    void SendMtsMessage(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency);
    IMS_RESULT ReportMoStatus(IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat,
            IN IMS_UINT8 nRetryAfter = 0, IN IMS_SINT32 nSeqId = -1);
    IMS_UINT32 ReportMtSms(
            IN SmsFormatType eSmsFormat, IN IMS_UINT32 nSmsLength, IN const IMS_BYTE* pbySmsData);

    IMS_BOOL ConstructSendMessage(IN IMessage* piMessage, IN const ByteArray& objSms,
            IN SmsFormatType eSmsFormat, IN IMS_BOOL bEmergency);
    IMS_BOOL FormDestinationByMti(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, OUT AString& strDestination);
    IMS_BOOL ProcessReceivedMessage(
            IN IPageMessage* piPageMessage, IN IMtsMessage* piMtsMessage, OUT ByteArray& objSms);
    void ReportTransmissionResult(
            IN IMS_SINT32 nResponse, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId = -1);
    void ReportTransmissionFailureWithRetryTime(
            IN SmsFormatType eSmsFormat, IN const IMS_UINT8 nRetryTime, IN IMS_SINT32 nSeqId = -1);
    IMS_BOOL RespondReceivedMessage(IN IPageMessage* piPageMessage, IN IMtsMessage* piMtsMessage,
            IN IMS_UINT32 nMtResult, IN IMS_BOOL bAdded);
    void Retry_MtsMessageInPending(IN IMtsMessage* piMtsMessage);

    void CleanMtsMessage(IN IMtsMessage* piMtsMessage);
    void CleanOperatorMtsMessage(IN IMS_SINT32 nMrOfRp);
    void TerminateAllMessages();
    void TerminateMessage(IN IMtsMessage* piMtsMessage);

    const AString& GetLastIpsmgwAddr();
    void SetLastIpsmgwAddr(IN const AString& strSmgwAddr);

    AString GetPreviousCallId(IN const ByteArray& objSms);
    IMS_SINT32 GetRetryAfterValue(IN IMessage* piMessage);
    IMS_BOOL GetSmsgwFromReceivedMessage(
            IN const IPageMessage* piPageMessage, OUT AString& strSmsgw);
    void GetUriFromHeaders(IN const AString& strFromHdr, OUT AString& strUri) const;
    void GetUserPartFromUris(IN const AString& strUri, OUT AString& strUserPart) const;
    IMS_BOOL IsDeliverMessage(IN IPageMessage* piPageMessage);
    IMS_BOOL IsReceivedMessage(IN IMtsMessage* piMtsMessage);
    void SetMessageInfo(IN IPageMessage* piPageMessage, IN const ByteArray& objSms,
            IN SmsFormatType eSmsFormat, IN const AString& strDestination,
            IN MtsTransactionType eMessageType, OUT IMtsMessage* piMtsMessage);
    void UpdateRPAckMap(IN IPageMessage* piPageMessage);

    // TODO: Need to check if these methods are deprecated for SCBM/ECBM feature
    void SetCallStateType(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    IMS_BOOL IsEmergencyCalling();

private:
    IMS_BOOL m_bProcessingMsg;
    IMS_UINT32 m_nCallStateMsg;
    IMS_UINT32 m_nCallTypeMsg;
    IMS_SINT32 m_nSlotId;
    AString m_strLastRcvIpsmgwAddr;
    ImsList<IMtsMessage*> m_objMsgList;
    ImsList<IMtsMessage*> m_objRPAckedMsgs;
    IMtsService* m_piMtsService;
    IMtsErrorHandler* m_piMtsErrorHandler;
    MtsDynamicLoader* m_pMtsDynamicLoader;
};

#endif
