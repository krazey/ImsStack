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

#ifndef UC_DIALOG_H_
#define UC_DIALOG_H_

#include "IMtcApp.h"
#include "dialogevent/IDialogEvent.h"

class UCDialog : public IDialogEvent
{
public:
    UCDialog(IN IMtcApp* pApp);
    virtual ~UCDialog();

    virtual AString Init(IN IElement* pDialogElement);
    virtual void DeInit();

public:
    virtual IMS_BOOL UpdateDialogInfo(
            IN IMS_UINT32 nVersion, IN IMS_UINT32 eState, IN AString aStrEntity);
    virtual IMS_BOOL Update(IN IElement* pDialogElement);
    virtual IMS_BOOL IsDialog(IN IElement* pDialogElement);

    virtual AString GetID();
    virtual AString GetCallID();
    virtual AString GetLocalTag();
    virtual AString GetRemoteTag();
    virtual AString GetDirection();
    virtual IMS_UINT32 GetState();
    virtual AString GetStateEvent();
    virtual AString GetStateCode();
    virtual IMS_UINT32 GetDuration();
    virtual AString GetReferredBy();
    virtual AString GetReferredByDisplay();
    virtual void GetLocal(OUT DialogLR* pstDialogLR);
    virtual AString GetLocalIdentity();
    virtual AString GetLocalIdentityDisplay();
    virtual AString GetLocalpVal(IN AString aStrpName);
    virtual void GetRemote(OUT DialogLR* pstDialogLR);
    virtual AString GetRemoteIdentity();
    virtual AString GetRemoteIdentityDisplay();
    virtual AString GetRemotepVal(IN AString aStrpName);
    virtual IMS_BOOL EnablePulled();

protected:
    virtual IMS_BOOL UpdateDialog(IN IElement* pDialogElement);
    virtual IMS_BOOL UpdateDialogState(IN IElement* pStateElement);
    virtual IMS_BOOL UpdateDialogDuration(IN IElement* pDurationElement);
    virtual IMS_BOOL UpdateDialogReplaces(IN IElement* pReplacesElement);
    virtual IMS_BOOL UpdateDialogReferredBy(IN IElement* pReferredByElement);
    virtual IMS_BOOL UpdateDialogLR(IN IElement* pLRElement, IN DialogLR* pstDialogLR);
    virtual IMS_BOOL UpdateDialogLRIdentity(
            IN IElement* pIdentityElement, IN DialogLR* pstDialogLR);
    virtual IMS_BOOL UpdateDialogLRTarget(IN IElement* pTargetElement, IN DialogLR* pstDialogLR);
    virtual IMS_BOOL UpdateDialogLRTarget_Param(
            IN IElement* pTargetElement, IN DialogLR* pstDialogLR);

    virtual IMS_BOOL UpdateEnablePulled(IN IElement* pDialogElement);
    virtual IMS_BOOL UpdateOnHold(IN IElement* pDialogElement);
    virtual IMS_BOOL UpdateDialogExtraInfo(IN IElement* pDialogElement);

    IMS_UINT32 ConvertDialogState(IN AString aStrState);
    AString GetValueElement(IN IElement* pIElement);
    AString GetValueSubElement(IN IElement* pIElement, IN const IMS_CHAR* strSubElement);
    IElement* GetSubElement(
            IN IElement* pIElement, IN const IMS_CHAR* strSubElement, IN IMS_UINT32 nIndex = 0);
    IMSList<IElement*> GetSubElementList(IN IElement* pIElement, IN const IMS_CHAR* strSubElement);
    AString GetAttributeFromElement(IN IElement* pIElement, IN const IMS_CHAR* strAttribute);

    virtual void LoadConfig();
    virtual void AddEventListn();
    virtual void DeleteEventListn();

public:
    enum DIALOG_STATE
    {
        DIALOG_STATE_IDLE = 0,

        DIALOG_STATE_TRYING = 1,
        DIALOG_STATE_PROCEEDING = 2,
        DIALOG_STATE_EARLY = 3,
        DIALOG_STATE_CONFIRMED = 4,
        DIALOG_STATE_TERMINATED = 5,

        DIALOG_STATE_ONHOLD = 6,
    };

    // dialog-info element
    IMS_UINT32 m_nInfoVersion;  // mandatory / attribute
    IMS_UINT32 m_eInfoState;    // mandatory / attribute
    AString m_aStrInfoEntity;   // mandatory / attribute

    // dialog element
    AString m_aStrID;         // mandatory / attribute
    AString m_aStrCallID;     // optional / attribute
    AString m_aStrLocalTag;   // optional / attribute
    AString m_aStrRemoteTag;  // optional / attribute
    AString m_aStrDirection;  // optional / attribute

    // dialog_state element
    IMS_UINT32 m_eState;        // element
    AString m_aStrState_Event;  // optional / attribute
    AString m_aStrState_Code;   // optional / attribute

    // dialog_duration element
    IMS_UINT32 m_nDuration;  // element

    // dialog_replaces element
    AString m_aStrReplaces_CallID;     // optional / attribute
    AString m_aStrReplaces_LocalTag;   // optional / attribute
    AString m_aStrReplaces_RemoteTag;  // optional / attribute

    // dialog_referred-by element
    AString m_aStrReferredBy;          // element
    AString m_aStrReferredBy_Display;  // optional / attribute

    // dialog_local element
    DialogLR m_stLocal;
    // dialog_remote element
    DialogLR m_stRemote;

    IMS_BOOL m_bEnablePulled;

protected:
    IMtcApp* m_pApp;
    IMS_SINT32 m_nSlotID;

private:
};

#endif  // UC_DIALOG_H_
