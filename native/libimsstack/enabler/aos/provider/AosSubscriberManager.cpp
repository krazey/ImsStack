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
#include "ServiceTimer.h"
#include "Engine.h"
#include "IAosService.h"
#include "IConfigurable.h"
#include "IConfiguration.h"
#include "ISubscriberConfig.h"
#include "SipAddress.h"
#include "ImsIdentity.h"
#include "IRegistration.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosUtil.h"
#include "provider/AosSubscriberManager.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosRegStateManager.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosSubscriberListener.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG  m_strTag.GetStr()
#define ID_FAKE "fake"

PUBLIC
AosSubscriberManager::AosSubscriberManager(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_objListeners(ImsList<IAosSubscriberManagerListener*>()),
        m_objMonitorListeners(ImsList<IAosSubscriberManagerListener*>()),
        m_piSubscriberConfig(IMS_NULL),
        m_piSubscriberConfigFake(IMS_NULL),
        m_bIsim(IMS_FALSE),
        m_bUsim(IMS_FALSE),
        m_bUsimFallback(IMS_FALSE),
        m_bIsRefreshStarted(IMS_FALSE),
        m_piTimerToIccLoadedWaiting(IMS_NULL),
        m_piTimerToPhoneRestartRecovery(IMS_NULL),
        m_bIsProvisioned(IMS_FALSE),
        m_bIsProvisionedForFake(IMS_FALSE),
        m_nNotifyState(IAosSubscriber::NOT_READY),
        m_nNotifyStateForFake(IAosSubscriber::NOT_READY),
        m_eNotifyIsimState(AosIsimState::INVALID),
        m_piNConfig(IMS_NULL),
        m_nIsimIndexForImpu(DEFAULT_ISIM_INDEX_FOR_IMPU),
        m_bSupportLimitedAdminSmsMode(IMS_FALSE),
        m_objImsIdentityPriority(ImsVector<IMS_SINT32>())
{
    m_strTag.Sprintf("%d", m_nSlotId);
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosSubscriberManager = %" PFLS_u "/%" PFLS_x,
            m_nSlotId, sizeof(AosSubscriberManager), this);
    Init();
}

PUBLIC VIRTUAL AosSubscriberManager::~AosSubscriberManager()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosSubscriberManager = %" PFLS_u "/%" PFLS_x,
            m_nSlotId, sizeof(AosSubscriberManager), this);

    CleanUp();
}

PUBLIC
IMS_BOOL AosSubscriberManager::IsReady(IN IMS_BOOL bIsFake /*= IMS_FALSE*/) const
{
    return (bIsFake) ? m_bIsProvisionedForFake : m_bIsProvisioned;
}

PUBLIC
IMS_BOOL AosSubscriberManager::IsIsim() const
{
    return m_bIsim;
}

PUBLIC
IMS_BOOL AosSubscriberManager::IsUsim() const
{
    return m_bUsim;
}

