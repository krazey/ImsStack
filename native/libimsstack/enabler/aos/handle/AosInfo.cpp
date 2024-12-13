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
#include "ServiceTrace.h"
#include "ServiceNetworkPolicy.h"

#include "ImsMap.h"

#include "ImsAosParameter.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosApplication.h"
#include "interface/IAosBlock.h"
#include "interface/IAosConnection.h"
#include "interface/IAosHandle.h"
#include "interface/IAosRegistration.h"

#include "handle/AosHandle.h"
#include "handle/AosInfo.h"

//__IMS_TRACE_TAG_AOS__;

PUBLIC
AosInfo::AosInfo(IN IAosAppContext* piContext) :
        m_piContext(piContext)
{
}

PUBLIC VIRTUAL AosInfo::~AosInfo() {}

PRIVATE VIRTUAL AString AosInfo::GetAssociatedUri()
{
    AString strAssociatedUri;
    IMS_UINT32 nNa;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_ASSOCIATED_URI, nNa, strAssociatedUri);
    return strAssociatedUri;
}

PRIVATE VIRTUAL IMS_SINT32 AosInfo::GetConnectionType()
{
    return m_piContext->GetConnection()->GetConnectionType();
}

PRIVATE VIRTUAL IMS_UINT32 AosInfo::GetImsFeatures()
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    IMS_UINT32 nFeatures = ImsAosFeature::NONE;

    for (int i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);
        AosHandle* pHandle = DYNAMIC_CAST(AosHandle*, piHandle);

        if (pHandle->IsImsConnected())
        {
            nFeatures |= piHandle->GetBindedFeatureTagList().GetFeatures();
        }
    }

    return nFeatures;
}

PRIVATE VIRTUAL IMS_UINT32 AosInfo::GetImsState()
{
    IMS_UINT32 nAppState = m_piContext->GetApp()->GetAppState();
    IMS_UINT32 nState = IMS_STATE_UNAVAILABLE;

    switch (nAppState)
    {
        case IAosApplication::STATE_NOTREADY:
            if (IsForbiddenBlock())
            {
                nState = IMS_STATE_FORBIDDEN;
            }
            else if (m_piContext->GetBlock()->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            {
                nState = IMS_STATE_UNSUBSCRIBED;
            }
            break;

        case IAosApplication::STATE_READY:  // FALL-THROUGH
        case IAosApplication::STATE_CONNECTING:
            nState = IMS_STATE_PENDING;
            break;

        case IAosApplication::STATE_CONNECTED:  // FALL-THROUGH
        case IAosApplication::STATE_UPDATING:
            nState = IMS_STATE_AVAILABLE;
            break;

        default:
            break;
    }

    return nState;
}

PRIVATE VIRTUAL IMS_SINT32 AosInfo::GetIpcanType()
{
    return m_piContext->GetConnection()->GetIpcanCategory();
}

PRIVATE VIRTUAL AString AosInfo::GetLastPathHeaderValue()
{
    AString strLastPath;
    IMS_UINT32 nNa;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_LAST_PATH, nNa, strLastPath);
    return strLastPath;
}

PRIVATE VIRTUAL AString AosInfo::GetLocalAddress()
{
    AString strLocalAddress;
    IMS_UINT32 nNa;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_LOCAL_ADDRESS, nNa, strLocalAddress);
    return strLocalAddress;
}

PRIVATE VIRTUAL IMS_UINT32 AosInfo::GetLocalPort()
{
    AString strNa;
    IMS_UINT32 nLocalPort;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_LOCAL_PORT, nLocalPort, strNa);
    return nLocalPort;
}

PRIVATE VIRTUAL IMS_UINT32 AosInfo::GetRegisteredNetworkType()
{
    AString strNa;
    IMS_UINT32 nNetworkType;
    m_piContext->GetApp()->GetProperty(
            IAosApplication::PROPERTY_REGISTERED_RAT, nNetworkType, strNa);
    return nNetworkType;
}

PRIVATE VIRTUAL AString AosInfo::GetPathHeaderValue()
{
    AString strPath;
    IMS_UINT32 nNa;
    m_piContext->GetRegistration()->GetProperty(IAosRegistration::PROPERTY_PATH, nNa, strPath);
    return strPath;
}

