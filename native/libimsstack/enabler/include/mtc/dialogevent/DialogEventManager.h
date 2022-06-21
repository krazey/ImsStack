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

#ifndef DIALOG_EVENT_MNGR_H_
#define DIALOG_EVENT_MNGR_H_

#include "ImsActivityEx.h"

#include "ISubscription.h"
#include "ISubscriptionListener.h"
#include "ICoreService.h"
#include "IMessage.h"
#include "IDocument.h"
#include "ITimer.h"

#include "define/MtcStringDef.h"
#include "call/MtcUiNotifier.h"
#include "IMtcApp.h"
#include "IMtcService.h"
#include "call/IMtcCall.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/IMtcTimerListener.h"

#include "dialogevent/IDialogEventManagerListener.h"
#include "dialogevent/DialogEvent.h"
#include "define/MtcInternalMsgDef.h"

#include "CallReasonInfo.h"

class DEMngr : public ImsActivityEx, public ISubscriptionListener, public IMtcTimerListener
{
public:
    DEMngr(IN IMtcApp* pApp);
    virtual ~DEMngr();

private:
    DEMngr(IN CONST DEMngr& objRHS);
    DEMngr& operator=(IN CONST DEMngr& objRHS);

public:
    virtual void Init(IN IDEMngrListener* pListener);
    virtual void DeInit();
    virtual void Start(IN AString aStrJNIServiceName);
    virtual IMS_BOOL Stop(IN IMS_BOOL bDestroy = IMS_FALSE);

    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

    virtual void SubscriptionForkedNotify(
            IN ISubscription* piSubscription, IN ISubscription* piForkedSubscription);
    virtual void SubscriptionNotify(IN ISubscription* piSubscription, IN IMessage* piNotify);
    virtual void SubscriptionStarted(IN ISubscription* piSubscription);
    virtual void SubscriptionStartFailed(IN ISubscription* piSubscription);
    virtual void SubscriptionTerminated(IN ISubscription* piSubscription);

    virtual void UCTimer_Expired(IN IMS_SINT32 eType);

public:
    IMS_BOOL UpdateService(IN IMtcService* pService);
    IMS_BOOL Subscribe(IN ICoreService* pICoreService);
    IMS_BOOL UnSubscribe();

    IDialogEvent* CreateDialog(IN IElement* pDialogElement);
    IDialogEvent* GetDialog(IN IElement* pDialogElement);
    IDialogEvent* GetDialog(IN AString aStrID);
    void DestroyDialog(IN AString aStrID);
    void DestroyAllDialog();

protected:
    virtual void HandleThisMsg(IN IMSMSG& objMSG);

    virtual IDialogEvent* Dialog_Create(IN IElement* pDialogElement);
    virtual IDialogEvent* Dialog_CreateCom();

    virtual AString GetFromURI();
    virtual AString GetToURI();
    virtual AString GetRequestURI();

    virtual void SetHeaderSubscribe();
    virtual void SetHeaderUnSubscribe();

    virtual void HandleForkedNotify(IN IMSMSG& objMSG);
    virtual void HandleNotify(IN IMSMSG& objMSG);
    virtual void HandleStarted(IN IMSMSG& objMSG);
    virtual void HandleStartFailed(IN IMSMSG& objMSG);
    virtual void HandleTerminated(IN IMSMSG& objMSG);
    virtual IMS_BOOL HandleFailureRes(IN ISubscription* pISubscription, IN IMS_SINT32 nStatusCode);
    virtual IMS_BOOL HandleFailureRes_423(IN ISubscription* pISubscription);
    virtual IMS_BOOL HandleSubState(IN AString aStrSubState);
    virtual IMS_BOOL HandleRetryByTerminated();
    virtual IMS_BOOL HandleUnSubCompleted(IN ISubscription* pISubscription);
    virtual IMS_BOOL HandleDialogInfo(IN const AString& aStrDialogInfo);
    virtual IMS_BOOL UpdateDialogInfo(IN IElement* pDialogInfoElement);
    virtual void HandleChangedCallState(IN IMSMSG& objMsg);
    virtual void HandleChangedTotalCallState(IN IMSMSG& objMsg);

