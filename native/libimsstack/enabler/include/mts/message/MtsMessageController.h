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
#include "MtsService.h"

class IMtsMessage;
class IMtsMessageControllerListener;
class IPageMessage;
class MtsDynamicLoader;

class MtsMessageController final : public ImsActivityEx, public IMtsServiceListener
{
public:
    MtsMessageController(IN IMS_SINT32 nSlotID, IN MtsService* pMtsService,
            IN MtsDynamicLoader* pMtsDynamicLoader);
    ~MtsMessageController();

    void DestroyMtsMessage();

    void Add(IN IMtsMessage* piMtsMessage);
    void Remove(IN IMtsMessage* piMtsMessage);
    IMtsMessage* Search(IN const AString& strDestination);
    IMtsMessage* Search(IN const AString& strDestination, IN IMS_SINT32 nMti);
    IMtsMessage* Search(
            IN IMS_SINT32 nMessageReference, IN IMS_SINT32 nMessageType = MESSAGE_TYPE_RECEIVE);

    void RegisterNoTransactionListener(IN IMtsMessageControllerListener* piListener);
    void DeregisterNoTransactionListener(IN IMtsMessageControllerListener* piListener);
    IMS_BOOL HasMessageSendingReceiving();
    void TerminateAllPendingMessages(IN IMS_BOOL bIs1xCallTerm);
    void TerminateAllPendingMessagesEx(IN IMS_UINT32 nReason);

    const AString& GetLastIpsmgwAddr();
    void SetLastIpsmgwAddr(IN const AString& strSmgwAddr);

    IMS_BOOL IsDeliverMessage(IN IPageMessage* piPageMessage);
    ICoreService* GetICoreService();
    MtsDynamicLoader* GetMtsUtils();

    void SetCallStateType(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    IMS_BOOL IsEmergencyCalling();

    IMS_RESULT ReportMoStatus(IN IMS_UINT32 nReason, IN IMS_UINT32 nSmsFormat,
            IN IMS_UINT8 nRetryAfter = 0, IN IMS_SINT32 nSeqId = -1);
    IMS_UINT32 ReportMtSms(IN IMS_UINT32 nSmsFormat, IN IMS_UINT32 nSmsLength,
            IN const IMS_BYTE* pbySmsData);

    void ReportTransmissionResult(IN IMS_UINT32 nResponse, IN IMS_UINT32 nSmsType,
            IN IMS_SINT32 nSeqId = -1);
    void ReportTransmissionFailureWithRetryTime(IN const IMS_UINT32 nSmsType,
            IN const IMS_UINT8 nRetryTime, IN IMS_SINT32 nSeqId = -1);

    // IMtsServiceListener
    virtual void NotifyMoSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId) override;
    virtual void NotifyMtSms(IN IPageMessage* piMessage) override;

protected:
    // ImsActivityEx
    IMS_BOOL OnMessage(IN IMSMSG& objMSG);

private:
    void ReceiveMtsMessage(IN IPageMessage* piPageMessage, IN IMS_BOOL bIsSmsEServiceType);
    void SendMtsMessage(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bIsSmsEServiceType);
    void UpdateRPAckMap(IN IPageMessage* piPageMessage);

public:
    enum
    {
        MESSAGE_TYPE_RECEIVE = 0,
        MESSAGE_TYPE_SEND = 1,
    };

    enum
    {
        TYPE_CS = 0,
        TYPE_NORMAL,
        TYPE_EMERGENCY
    };

    enum
    {
        STATE_IDLE = 0,
        STATE_TERMINATING = 1,
        STATE_RINGBACK = 2,
        STATE_RINGING = 3,
        STATE_ALERTING = 4,
        STATE_OFFHOOK = 5
    };

    // State of Service
    enum
    {
        STATE_INIT = 0,
        STATE_READY,
        STATE_LIMITED,
        STATE_NOTREADY
    };

    enum
    {
        MO_INVALID = 0,
        MO_SUCCESS = 1,
        MO_IMS_TEMP_FAILURE = 2,
        MO_IMS_PERM_FAILURE = 3,
        MO_IMS_LIMITEDSMSSVCREGI = 4,
        MO_RETRY_CS = 5,
        MO_RETRY_CS_OR_SGS = 6
    };

    enum
    {
        MT_INVALID = 0,
        MT_SUCCESS = 1,
        MT_FAILURE = 2,
        MT_SMS_FORMAT_FAILURE = 3,
        MT_SMS_NODATA_FAILURE = 4
    };

protected:
    IMS_SINT32 m_nSlotId;

public:
    IMS_BOOL m_bProcessingMsg;
    IMS_UINT32 m_nCallTypeMsg;
    IMS_UINT32 m_nCallStateMsg;

private:
    AString m_strLastRcvIpsmgwAddr;
    IMSList<IMtsMessage*> m_objMsgList;
    IMtsMessageControllerListener* m_piMtsMessageControllerListener;
    IMSList<IMtsMessage*> m_objRPAckedMsgs;
    MtsService* m_pMtsService;

protected:
    MtsDynamicLoader* m_pMtsDynamicLoader;
};

#endif
