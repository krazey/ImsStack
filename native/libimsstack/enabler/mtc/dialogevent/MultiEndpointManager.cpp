/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "AString.h"
#include "ICarrierConfig.h"
#include "ICoreService.h"
#include "IJniMtcServiceThread.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "JniExternalCall.h"
#include "MtcDef.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/DialogInfoManager.h"
#include "dialogevent/DialogSubscription.h"
#include "dialogevent/IDialogInfoManager.h"
#include "dialogevent/IDialogSubscription.h"
#include "dialogevent/IMultiEndpointManager.h"
#include "dialogevent/MultiEndpointFactory.h"
#include "dialogevent/MultiEndpointManager.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IMtcAosStateListener.h"
#include "registration/SipUrnHelper.h"
#include "sipcore/SipAddress.h"
#include <memory>
#include <utility>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MultiEndpointManager::MultiEndpointManager(
        IN IMtcContext& objContext, IN std::unique_ptr<MultiEndpointFactory> pFactory) :
        m_objContext(objContext),
        m_pFactory(std::move(pFactory)),
        m_piDialogInfoManager(nullptr),
        m_piDialogSubscription(nullptr)
{
    IMS_TRACE_I("+MultiEndpointManager", 0, 0, 0);
    IMtcService* piNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (piNormalService)
    {
        piNormalService->AddAosStateListener(this);
    }

    ICarrierConfig* piCc =
            ConfigService::GetConfigService()->GetCarrierConfig(m_objContext.GetSlotId());
    if (piCc)
    {
        piCc->AddListener(this);
    }
    HandleConditionChanged();
}

PUBLIC
MultiEndpointManager::~MultiEndpointManager()
{
    IMS_TRACE_I("~MultiEndpointManager", 0, 0, 0);

    IMtcService* piNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (piNormalService)
    {
        piNormalService->RemoveAosStateListener(this);
    }

    ICarrierConfig* piCc =
            ConfigService::GetConfigService()->GetCarrierConfig(m_objContext.GetSlotId());
    if (piCc)
    {
        piCc->RemoveListener(this);
    }
}

PUBLIC GLOBAL IMS_BOOL MultiEndpointManager::IsRequired(
        IN const MtcConfigurationProxy& objConfigProxy)
{
    return objConfigProxy.Is(Feature::MULTIENDPOINT_SUPPORTED);
}

VIRTUAL PUBLIC IMultiEndpointManager::PullingDialogInfo MultiEndpointManager::GetDialogInfo(
        IN IMS_UINT32 nId) const
{
    IMultiEndpointManager::PullingDialogInfo objInfo;

    if (!IsRunning())
    {
        return objInfo;
    }

    IMS_TRACE_I("GetDialogInfo ID[%d]", nId, 0, 0);
    for (IMS_UINT32 i = 0; i < m_piDialogInfoManager->GetDialogs().GetSize(); ++i)
    {
        const Dialog* pDialog = m_piDialogInfoManager->GetDialogs().GetAt(i);
        if (pDialog->GetId().GetHashCode() == nId)
        {
            objInfo.strCallId = pDialog->GetCallId();
            objInfo.strLocalTag = pDialog->GetLocalTag();
            objInfo.strRemoteTag = pDialog->GetRemoteTag();
            objInfo.bHeld = IsHeld(*pDialog);
            objInfo.bPullable = IsPullable(*pDialog, objInfo.bHeld);
            objInfo.eCallType = GetCallType(*pDialog);
            objInfo.pMediaInfo = &pDialog->GetExtraInfo().GetMediaInfo();
            break;
        }
    }
    return objInfo;
}

VIRTUAL PUBLIC void MultiEndpointManager::OnAosStateChanged(
        IN IMtcService& /*objMtcService*/, IN MtcAosState /*eState*/, IN IMS_UINT32 /*eAosReason*/)
{
    HandleConditionChanged();
}

VIRTUAL PUBLIC void MultiEndpointManager::OnIpcanChanged(
        IN IMtcService& /*objMtcService*/, IN IMS_UINT32 /*eIpcan*/)
{
    HandleConditionChanged();
}

VIRTUAL PUBLIC void MultiEndpointManager::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    if (m_objContext.GetSlotId() != nSlotId)
    {
        return;
    }

    HandleConditionChanged();
}

VIRTUAL PUBLIC void MultiEndpointManager::OnSubscriptionStarted()
{
    IMS_TRACE_D("OnSubscriptionStarted", 0, 0, 0);
    // In unsubscription case, m_piDialogSubscription must be already deleted when calling
    // Unsubscribe so this cannot be invoked.
}

