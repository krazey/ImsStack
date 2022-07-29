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

#include "ICoreService.h"
#include "ImsActivityEx.h"
#include "IuMts.h"
#include "MtsDef.h"
#include "MtsService.h"

class IMtsMessage;
class IMtsMessageControllerListener;
class IPageMessage;
class MtsDynamicLoader;

class MtsMessageController final : public ImsActivityEx, public IMtsServiceListener
{
public:
    MtsMessageController(IN IMS_SINT32 nSlotID, IN IMtsService* piMtsService,
            IN MtsDynamicLoader* pMtsDynamicLoader);
    ~MtsMessageController();

    void DestroyMtsMessage();

    void Add(IN IMtsMessage* piMtsMessage);
    void Remove(IN IMtsMessage* piMtsMessage);
    IMtsMessage* Search(IN const AString& strDestination);
    IMtsMessage* Search(IN const AString& strDestination, IN IMS_SINT32 nMti);
    IMtsMessage* Search(IN IMS_SINT32 nMessageReference,
            IN MtsTransactionType eMessageType = MtsTransactionType::MESSAGE_TYPE_RECEIVE);

    void RegisterNoTransactionListener(IN IMtsMessageControllerListener* piListener);
    void DeregisterNoTransactionListener(IN IMtsMessageControllerListener* piListener);
    IMS_BOOL HasMessageSendingReceiving();
    void TerminateAllPendingMessages(IN IMS_BOOL bIs1xCallTerm);
    void TerminateAllPendingMessagesEx(IN IMS_UINT32 nReason);

    const AString& GetLastIpsmgwAddr();
    void SetLastIpsmgwAddr(IN const AString& strSmgwAddr);

    IMS_BOOL IsDeliverMessage(IN IPageMessage* piPageMessage);
    ICoreService* GetICoreService(IN IMS_BOOL bEmergency);
    MtsDynamicLoader* GetMtsUtils();

    void SetCallStateType(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    IMS_BOOL IsEmergencyCalling();

    IMS_RESULT ReportMoStatus(IN IMS_UINT32 nReason, IN SmsFormatType eSmsFormat,
            IN IMS_UINT8 nRetryAfter = 0, IN IMS_SINT32 nSeqId = -1);
    IMS_UINT32 ReportMtSms(
            IN SmsFormatType eSmsFormat, IN IMS_UINT32 nSmsLength, IN const IMS_BYTE* pbySmsData);

    void ReportTransmissionResult(
            IN IMS_UINT32 nResponse, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId = -1);
    void ReportTransmissionFailureWithRetryTime(
            IN SmsFormatType eSmsFormat, IN const IMS_UINT8 nRetryTime, IN IMS_SINT32 nSeqId = -1);

    // IMtsServiceListener
    virtual void NotifyMoSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency) override;
    virtual void NotifyMtSms(IN IPageMessage* piMessage) override;

private:
    // ImsActivityEx
    IMS_BOOL OnMessage(IN IMSMSG& objMSG);

    void ReceiveMtsMessage(IN IPageMessage* piPageMessage, IN IMS_BOOL bEmergency);
    void SendMtsMessage(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency);
    void UpdateRPAckMap(IN IPageMessage* piPageMessage);

public:
    IMS_BOOL m_bProcessingMsg;
    IMS_UINT32 m_nCallTypeMsg;
    IMS_UINT32 m_nCallStateMsg;

private:
    IMS_SINT32 m_nSlotId;
    AString m_strLastRcvIpsmgwAddr;
    IMSList<IMtsMessage*> m_objMsgList;
    IMSList<IMtsMessage*> m_objRPAckedMsgs;
    IMtsMessageControllerListener* m_piMtsMessageControllerListener;
    IMtsService* m_piMtsService;
    MtsDynamicLoader* m_pMtsDynamicLoader;
};

#endif