    virtual IMSList<DialogInfo*> GetDialogInfos();
    virtual IMS_SINT32 GetStateReason(IN IMS_UINT32 eState, IN IDialogEvent* pDialog);
    virtual IMS_SINT32 GetStateCode(IN IMS_UINT32 eState, IN IDialogEvent* pDialog);
    virtual IMS_BOOL IsInitiator(IN IDialogEvent* pDialog);
    virtual IMS_BOOL IsConf(IN IDialogEvent* pDialog);
    virtual MediaInfo* GetMediaInfo(IN IDialogEvent* pDialog);
    virtual IMS_BOOL IsCallPull(IN IDialogEvent* pDialog);
    virtual AString ConvertNumber(IN AString aStrIdentity);

    virtual void SendTerminatedToListn(IN CallReasonInfo terminatedReason);
    virtual void SendNotifyInfoToUI(IN IMSList<DialogInfo*> lstDialogInfos);

    IMS_UINT32 GetCallState();
    IMS_UINT32 ConvertInfoState(IN AString aStrState);
    IMS_UINT32 ConvertTerminatedReason(IN AString aStrReason);

    virtual void LoadConfig();
    virtual void AddEventListn();
    virtual void DeleteEventListn();

public:
    enum
    {
        DEMNGR_BASE_DEFAULT = MTC_INTERNAL_MSG::DIALOG_MSG_BASE,

        DEMNGR_S_FORKEDNOTIFY,
        DEMNGR_S_NOTIFY,
        DEMNGR_S_STARTED,
        DEMNGR_S_STARTFAILED,
        DEMNGR_S_TERMINATED,

        DEMNGR_CALLSTATE_CHANGED,
        DEMNGR_CALLSTATE_TOTALCHANGED,

        DEMNGR_COM_DEFAULT = DEMNGR_BASE_DEFAULT + 50
    };

    enum DIALOGINFO_STATE
    {
        DIALOGINFO_STATE_IDLE = 0,

        DIALOGINFO_STATE_FULL = 1,
        DIALOGINFO_STATE_PARTIAL = 2,
        DIALOGINFO_STATE_DELETED = 3
    };

    enum
    {
        DIALOG_TERMINATED_REASON_NONE = 0,
        DIALOG_TERMINATED_REASON_DEACTIVATED = 1,
        DIALOG_TERMINATED_REASON_PROBATION = 2,
        DIALOG_TERMINATED_REASON_REJECTED = 3,
        DIALOG_TERMINATED_REASON_TIMEOUT = 4,
        DIALOG_TERMINATED_REASON_GIVEUP = 5,
        DIALOG_TERMINATED_REASON_NORESOURCE = 6,
        DIALOG_TERMINATED_REASON_UNKNOWN = 7
    };

protected:
    IMtcApp* m_pApp;
    IDEMngrListener* m_pListener;
    MtcTimerWrapper* m_pTimer;
    AString m_aStrJNIServiceName;
    IMS_BOOL m_bDestroy;
    IMS_UINT32 m_eCallState;
    IMS_SINT32 m_nSlotID;

    IMtcService* m_pService;
    ISubscription* m_pISubscription;
    IMSMap<AString, IDialogEvent*> m_objDialogs;

    IMS_SINT32 m_nExpireTime;

    // dialog-info element
    IMS_UINT32 m_nVersion;  // mandatory / attribute
    IMS_UINT32 m_eState;    // mandatory / attribute
    AString m_aStrEntity;   // mandatory / attribute

    // For subscription re-try by "Subscription-state", RFC 3265
    IMS_UINT32 m_eTerminatedReason;
    IMS_UINT32 m_nRetryAfter;

protected:
    enum
    {
        /* Base Timer */
        TIMER_BASE_DEFAULT = 0,

        TIMER_BASE_TERMINATED_RETRY = 1,

        /* Com Timer */
        TIMER_COM_DEFAULT = 10
    };

private:
};

#endif