VIRTUAL PUBLIC void MultiEndpointManager::OnSubscriptionStartFailed()
{
    IMS_TRACE_D("OnSubscriptionStartFailed", 0, 0, 0);
    // If the response code is 403, MEP must be stopped and re-started after an initial
    // registration. Here, not checking the response code because subscription retry is done
    // in DialogSubscription and if a subscription is failed, there is no way to keep MEP on.
    // TODO: timer for Retry-After and call HandleConditionChanged() when the timer expires.
    Stop();
}

VIRTUAL PUBLIC void MultiEndpointManager::OnSubscriptionTerminated()
{
    IMS_TRACE_D("OnSubscriptionTerminated", 0, 0, 0);
    Stop();
    HandleConditionChanged();
}

VIRTUAL PUBLIC void MultiEndpointManager::OnSubscriptionNotified(IN const AString& strBody)
{
    IMS_TRACE_I("OnSubscriptionNotified", 0, 0, 0);
    if (m_piDialogInfoManager->Update(strBody) == IMS_SUCCESS)
    {
        NotifyExternalCalls();
    }
}

PUBLIC
IMS_BOOL MultiEndpointManager::IsRunning() const
{
    // m_piDialogInfoManager and m_piDialogSubscription have same lifetime so checking only one is
    // enough.
    return m_piDialogInfoManager != nullptr;
}

PRIVATE
void MultiEndpointManager::HandleConditionChanged()
{
    IMS_TRACE_D("HandleConditionChanged", 0, 0, 0);
    IMS_BOOL bReady = IsReady();
    if (bReady && !IsRunning())
    {
        Start();
    }
    else if (!bReady && IsRunning())
    {
        Stop();
    }
}

