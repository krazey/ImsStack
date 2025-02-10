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

#include "IMessage.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "IuMtcService.h"
#include "MtcDef.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/ISilentRedialHelper.h"
#include "call/MtcCallController.h"
#include "call/SilentRedialHelper.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/IConferenceManager.h"
#include "ect/IEctManager.h"
#include "helper/OperationAsyncRunner.h"
#include "ussi/UssiConstants.h"
#include <memory>

PUBLIC
MtcCallController::MtcCallController(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_objCallManager(objContext.GetCallManager()),
        m_pRedialHelper(IMS_NULL)
{
}

PUBLIC VIRTUAL MtcCallController::~MtcCallController()
{
    m_objContext.ReleaseAsyncOperation(this);
    delete m_pRedialHelper;
}

PUBLIC
CallKey MtcCallController::Open(IN ServiceType eServiceType, IN CallInfo& objCallInfo)
{
    return m_objCallManager.CreateCall(eServiceType, objCallInfo)->GetKey();
}

PUBLIC
void MtcCallController::Attach(IN CallKey nCallKey)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->Attach();
}

PUBLIC
void MtcCallController::Detach(IN CallKey nCallKey)
{
    m_objContext.RunAsyncOperation(this,
            [&, nCallKey]()
            {
                m_objCallManager.RemoveCall(nCallKey);
            });
}

PUBLIC
void MtcCallController::HandleIncoming(IN IMtcService* pService, IN ISession* piSession)
{
    CallInfo objCallInfo;
    if (pService->IsEmergency())
    {
        objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    }
    m_objCallManager.CreateCall(pService->GetServiceType(), objCallInfo)->HandleIncoming(piSession);
}

PUBLIC
void MtcCallController::Start(IN CallKey nCallKey, IN CallType eCallType,
        IN const AString& strTarget, IN MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->Start(
            eCallType, strTarget, objMediaInfo, objSuppServices);
}

PUBLIC
void MtcCallController::HandleUserAlert(IN CallKey nCallKey)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->HandleUserAlert();
}

PUBLIC
void MtcCallController::Accept(
        IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->Accept(eCallType, objMediaInfo);
}

PUBLIC
void MtcCallController::Reject(IN CallKey nCallKey, IN const CallReasonInfo& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->Reject(objReason);
}

PUBLIC
void MtcCallController::Hold(IN CallKey nCallKey, IN MediaInfo& objMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->Hold(objMediaInfo);
}

PUBLIC
void MtcCallController::Resume(IN CallKey nCallKey, IN MediaInfo& objMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->Resume(objMediaInfo);
}

PUBLIC
void MtcCallController::AcceptResume(
        IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->AcceptResume(eCallType, objMediaInfo);
}

PUBLIC
void MtcCallController::RejectResume(IN CallKey nCallKey, IN const CallReasonInfo& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->RejectResume(objReason);
}

PUBLIC
void MtcCallController::Terminate(IN CallKey nCallKey, IN const CallReasonInfo& objReason)
{
    m_objContext.RunAsyncOperation(this,
            [&, nCallKey, objReason]()
            {
                m_objCallManager.GetCallByCallKey(nCallKey)->Terminate(objReason);
            });
}

PUBLIC
void MtcCallController::Update(
        IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->Update(eCallType, objMediaInfo);
}

PUBLIC
void MtcCallController::CancelUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->CancelUpdate(objReason);
}

PUBLIC
void MtcCallController::AcceptUpdate(
        IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->AcceptUpdate(eCallType, objMediaInfo);
}

PUBLIC
void MtcCallController::RejectUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->RejectUpdate(objReason);
}

PUBLIC
void MtcCallController::SendUssd(IN CallKey nCallKey, IN const AString& strUssd)
{
    m_objCallManager.GetCallByCallKey(nCallKey)->SendUssd(strUssd);
}

/*
PUBLIC
void MtcCallController::HandleConference(IN CallKey nCallKey, IN IMS_UINT32 nCmd,
        IN ImsList<ConfUser*>& objUsers)
{
    ConferenceCallProxy::GetInstance()->ProcessCmd(
            m_objContext.GetSlotId(),
            m_objCallManager.GetCallByCallKey(nCallKey),
            objMsg);
}
*/

PUBLIC
void MtcCallController::MergeToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers)
{
    IConferenceController* pController =
            m_objContext.GetConferenceManager().GetController(nCallKey);
    if (pController == IMS_NULL)
    {
        pController = &m_objContext.GetConferenceManager().CreateController(
                nCallKey, ConferenceType::MERGE_CALL);
    }
    pController->ProcessCommand(IConferenceController::MERGE, objUsers);
}

PUBLIC
void MtcCallController::AddToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers)
{
    IConferenceController* pController =
            m_objContext.GetConferenceManager().GetController(nCallKey);
    if (pController == IMS_NULL)
    {
        return;
    }
    pController->ProcessCommand(IConferenceController::ADD, objUsers);
}

PUBLIC
void MtcCallController::RemoveFromConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers)
{
    IConferenceController* pController =
            m_objContext.GetConferenceManager().GetController(nCallKey);
    if (pController == IMS_NULL)
    {
        return;
    }
    pController->ProcessCommand(IConferenceController::REMOVE, objUsers);
}

PUBLIC
void MtcCallController::Transfer(IN CallKey nCallKey, IN const AString& strTarget)
{
    m_objContext.GetEctManager().Transfer(nCallKey, strTarget);
}

PUBLIC
ISilentRedialHelper& MtcCallController::GetRedialHelper(
        IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason)
{
    if (m_pRedialHelper)
    {
        if (m_pRedialHelper->IsSameRedialType(objReason))
        {
            return *m_pRedialHelper;
        }
        // not support different type of redial.
        delete m_pRedialHelper;
    }
    m_pRedialHelper = new SilentRedialHelper(objContext, objReason);
    return *m_pRedialHelper;
}

PUBLIC
void MtcCallController::ReleaseRedialHelper()
{
    delete m_pRedialHelper;
    m_pRedialHelper = IMS_NULL;
}