PUBLIC
void AosSubscriberManager::AddListener(IN IAosSubscriberManagerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const IAosSubscriberManagerListener* pTempListener = m_objListeners.GetAt(i);

        if (pTempListener == piListener)
        {
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC
void AosSubscriberManager::RemoveListener(IN IAosSubscriberManagerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const IAosSubscriberManagerListener* pTempListener = m_objListeners.GetAt(i);

        if (pTempListener == piListener)
        {
            m_objListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
void AosSubscriberManager::AddListenerForMonitor(IN IAosSubscriberManagerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objMonitorListeners.GetSize(); ++i)
    {
        const IAosSubscriberManagerListener* pTempListener = m_objMonitorListeners.GetAt(i);

        if (pTempListener == piListener)
        {
            return;
        }
    }

    m_objMonitorListeners.Append(piListener);
}

PUBLIC
void AosSubscriberManager::RemoveListenerForMonitor(IN IAosSubscriberManagerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objMonitorListeners.GetSize(); ++i)
    {
        const IAosSubscriberManagerListener* pTempListener = m_objMonitorListeners.GetAt(i);

        if (pTempListener == piListener)
        {
            m_objMonitorListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
const AStringArray& AosSubscriberManager::GetConfiguredImpus() const
{
    return m_objPuids;
}

PUBLIC
const AStringArray& AosSubscriberManager::GetConfiguredImpusForFake() const
{
    return m_objPuidsForFake;
}

PUBLIC
const AStringArray& AosSubscriberManager::GetFakeImpus() const
{
    return (m_piSubscriberConfigFake != IMS_NULL) ? m_piSubscriberConfigFake->GetPublicUserIds()
                                                  : AStringArray::ConstNull();
}

PUBLIC
const ISubscriberConfig* AosSubscriberManager::GetSubscriberConfig(
        IN IMS_SINT32 nType /*= IAosSubscriber::NORMAL*/) const
{
    return GetSubscriberConfiguration(nType);
}

PROTECTED
void AosSubscriberManager::Init()
{
    A_IMS_TRACE_I(AOSTAG, "Init", 0, 0, 0);

    m_piNConfig = GET_N_CONFIG(m_nSlotId);
    if (m_piNConfig != IMS_NULL)
    {
        m_piNConfig->SetListener(this);
        UpdateNConfiguration();
    }

    if (m_piSubscriberConfig == IMS_NULL)
    {
        m_piSubscriberConfig = GetSubscriberConfiguration();
    }

    if (m_piSubscriberConfig != IMS_NULL)
    {
        m_piSubscriberConfig->SetListener(this, ISubscriberConfig::LISTEN_EVENT_ISIM_PROVISIONING);
    }

    if (m_piSubscriberConfigFake == IMS_NULL)
    {
        m_piSubscriberConfigFake = GetSubscriberConfiguration(IAosSubscriber::FAKE);
    }

    UpdateImsIdentity(GetIdentity(Index::FIRST));

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        piService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }

    if (UpdateImsi())
    {
        RemoveImpu();
    }

    ConfigureAsDefault();
    ConfigureAsFake();
}

PROTECTED
void AosSubscriberManager::Restart()
{
    A_IMS_TRACE_I(AOSTAG, "Restart", 0, 0, 0);
    ClearAll();
    RequestStop();
    NotifyState(IAosSubscriber::NOT_READY);
    ConfigureAsDefault();
}

PROTECTED
void AosSubscriberManager::CleanUp()
{
    A_IMS_TRACE_I(AOSTAG, "CleanUp", 0, 0, 0);

    if (m_piNConfig != IMS_NULL)
    {
        m_piNConfig->RemoveListener(this);
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        piService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }

    m_piSubscriberConfigFake = IMS_NULL;
    if (m_piSubscriberConfig != IMS_NULL)
    {
        m_piSubscriberConfig->RemoveListener(this);
        m_piSubscriberConfig = IMS_NULL;
    }

    m_objMonitorListeners.Clear();
    m_objListeners.Clear();

    ClearTimers();
}

PROTECTED
void AosSubscriberManager::SetIsim(IN IMS_BOOL bOn)
{
    m_bIsim = bOn;
}

PROTECTED
void AosSubscriberManager::SetUsim(IN IMS_BOOL bOn)
{
    m_bUsim = bOn;
}

PROTECTED
void AosSubscriberManager::SetProvisioned(
        IN IMS_BOOL bProvision, IN IMS_UINT32 nType /*= IAosSubscriber::NORMAL*/)
{
    if (nType == IAosSubscriber::FAKE)
    {
        m_bIsProvisionedForFake = bProvision;
    }
    else
    {
        m_bIsProvisioned = bProvision;
    }
}

PROTECTED
void AosSubscriberManager::ClearAll()
{
    m_objPuids.RemoveAllElements();
    SetProvisioned(IMS_FALSE);
}

PROTECTED
IMS_BOOL AosSubscriberManager::IsProvisioned(IN IMS_BOOL bIsFake /*= IMS_FALSE*/) const
{
    IMS_BOOL bResult = (bIsFake) ? m_bIsProvisionedForFake : m_bIsProvisioned;

    A_IMS_TRACE_I(
            AOSTAG, "IsProvisioned(%s) :: %s", (bIsFake) ? "Fake" : "", _TRACE_B_(bResult), 0);

    return bResult;
}

PROTECTED
IMS_BOOL AosSubscriberManager::IsRefreshStarted() const
{
    return m_bIsRefreshStarted;
}

PROTECTED
IMS_BOOL AosSubscriberManager::IsTimerRunning(IN IMS_UINT32 nType) const
{
    switch (nType)
    {
        case TIMER_ICC_LOADED_WAITING:
            return (m_piTimerToIccLoadedWaiting != IMS_NULL);

        case TIMER_PHONE_RESTART_RECOVERY:
            return (m_piTimerToPhoneRestartRecovery != IMS_NULL);

        default:
            return IMS_FALSE;
    }
}

PROTECTED
IMS_BOOL AosSubscriberManager::IsSupportFallback(IN IMS_UINT32 nIdentity) const
{
    return (GetIdentity(Index::SECOND) == nIdentity);
}

PROTECTED
IMS_UINT32 AosSubscriberManager::GetIsimAt() const
{
    return m_nIsimIndexForImpu;
}

PROTECTED
void AosSubscriberManager::ConfigureAsDefault()
{
    A_IMS_TRACE_I(AOSTAG, "ConfigureAsDefault", 0, 0, 0);
    if (m_piSubscriberConfig == IMS_NULL)
    {
        A_IMS_TRACE_I(AOSTAG, "ConfigureAsDefault :: SubscriberConfig is null", 0, 0, 0);
        return;
    }

    if (IsIsim())
    {
        if (!m_piSubscriberConfig->IsProvisioningDone())
        {
            A_IMS_TRACE_I(AOSTAG, "ConfigureAsDefault :: Provisioning is not done", 0, 0, 0);
            return;
        }

        if (!UpdateImpuFromIsim(m_objPuids))
        {
            return;
        }
    }
    else if (IsUsim())
    {
        if (!UpdateSubscriberInfoWithTempImpu(m_objPuids))
        {
            return;
        }
    }
    else
    {
        // if ISIM & USIM is false, IMPU is configured from CONF
        IMS_SINT32 nIndex = m_piSubscriberConfig->GetIndexOfPrimaryPublicUserId();
        AStringArray objPublicUserIds = m_piSubscriberConfig->GetPublicUserIds();
        AString strImpu;
        if (objPublicUserIds.GetCount() > nIndex)
        {
            strImpu = m_piSubscriberConfig->GetPublicUserIds().GetElementAt(nIndex);
        }

        if (strImpu.GetLength() == 0)
        {
            A_IMS_TRACE_I(AOSTAG, "ConfigureAsDefault :: Getting IMPU has failed", 0, 0, 0);
            return;
        }

        m_objPuids.AddElement(strImpu);

        for (int i = 0; i < objPublicUserIds.GetCount(); i++)
        {
            if (i != nIndex && objPublicUserIds.GetElementAt(i).GetLength() > 0)
            {
                m_objPuids.AddElement(objPublicUserIds.GetElementAt(i));
            }
        }
    }

    if (m_objPuids.GetCount() > 0)
    {
        SetProvisioned(IMS_TRUE);

        A_IMS_TRACE_I(AOSTAG, "ConfigureAsDefault :: primary IMPU(%s) is provisioned",
                m_objPuids.GetElementAt(0).GetStr(), 0, 0);

        NotifyState(IAosSubscriber::READY);
    }
}

PROTECTED
void AosSubscriberManager::ConfigureAsFake()
{
    if (m_piSubscriberConfigFake == IMS_NULL)
    {
        return;
    }

    const AStringArray& objPublicUserIds = m_piSubscriberConfigFake->GetPublicUserIds();

    if (objPublicUserIds.IsEmpty())
    {
        A_IMS_TRACE_I(AOSTAG, "ConfigureAsFake :: PUIDs are empty", 0, 0, 0);
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "ConfigureAsFake", 0, 0, 0);

    const AString strImpu = objPublicUserIds.GetElementAt(0);
    if (strImpu.GetLength() == 0)
    {
        A_IMS_TRACE_I(AOSTAG, "ConfigureAsFake :: IMPU is failed", 0, 0, 0);
        return;
    }

    m_objPuidsForFake.AddElement(strImpu);
    SetProvisioned(IMS_TRUE, IAosSubscriber::FAKE);

    A_IMS_TRACE_I(AOSTAG, "ConfigureAsFake :: IMPU(%s) is provisioned", strImpu.GetStr(), 0, 0);
    NotifyMonitorState(IAosSubscriber::READY);
}

PROTECTED
IMS_BOOL AosSubscriberManager::CheckIsimValues()
{
    if (!m_piSubscriberConfig->IsIsimSupported())
    {
        A_IMS_TRACE_I(AOSTAG, "CheckIsimValues :: ISIM is not supported", 0, 0, 0);
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(AOSTAG, "CheckIsimValues", 0, 0, 0);

    const AStringArray& objImpus = m_piSubscriberConfig->GetPublicUserIds();
    const AString& strImpi = m_piSubscriberConfig->GetPrivateUserId();
    const AString& strHomeDomainName = m_piSubscriberConfig->GetHomeDomainName();

    const IMS_BOOL bSupportImsiFallack =
            IsSupportFallback(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);
    IMS_BOOL bUseTempValues = IMS_FALSE;

    if (objImpus.IsEmpty())
    {
        A_IMS_TRACE_I(AOSTAG, "CheckIsimValues :: IMPU is empty", 0, 0, 0);
        if (bSupportImsiFallack)
        {
            bUseTempValues = IMS_TRUE;
        }
        else
        {
            return IMS_FALSE;
        }
    }
    else
    {
        SipAddress objSipAddress;
        IMS_BOOL bImpuValid = IMS_FALSE;

        for (IMS_SINT32 nAt = 0; nAt < objImpus.GetCount(); ++nAt)
        {
            const AString& strImpu = objImpus.GetElementAt(nAt);

            if (strImpu.GetLength() > 0)
            {
                if (objSipAddress.Create(strImpu))
                {
                    if (objSipAddress.IsSchemeSip() || objSipAddress.IsSchemeSips() ||
                            objSipAddress.IsSchemeTel())
                    {
                        bImpuValid = IMS_TRUE;
                        break;
                    }
                }
            }
        }

        if (!bImpuValid)
        {
            A_IMS_TRACE_I(AOSTAG, "CheckIsimValues :: IMPU is invalid", 0, 0, 0);
            if (bSupportImsiFallack)
            {
                bUseTempValues = IMS_TRUE;
            }
            else
            {
                return IMS_FALSE;
            }
        }
    }

    if (strImpi.GetLength() == 0)
    {
        A_IMS_TRACE_I(AOSTAG, "CheckIsimValues :: IMPI is invalid", 0, 0, 0);
        if (bSupportImsiFallack)
        {
            bUseTempValues = IMS_TRUE;
        }
        else
        {
            return IMS_FALSE;
        }
    }

    if (strHomeDomainName.GetLength() == 0)
    {
        A_IMS_TRACE_I(AOSTAG, "CheckIsimValues :: HomeDomainName is invalid", 0, 0, 0);
        if (bSupportImsiFallack)
        {
            bUseTempValues = IMS_TRUE;
        }
        else
        {
            return IMS_FALSE;
        }
    }

    if (bUseTempValues)
    {
        A_IMS_TRACE_I(AOSTAG, "CheckIsimValues :: Support ISIM_IMSI fallback", 0, 0, 0);
        if (!UpdateSubscriberInfoWithTempImpu(m_objPuids, IMS_TRUE))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED
ISubscriberConfig* AosSubscriberManager::GetSubscriberConfiguration(
        IN IMS_SINT32 nType /*= IAosSubscriber::NORMAL*/) const
{
    return Engine::GetConfiguration()->GetSubscriberConfig(
            m_nSlotId, (nType == IAosSubscriber::FAKE) ? ID_FAKE : AString::ConstNull());
}

PROTECTED
IMS_BOOL AosSubscriberManager::UpdateImpuFromIsim(OUT AStringArray& objImpus)
{
    AStringArray objValidImpus;

    for (IMS_SINT32 nAt = 0; nAt < m_piSubscriberConfig->GetPublicUserIds().GetCount(); ++nAt)
    {
        const AString& strTemp = m_piSubscriberConfig->GetPublicUserIds().GetElementAt(nAt);
        if (strTemp.GetLength() != 0)
        {
            objValidImpus.AddElement(strTemp);
        }
    }

    A_IMS_TRACE_I(AOSTAG, "UpdateImpuFromIsim :: total size (%d), valid size (%d)",
            m_piSubscriberConfig->GetPublicUserIds().GetCount(), objValidImpus.GetCount(), 0);

    if (objValidImpus.GetCount() == 0)
    {
        A_IMS_TRACE_I(AOSTAG, "UpdateImpuFromIsim :: No valid IMPU", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_bSupportLimitedAdminSmsMode)
    {
        if (objValidImpus.GetCount() == 1)
        {
            objImpus.AddElement(objValidImpus.GetElementAt(0));
            return IMS_TRUE;
        }

        if (!IsPrimaryImpuValid(objValidImpus))
        {
            objImpus.AddElement(objValidImpus.GetElementAt(0));
            return IMS_TRUE;
        }

        if (IsSipUri(objValidImpus.GetElementAt(1)))
        {
            objImpus.AddElement(objValidImpus.GetElementAt(1));
            objImpus.AddElement(objValidImpus.GetElementAt(0));
            return IMS_TRUE;
        }

        for (IMS_SINT32 nAt = 0; nAt < objValidImpus.GetCount(); ++nAt)
        {
            if (IsSipUri(objValidImpus.GetElementAt(nAt)))
            {
                objImpus.AddElement(objValidImpus.GetElementAt(nAt));
                objImpus.AddElement(objValidImpus.GetElementAt(nAt));
                break;
            }
        }

        return (objImpus.GetCount() > 0) ? IMS_TRUE : IMS_FALSE;
    }

    AString strImpu = m_piSubscriberConfig->GetPublicUserIds().GetElementAt(
            (objValidImpus.GetCount() == 1) ? 0 : GetIsimAt());

    if (strImpu.GetLength() == 0)
    {
        if (objValidImpus.GetElementAt(0).GetLength() == 0)
        {
            return IMS_FALSE;
        }
        else
        {
            strImpu = objValidImpus.GetElementAt(0);
        }
    }

    objImpus.AddElement(strImpu);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosSubscriberManager::UpdateSubscriberInfoWithTempImpu(
        OUT AStringArray& objImpus, IN IMS_BOOL bIsIsim /*= IMS_FALSE*/)
{
    AString strImpu(GetTemporaryPublicUserId());
    if (strImpu.GetLength() == 0)
    {
        A_IMS_TRACE_I(AOSTAG,
                "UpdateSubscriberInfoWithTempImpu :: CreateTemporaryPublicUserId fails", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strImpi(GetTemporaryPrivateUserId());
    if (strImpi.GetLength() == 0)
    {
        A_IMS_TRACE_I(AOSTAG,
                "UpdateSubscriberInfoWithTempImpu :: CreateTemporaryPrivateUserId fails", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strHdn(GetTemporaryHomeDomainName());
    if (strHdn.GetLength() == 0)
    {
        A_IMS_TRACE_I(AOSTAG,
                "UpdateSubscriberInfoWithTempImpu :: CreateTemporaryHomeDomainName fails", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piSubscriberConfig == IMS_NULL)
    {
        A_IMS_TRACE_I(
                AOSTAG, "UpdateSubscriberInfoWithTempImpu :: SubscriberConfig is null", 0, 0, 0);
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(AOSTAG, "UpdateSubscriberInfoWithTempImpu :: Update SubscriberInfo with %s",
            (bIsIsim) ? "ISIM" : "USIM", 0, 0);
    m_piSubscriberConfig->UpdateSubscriberInfo(strHdn, strImpi, strImpu, bIsIsim);

    objImpus.AddElement(strImpu);
    return IMS_TRUE;
}

PROTECTED
void AosSubscriberManager::RemoveImpu() const
{
    PhoneInfoService* pPhoneInfoService = PhoneInfoService::GetPhoneInfoService();
    ISubscriberInfo* piSubsInfo = (pPhoneInfoService != IMS_NULL)
            ? pPhoneInfoService->GetSubscriberInfo(m_nSlotId)
            : IMS_NULL;
    if (piSubsInfo != null)
    {
        piSubsInfo->SetPreference("impu_list", "size", "0");
        A_IMS_TRACE_I(AOSTAG, "RemoveImpu :: The recorded IMPUs have been removed.", 0, 0, 0);
    }
}

PROTECTED
IMS_BOOL AosSubscriberManager::UpdateImsi() const
{
    IMS_BOOL bIsUpdated = IMS_FALSE;

    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(m_nSlotId);
    if (piSubsInfo == null)
    {
        return bIsUpdated;
    }

    AString strImsi;
    piSubsInfo->GetSubscriberId(strImsi);

    AString strImsiRecorded;
    piSubsInfo->GetPreference("impu_list", "imsi", strImsiRecorded);

    if (strImsi.GetLength() == 0 || !strImsi.Equals(strImsiRecorded))
    {
        piSubsInfo->SetPreference("impu_list", "imsi", strImsi);
        A_IMS_TRACE_I(AOSTAG, "UpdateImsi :: IMSI is initialized.", 0, 0, 0);
        bIsUpdated = IMS_TRUE;
    }

    return bIsUpdated;
}

PROTECTED
void AosSubscriberManager::UpdateImsIdentity(IN IMS_UINT32 nIdentity)
{
    switch (nIdentity)
    {
        case CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM:
            SetIsim(IMS_TRUE);
            SetUsim(IMS_FALSE);
            break;

        case CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM:
            SetIsim(IMS_FALSE);
            SetUsim(IMS_TRUE);
            break;

        default:
            SetIsim(IMS_FALSE);
            SetUsim(IMS_FALSE);
            break;
    }

    if (IsIsim() && m_piSubscriberConfig != IMS_NULL && !m_piSubscriberConfig->IsIsimSupported())
    {
        A_IMS_TRACE_I(AOSTAG, "UpdateImsIdentity :: Enable ISIM", 0, 0, 0);
        m_piSubscriberConfig->EnableIsim();
    }

    A_IMS_TRACE_I(AOSTAG, "UpdateImsIdentity :: ISIM(%s),USIM(%s)", _TRACE_B_(IsIsim()),
            _TRACE_B_(IsUsim()), 0);
}

PROTECTED
IMS_UINT32 AosSubscriberManager::GetIdentity(IN Index eIndex) const
{
    if (m_objImsIdentityPriority.IsEmpty() ||
            (m_objImsIdentityPriority.GetSize() <= static_cast<IMS_UINT32>(eIndex)))
    {
        return CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF;
    }

    switch (eIndex)
    {
        case Index::FIRST:
            return m_objImsIdentityPriority.GetAt(0);

        case Index::SECOND:
            return m_objImsIdentityPriority.GetAt(1);

        default:
            return CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF;
    }
}

PROTECTED
IMS_BOOL AosSubscriberManager::ReconfigureFallback(IN IMS_BOOL bToUsim)
{
    A_IMS_TRACE_I(AOSTAG, "ReconfigureFallback :: Dir(%s), USIM fallback %s ",
            (bToUsim) ? "USIM" : "ISIM",
            (m_bUsimFallback) ? "already in progress" : "not in progress", 0);

    if (!bToUsim && !m_bUsimFallback)
    {
        A_IMS_TRACE_I(AOSTAG, "ReconfigureFallback :: No fallback needed", 0, 0, 0);
        return IMS_FALSE;
    }

    UpdateImsIdentity(bToUsim ? CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM
                              : CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);

    m_bUsimFallback = bToUsim;

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosSubscriberManager::ProcessPhoneNumberAvailable()
{
    if (IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY))
    {
        A_IMS_TRACE_I(AOSTAG,
                "ProcessPhoneNumberAvailable :: Phone restart recovery is in progress", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!IsUsim())
    {
        A_IMS_TRACE_I(AOSTAG, "ProcessPhoneNumberAvailable :: USIM is disabled", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!IsReady())
    {
        A_IMS_TRACE_I(AOSTAG, "ProcessPhoneNumberAvailable : State is not ready", 0, 0, 0);
        Restart();
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(AOSTAG, "ProcessPhoneNumberAvailable", 0, 0, 0);

    const AStringArray& objImpus = GetConfiguredImpus();

    if (objImpus.GetCount() > 0)
    {
        AString strImpu = objImpus.GetElementAt(0);

        if (strImpu.GetLength() == 0)
        {
            return IMS_FALSE;
        }

        A_IMS_TRACE_I(AOSTAG, "ProcessPhoneNumberAvailable :: primary IMPU(%s) is provisioned",
                strImpu.GetStr(), 0, 0);

        AString strTemporaryImpu = ImsIdentity::CreateTemporaryPublicUserId(m_nSlotId);

        if (strTemporaryImpu.GetLength() == 0)
        {
            return IMS_FALSE;
        }

        A_IMS_TRACE_I(AOSTAG, "ProcessPhoneNumberAvailable :: temp IMPU(%s) is provisioned",
                strTemporaryImpu.GetStr(), 0, 0);

        if (strImpu.EqualsIgnoreCase(strTemporaryImpu))
        {
            A_IMS_TRACE_I(AOSTAG, "ProcessPhoneNumberAvailable :: temp IMPU unchanged", 0, 0, 0);
            return IMS_FALSE;
        }

        A_IMS_TRACE_I(AOSTAG, "ProcessPhoneNumberAvailable :: USIM refresh processed", 0, 0, 0);
        RemoveImpu();
        NotifyState(IAosSubscriber::REFRESH_STARTED);
        Restart();
        UpdateImsi();

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosSubscriberManager::ProcessIsimStateChange(IN IsimState eState)
{
    if (eState == IsimState::LOADED || eState == IsimState::REFRESH_COMPLETED)
    {
        ReconfigureFallback(IMS_FALSE);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

// Currently Not used
PROTECTED
void AosSubscriberManager::ProcessPhoneRestarted()
{
    A_IMS_TRACE_I(AOSTAG, "ProcessPhoneRestarted :: phone is restarted", 0, 0, 0);
    StartTimer(TIMER_PHONE_RESTART_RECOVERY, PHONE_RESTART_RECOVERY_INTERVAL);
}

PROTECTED
void AosSubscriberManager::ProcessIccLoadedWaitingTimerExpired()
{
    StopTimer(TIMER_ICC_LOADED_WAITING);
}

PROTECTED
void AosSubscriberManager::ProcessPhoneRestartRecoveryTimerExpired()
{
    StopTimer(TIMER_PHONE_RESTART_RECOVERY);

    if (IsUsim())
    {
        ProcessPhoneNumberAvailable();
        return;
    }

    if (!IsReady() || IsRefreshStarted())
    {
        if (CheckAndTryUsimFallback())
        {
            return;
        }

        if (CheckAndTryIsimImsiFallback())
        {
            return;
        }
    }
}

void AosSubscriberManager::ProcessValidIsimOnCompleted(IN IMS_BOOL bIsRefresh)
{
    A_IMS_TRACE_I(
            AOSTAG, "ProcessValidIsimOnCompleted :: %s", (bIsRefresh) ? "Refresh" : "Init", 0, 0);

    StopTimer(TIMER_PHONE_RESTART_RECOVERY);
    Restart();

    IMS_SINT32 nSubscriberState = (IsProvisioned())
            ? ((bIsRefresh) ? IAosSubscriber::REFRESH_COMPLETED : IAosSubscriber::READY)
            : IAosSubscriber::NOT_READY;

    NotifyState(nSubscriberState);
    NotifyAosIsimState((bIsRefresh) ? AosIsimState::REFRESH_COMPLETE : AosIsimState::VALID);
}

PROTECTED
void AosSubscriberManager::ProcessInvalidIsimOnCompleted(IN IMS_BOOL bIsRefresh)
{
    A_IMS_TRACE_I(
            AOSTAG, "ProcessInvalidIsimOnCompleted :: %s", (bIsRefresh) ? "Refresh" : "Init", 0, 0);

    if (!CheckAndTryUsimFallback())
    {
        ClearAll();
        NotifyState((bIsRefresh) ? IAosSubscriber::REFRESH_FAILED : IAosSubscriber::NOT_READY);

        A_IMS_TRACE_I(AOSTAG, "ProcessInvalidIsimOnCompleted :: No further attempts", 0, 0, 0);
    }

    NotifyAosIsimState(AosIsimState::INVALID);
}

PROTECTED
IMS_BOOL AosSubscriberManager::CheckAndTryUsimFallback()
{
    if (!IsSupportFallback(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM))
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(AOSTAG, "CheckAndTryUsimFallback", 0, 0, 0);

    if (ReconfigureFallback(IMS_TRUE))
    {
        Restart();
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosSubscriberManager::CheckAndTryIsimImsiFallback()
{
    if (!IsSupportFallback(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI))
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(AOSTAG, "CheckAndTryIsimImsiFallback", 0, 0, 0);
    if (CheckIsimValues())
    {
        A_IMS_TRACE_I(AOSTAG, "CheckAndTryIsimImsiFallback :: ISIM values are valid", 0, 0, 0);

        Restart();

        NotifyState((IsProvisioned()) ? IAosSubscriber::READY : IAosSubscriber::NOT_READY);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosSubscriberManager::UpdateNConfiguration()
{
    IMS_BOOL bIsUpdated = IMS_FALSE;

    A_IMS_TRACE_I(AOSTAG,
            "UpdateNConfiguration :: Current - ImsIdentityPriority(%s), IsimIndexForImpu(%d), "
            "SupportlimitedAdminSmsMode(%s)",
            IdentityPriorityToString(), m_nIsimIndexForImpu,
            _TRACE_B_(m_bSupportLimitedAdminSmsMode));

    if (m_piNConfig == IMS_NULL)
    {
        A_IMS_TRACE_I(
                AOSTAG, "UpdateNConfiguration :: Not Updated - NConfiguration is null", 0, 0, 0);
        return bIsUpdated;
    }

    IMS_UINT32 nIsimIndexForImpu = m_piNConfig->GetIsimIndexForImpu();
    IMS_BOOL bSupportLimitedAdminSmsMode = m_piNConfig->IsSupportLimitedAdminSmsMode();
    ImsVector<IMS_SINT32> objImsIdentityPriority = m_piNConfig->GetImsIdentityPriority();

    if (m_nIsimIndexForImpu != nIsimIndexForImpu)
    {
        m_nIsimIndexForImpu = nIsimIndexForImpu;
        bIsUpdated = IMS_TRUE;
    }

    if (m_bSupportLimitedAdminSmsMode != bSupportLimitedAdminSmsMode)
    {
        m_bSupportLimitedAdminSmsMode = bSupportLimitedAdminSmsMode;
        bIsUpdated = IMS_TRUE;
    }

    if (m_objImsIdentityPriority.GetSize() != objImsIdentityPriority.GetSize())
    {
        m_objImsIdentityPriority = objImsIdentityPriority;
        bIsUpdated = IMS_TRUE;
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objImsIdentityPriority.GetSize(); ++i)
        {
            if (m_objImsIdentityPriority.GetAt(i) != objImsIdentityPriority.GetAt(i))
            {
                m_objImsIdentityPriority = objImsIdentityPriority;
                bIsUpdated = IMS_TRUE;
                break;
            }
        }
    }

    if (bIsUpdated)
    {
        A_IMS_TRACE_I(AOSTAG,
                "UpdateNConfiguration :: Updated - ImsIdentityPriority(%s), IsimIndexForImpu(%d), "
                "SupportlimitedAdminSmsMode(%s)",
                IdentityPriorityToString(), m_nIsimIndexForImpu,
                _TRACE_B_(m_bSupportLimitedAdminSmsMode));
    }
    else
    {
        A_IMS_TRACE_I(AOSTAG, "UpdateNConfiguration :: Not Updated", 0, 0, 0);
    }

    return bIsUpdated;
}

PROTECTED
void AosSubscriberManager::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_ICC_LOADED_WAITING:
            ppiTimer = &m_piTimerToIccLoadedWaiting;
            break;

        case TIMER_PHONE_RESTART_RECOVERY:
            ppiTimer = &m_piTimerToPhoneRestartRecovery;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, TimerToString(nType));
}

PROTECTED
void AosSubscriberManager::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_ICC_LOADED_WAITING:
            ppiTimer = &m_piTimerToIccLoadedWaiting;
            break;

        case TIMER_PHONE_RESTART_RECOVERY:
            ppiTimer = &m_piTimerToPhoneRestartRecovery;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(*ppiTimer, TimerToString(nType));
}

PROTECTED
void AosSubscriberManager::ClearTimers()
{
    if (m_piTimerToIccLoadedWaiting != IMS_NULL)
    {
        StopTimer(TIMER_ICC_LOADED_WAITING);
    }

    if (m_piTimerToPhoneRestartRecovery != IMS_NULL)
    {
        StopTimer(TIMER_PHONE_RESTART_RECOVERY);
    }
}

PROTECTED
void AosSubscriberManager::NotifyState(IN IMS_UINT32 nState)
{
    if (m_nNotifyState == nState)
    {
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "NotifyState :: Current(%s) -> New(%s)", StateToString(m_nNotifyState),
            StateToString(nState), 0);
    m_nNotifyState = nState;

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosSubscriberManagerListener* piListener = m_objListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->AosSubscriberManager_NotifyState(nState);
        }
    }
}

PROTECTED
void AosSubscriberManager::NotifyMonitorState(IN IMS_UINT32 nState)
{
    if (m_nNotifyStateForFake == nState)
    {
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "NotifyMonitorState :: Current(%s) -> New(%s)",
            StateToString(m_nNotifyStateForFake), StateToString(nState), 0);
    m_nNotifyStateForFake = nState;

    for (IMS_UINT32 i = 0; i < m_objMonitorListeners.GetSize(); ++i)
    {
        IAosSubscriberManagerListener* piListener = m_objMonitorListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->AosSubscriberManager_NotifyState(nState);
        }
    }
}

PROTECTED
void AosSubscriberManager::NotifyAosIsimState(IN AosIsimState eState)
{
    if (m_eNotifyIsimState == eState)
    {
        return;
    }

    A_IMS_TRACE_I(
            AOSTAG, "NotifyAosIsimState :: Current(%d) -> New(%d)", m_eNotifyIsimState, eState, 0);
    m_eNotifyIsimState = eState;

    IAosService* service = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (service != IMS_NULL)
    {
        service->NotifyAosIsimState(eState);
    }
}

PROTECTED
IMS_BOOL AosSubscriberManager::IsPrimaryImpuValid(IN const AStringArray& objImpus)
{
    AString strPhoneNumber;
    GetPhoneNumber(strPhoneNumber);

    if (strPhoneNumber.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    AString strMsisdn;
    if (strPhoneNumber.GetLength() > USIM_MSISDN_LENGTH)
    {
        strMsisdn = strPhoneNumber.GetSubStr(strPhoneNumber.GetLength() - USIM_MSISDN_LENGTH);
    }
    else
    {
        strMsisdn = strPhoneNumber;
    }

    IMS_BOOL bIsAllZero = IMS_TRUE;
    for (IMS_SINT32 nAt = 0; nAt < strMsisdn.GetLength(); ++nAt)
    {
        if (strMsisdn[nAt] != '0')
        {
            bIsAllZero = IMS_FALSE;
            break;
        }
    }

    if (bIsAllZero)
    {
        return IMS_FALSE;
    }

    const AString& strMsisdnImpu = objImpus.GetElementAt(1);
    SipAddress objSipAddress;

    if (!objSipAddress.Create(strMsisdnImpu))
    {
        return IMS_FALSE;
    }

    AString strUser = objSipAddress.GetUser();
    if (!strUser.EndsWith(strMsisdn))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosSubscriberManager::IsSipUri(IN const AString& strImpu) const
{
    SipAddress objSipAddress;
    if (strImpu.GetLength() > 0)
    {
        if (objSipAddress.Create(strImpu))
        {
            if (objSipAddress.IsSchemeSip() || objSipAddress.IsSchemeSips())
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED
void AosSubscriberManager::RequestStop() const
{
    IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    if (piRsm == IMS_NULL || piRsm->GetImsRegState() == IMS_REG_OFF)
    {
        return;
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        piService->ControlRegistration(static_cast<IMS_SINT32>(AosRegRequestType::STOP),
                static_cast<IMS_SINT32>(AosPcscfOrder::CURRENT),
                static_cast<IMS_SINT32>(AosControlCause::IMS_SUBSCRIBER));
        A_IMS_TRACE_I(AOSTAG, "RequestStop :: Stop", 0, 0, 0);
    }
}

PROTECTED
void AosSubscriberManager::NConfiguration_NotifyConfigChanged()
{
    A_IMS_TRACE_I(AOSTAG, "NConfiguration_NotifyConfigChanged", 0, 0, 0);

    if (UpdateNConfiguration())
    {
        UpdateImsIdentity(GetIdentity(Index::FIRST));
        Restart();
    }
}

PROTECTED
void AosSubscriberManager::SubscriberConfig_InitCompleted()
{
    A_IMS_TRACE_I(AOSTAG, "SubscriberConfig_InitCompleted", 0, 0, 0);

    if (CheckIsimValues())
    {
        ProcessValidIsimOnCompleted(IMS_FALSE);
    }
    else
    {
        ProcessInvalidIsimOnCompleted(IMS_FALSE);
    }
}

PROTECTED
void AosSubscriberManager::SubscriberConfig_RefreshCompleted()
{
    A_IMS_TRACE_I(AOSTAG, "SubscriberConfig_RefreshCompleted", 0, 0, 0);

    m_bIsRefreshStarted = IMS_FALSE;

    if (CheckIsimValues())
    {
        ProcessValidIsimOnCompleted(IMS_TRUE);
    }
    else
    {
        ProcessInvalidIsimOnCompleted(IMS_TRUE);
    }

    UpdateImsi();
}

PROTECTED
void AosSubscriberManager::SubscriberConfig_RefreshStarted()
{
    A_IMS_TRACE_I(AOSTAG, "SubscriberConfig_RefreshStarted", 0, 0, 0);

    m_bIsRefreshStarted = IMS_TRUE;

    RemoveImpu();

    NotifyState(IAosSubscriber::REFRESH_STARTED);
    NotifyAosIsimState(AosIsimState::REFRESH_STARTED);
}

PROTECTED
void AosSubscriberManager::SubscriberConfig_NotifyError(IN IMS_SINT32 nErrorCode)
{
    if (IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY))
    {
        A_IMS_TRACE_I(AOSTAG, "SubscriberConfig_NotifyError :: Ignore error due to phone restart",
                0, 0, 0);
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "SubscriberConfig_NotifyError :: (%d)", nErrorCode, 0, 0);

    if (CheckAndTryUsimFallback())
    {
        return;
    }

    if (!CheckAndTryIsimImsiFallback())
    {
        ClearAll();
        A_IMS_TRACE_I(AOSTAG, "SubscriberConfig_NotifyError :: No further attempts", 0, 0, 0);
    }

    NotifyAosIsimState(AosIsimState::INVALID);
}

PROTECTED
void AosSubscriberManager::ConfigUpdate_NotifyUpdate(IN IMS_SINT32 /*nCpi*/,
        IN const AString& /*strConfName*/ /* = AString::ConstNull() */,
        IN const AString& /*strExtraParam*/ /* = AString::ConstNull() */)
{
}

PROTECTED
void AosSubscriberManager::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piTimerToIccLoadedWaiting)
    {
        ProcessIccLoadedWaitingTimerExpired();
        return;
    }

    if (piTimer == m_piTimerToPhoneRestartRecovery)
    {
        ProcessPhoneRestartRecoveryTimerExpired();
        return;
    }
}

PROTECTED
void AosSubscriberManager::ServicePhone_PhoneNumberStateChanged(
        IN IMS_BOOL /*bIsRefresh*/, IN PhoneNumberState /*eState*/)
{
    A_IMS_TRACE_I(AOSTAG, "ServicePhone_PhoneNumberStateChanged", 0, 0, 0);
    ProcessPhoneNumberAvailable();
}

PROTECTED
void AosSubscriberManager::ServicePhone_IsimStateChanged(IN IsimState eState)
{
    A_IMS_TRACE_I(AOSTAG, "ServicePhone_IsimStateChanged :: eState(%d)", eState, 0, 0);
    ProcessIsimStateChange(eState);
}

PROTECTED const IMS_CHAR* AosSubscriberManager::IdentityPriorityToString()
{
    m_strPriority.Resize(0);

    if (m_objImsIdentityPriority.IsEmpty())
    {
        return m_strPriority.Append("[Empty]").GetStr();
    }

    for (IMS_UINT32 i = 0; i < m_objImsIdentityPriority.GetSize(); ++i)
    {
        m_strPriority.Append("[");
        m_strPriority.Append(PrintIdentity(GetIdentity(static_cast<Index>(i))));
        m_strPriority.Append("]");
    }

    return m_strPriority.GetStr();
}

PROTECTED GLOBAL const IMS_CHAR* AosSubscriberManager::PrintIdentity(IN IMS_UINT32 nIdentity)
{
    switch (nIdentity)
    {
        case CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM:
            return "ISIM";

        case CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM:
            return "USIM";

        case CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI:
            return "ISIM_IMSI";

        default:
            return "CONF";
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosSubscriberManager::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_ICC_LOADED_WAITING:
            return "TIMER_ICC_LOADED_WAITING";

        case TIMER_PHONE_RESTART_RECOVERY:
            return "TIMER_PHONE_RESTART_RECOVERY";

        default:
            return "__INVALID__";
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosSubscriberManager::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case IAosSubscriber::NOT_READY:
            return "NOT_READY";

        case IAosSubscriber::READY:
            return "READY";

        case IAosSubscriber::REFRESH_STARTED:
            return "REFRESH_STARTED";

        case IAosSubscriber::REFRESH_COMPLETED:
            return "REFRESH_COMPLETED";

        case IAosSubscriber::REFRESH_FAILED:
            return "REFRESH_FAILED";

        default:
            return "INVALID";
    }
}