PRIVATE
IMS_BOOL MultiEndpointManager::IsReady() const
{
    IMS_TRACE_D("IsReady", 0, 0, 0);
    // TODO: check config and service activation status.
    if (IsRequired(m_objContext.GetConfigurationProxy()) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    // TODO: need to check AoS behavior.
    return (m_objContext.GetAosConnector(ServiceType::NORMAL)->GetFeatures() &
                   ImsAosFeature::MMTEL) == ImsAosFeature::MMTEL;
}

PRIVATE
void MultiEndpointManager::Start()
{
    IMS_TRACE_D("Start", 0, 0, 0);
    const IMtcService* piService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    const ICoreService* piCoreService = piService ? piService->GetICoreService() : IMS_NULL;
    if (!piCoreService)
    {
        return;
    }

    m_piDialogInfoManager = m_pFactory->CreateDialogInfoManager();
    m_piDialogSubscription = m_pFactory->CreateDialogSubscription(
            m_objContext, *this, piCoreService->GetAuthorizedUserId().ToString());

    if (m_piDialogSubscription->Subscribe() == IMS_FAILURE)
    {
        Stop();
    }
}

PRIVATE
void MultiEndpointManager::Stop()
{
    IMS_TRACE_D("Stop", 0, 0, 0);
    m_piDialogInfoManager.reset();
    m_piDialogSubscription->Unsubscribe();
    m_piDialogSubscription.reset();

    NotifyExternalCalls();
}

PRIVATE
void MultiEndpointManager::NotifyExternalCalls() const
{
    IMS_TRACE_D("NotifyExternalCalls", 0, 0, 0);
    const IMtcService* piService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    IJniMtcServiceThread* pThread = piService ? piService->GetJniServiceThread() : IMS_NULL;
    if (pThread == IMS_NULL)
    {
        return;
    }
    ImsList<const JniExternalCall*> objJniExternalCalls = GetJniExternalCalls();
    pThread->OnExternalCallsChanged(objJniExternalCalls);
    for (IMS_UINT32 i = 0; i < objJniExternalCalls.GetSize(); ++i)
    {
        delete objJniExternalCalls.GetAt(i);
    }
}

PRIVATE
ImsList<const JniExternalCall*> MultiEndpointManager::GetJniExternalCalls() const
{
    ImsList<const JniExternalCall*> objJniExternalCalls;
    if (!m_piDialogInfoManager)
    {
        return objJniExternalCalls;
    }

    for (IMS_UINT32 index = 0; index < m_piDialogInfoManager->GetDialogs().GetSize(); index++)
    {
        const Dialog& objDialog = *m_piDialogInfoManager->GetDialogs().GetAt(index);

        if (IsOwnDialog(objDialog))
        {
            IMS_TRACE_D("GetJniExternalCalls ignores my own dialog", 0, 0, 0);
            // TODO: this could be also done in Telephony Side.
            continue;
        }

        if (IsEarlyState(objDialog))
        {
            IMS_TRACE_D("GetJniExternalCalls ignores early states [%d]",
                    objDialog.GetState().GetState(), 0, 0);
            continue;
        }

        JniExternalCall* pJniExternalCall = new JniExternalCall();

        pJniExternalCall->m_strCallId.SetNumber(objDialog.GetId().GetHashCode());
        pJniExternalCall->m_strAddress = objDialog.GetRemoteParticipant().GetIdentity().GetUri();
        pJniExternalCall->m_strLocalAddress =
                objDialog.GetLocalParticipant().GetIdentity().GetUri();
        pJniExternalCall->m_bIsHeld = IsHeld(objDialog);
        pJniExternalCall->m_bIsPullable = IsPullable(objDialog, pJniExternalCall->m_bIsHeld);
        pJniExternalCall->m_nCallState =
                objDialog.GetState().GetState() == Dialog::State::STATE_CONFIRMED
                ? JniExternalCall::CALL_STATE_CONFIRMED
                : JniExternalCall::CALL_STATE_TERMINATED;
        pJniExternalCall->m_nCallType = static_cast<IMS_UINT32>(GetCallType(objDialog));

        IMS_TRACE_D("GetJniExternalCalls >> %s", pJniExternalCall->ToString().GetStr(), 0, 0);

        objJniExternalCalls.Append(pJniExternalCall);
    }

    return objJniExternalCalls;
}

PRIVATE
IMS_BOOL MultiEndpointManager::IsPullable(const Dialog& objDialog, IN IMS_BOOL bHeld) const
{
    if (bHeld)
    {
        return IMS_FALSE;
    }

    if (objDialog.GetState().GetState() != Dialog::State::STATE_CONFIRMED)
    {
        return IMS_FALSE;
    }

    const Dialog::ExtraInfo& objExtraInfo = objDialog.GetExtraInfo();
    if (objExtraInfo.GetExclusive().EqualsIgnoreCase("true"))
    {
        return IMS_FALSE;
    }

    if (objExtraInfo.GetMediaInfo().eVideoQuality != VIDEO_QUALITY_NONE &&
            objExtraInfo.GetMediaInfo().eVideoQuality != VIDEO_QUALITY_NOTUSED)
    {
        if (objExtraInfo.GetMediaInfo().eVideoDirection == DIRECTION_INVALID ||
                objExtraInfo.GetMediaInfo().eVideoDirection == DIRECTION_INACTIVE)
        {
            // video is used but direction is inactive
            return IMS_FALSE;
        }
    }

    if (objExtraInfo.GetMediaInfo().eAudioDirection != DIRECTION_SEND_RECEIVE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
CallType MultiEndpointManager::GetCallType(const Dialog& objDialog) const
{
    if (objDialog.GetExtraInfo().GetMediaInfo().eAudioQuality == AUDIO_QUALITY_AMR_WB)
    {
        // VIDEO_QUALITY_NOTUSED is downgraded VT.
        return objDialog.GetExtraInfo().GetMediaInfo().eVideoQuality != VIDEO_QUALITY_NONE
                ? CallType::VT
                : CallType::VOIP;
    }

    return CallType::UNKNOWN;
}

PRIVATE
IMS_BOOL MultiEndpointManager::IsHeld(const Dialog& objDialog) const
{
    AString strSipRendering("+sip.rendering");
    IMS_SLONG nIndex =
            objDialog.GetLocalParticipant().GetTarget().GetParams().GetIndexOfKey(strSipRendering);

    if (nIndex >= 0)
    {
        return objDialog.GetLocalParticipant()
                .GetTarget()
                .GetParams()
                .GetValueAt(nIndex)
                .EqualsIgnoreCase("no");
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MultiEndpointManager::IsOwnDialog(const Dialog& objDialog) const
{
    ImsList<AString> objTokens = objDialog.GetLocalParticipant().GetIdentity().GetUri().Split(';');
    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); i++)
    {
        AString strToken = objTokens.GetAt(i);

        if (strToken.StartsWith("gr=urn:gsma:imei:"))
        {
            // Removing "gr=" (3 characters)
            strToken = strToken.GetSubStr(3, strToken.GetLength() - 3);
            return SipUrnHelper::GetUrn(
                    m_objContext.GetSlotId(), SipUrnHelper::GSMA_IMEI, IMS_FALSE)
                    .Equals(strToken);
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MultiEndpointManager::IsEarlyState(const Dialog& objDialog) const
{
    switch (objDialog.GetState().GetState())
    {
        case Dialog::State::STATE_IDLE:
        case Dialog::State::STATE_TRYING:
        case Dialog::State::STATE_PROCEEDING:
        case Dialog::State::STATE_EARLY:
            return IMS_TRUE;
        default:
            return IMS_FALSE;
    }
}