PRIVATE VIRTUAL AString AosInfo::GetPcscfAddress()
{
    AString strPcscfAddress;
    IMS_UINT32 nNa;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_PCSCF_ADDRESS, nNa, strPcscfAddress);
    return strPcscfAddress;
}

PRIVATE VIRTUAL IMS_UINT32 AosInfo::GetPcscfPort()
{
    AString strNa;
    IMS_UINT32 nPcscfPort;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_PCSCF_PORT, nPcscfPort, strNa);
    return nPcscfPort;
}

PRIVATE VIRTUAL IMS_UINT32 AosInfo::GetRegistrationMode()
{
    IMS_UINT32 nMode = REG_MODE_UNKNOWN;
    switch (m_piContext->GetRegistration()->GetMode())
    {
        case IAosRegistration::MODE_NORMAL:
            nMode = REG_MODE_NORMAL;
            break;

        case IAosRegistration::MODE_LIMITED:
            nMode = REG_MODE_ADMIN;
            break;

        case IAosRegistration::MODE_FAKE:
            if (m_piContext->GetBlock()->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            {
                nMode = REG_MODE_NOUICC;
            }
            else
            {
                nMode = REG_MODE_INTERNAL;
            }
            break;

        default:
            break;
    }

    return nMode;
}

PRIVATE VIRTUAL AString AosInfo::GetSupportedHeaderValue()
{
    AString strSupported;
    IMS_UINT32 nNa;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_SUPPORTED, nNa, strSupported);
    return strSupported;
}

PRIVATE VIRTUAL AString AosInfo::GetServiceRouteHeaderValue()
{
    AString strServiceRoute;
    IMS_UINT32 nNa;
    m_piContext->GetRegistration()->GetProperty(
            IAosRegistration::PROPERTY_SERVICE_ROUTE, nNa, strServiceRoute);
    return strServiceRoute;
}

PRIVATE VIRTUAL void AosInfo::NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized)
{
    if (bIsInitialized)
    {
        m_piContext->GetRegistration()->RequestCmd(IAosRegistration::CMD_ECALL_INIT);
    }
    else
    {
        m_piContext->GetRegistration()->RequestCmd(IAosRegistration::CMD_ECALL_DONE);
    }
}

PRIVATE VIRTUAL void AosInfo::NotifyScbmState(IN IMS_UINT32 nState)
{
    IMS_UINT32 nCommand = 0;

    switch (nState)
    {
        case SCBM_STARTED:
            nCommand = IAosRegistration::CMD_SCBM_STARTED;
            break;
        case SCBM_TERMINATED:
            nCommand = IAosRegistration::CMD_SCBM_TERMINATED;
            break;
        case SCBM_TERMINATED_BY_ECALL:
            nCommand = IAosRegistration::CMD_SCBM_TERMINATED_ECALL;
            break;
        case SCBM_TERMINATED_BY_ESMS:
            nCommand = IAosRegistration::CMD_SCBM_TERMINATED_ESMS;
            break;
        default:
            return;
    }

    m_piContext->GetRegistration()->RequestCmd(nCommand);
}

PRIVATE VIRTUAL void AosInfo::NotifyPublishState(IN IMS_BOOL bIsStarted)
{
    m_piContext->GetApp()->NotifyPublishState(bIsStarted);
}

PRIVATE VIRTUAL void AosInfo::NotifyEmergencySmsState(IN IMS_BOOL bIsInitialized)
{
    if (bIsInitialized)
    {
        m_piContext->GetRegistration()->RequestCmd(IAosRegistration::CMD_ESMS_INIT);
    }
    else
    {
        m_piContext->GetRegistration()->RequestCmd(IAosRegistration::CMD_ESMS_DONE);
    }
}

PRIVATE VIRTUAL void AosInfo::NotifyEpsfbCallState(IN IMS_UINT32 nState)
{
    m_piContext->GetApp()->NotifyEpsFallbackCallState(nState);
}

PRIVATE
IMS_BOOL AosInfo::IsForbiddenBlock()
{
    IAosBlock* piBlock = m_piContext->GetBlock();

    if (piBlock->IsReasonBlocked(BLOCK_IMS_DISABLED) ||
            piBlock->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED) ||
            piBlock->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
