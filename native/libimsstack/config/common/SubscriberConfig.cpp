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
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "AsyncConfigHelper.h"
#include "Credential.h"
#include "ISubscriberConfigListener.h"
#include "ISubscriberInfoListener.h"
#include "ServerAddress.h"
#include "private/SubscriberConfig.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC
SubscriberConfig::SubscriberConfig(IN IMS_SINT32 nSlotId, IN const AString& strConfName) :
        ConfigBase(nSlotId),
        m_pConfigHelper(IMS_NULL),
        m_strId(AString::ConstNull()),
        m_strConfName(strConfName),
        m_nSubscriptionAttributes(SUBSCRIPTION_ATTRIBUTE_IMS | SUBSCRIPTION_ATTRIBUTE_ISIM),
        m_piIsim(IMS_NULL),
        m_bFlagRequestPending(IMS_FALSE),
        m_nConfiguredIsimRecords(ISIM_DONE),
        m_nIsimRecords(ISIM_NONE),
        m_nIsimErrorCode(ISIM_NO_ERROR),
        m_byIst1(IST_1_NONE),
        m_nState(STATE_INIT),
        m_piSubsInfoListener(IMS_NULL),
        m_pConfigurable(IMS_NULL),
        m_nInitRetryCount(0),
        m_piInitRetryTimer(IMS_NULL),
        m_nInitRetryCountByStartRetry(0),
        m_nStartRetryCount(0),
        m_piStartRetryTimer(IMS_NULL),
        m_strLog(AString::ConstNull())
{
    IMS_SINT32 nIndex = strConfName.GetIndexOf('_');

    if (nIndex == AString::NPOS)
    {
        m_strId = GetDefaultId();
    }
    else
    {
        m_strId = strConfName.GetSubStr(nIndex + 1);
    }

    m_pConfigurable = new Configurable(this);

    m_objPcscfDiscoveryMethods.Add(PCSCF_DISCOVERY_METHOD_PCO);
}

PUBLIC VIRTUAL SubscriberConfig::~SubscriberConfig()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    piCc->RemoveListener(this);

    ClearPcscfAddressAndSubscriberInfo();

    if (m_pConfigurable != IMS_NULL)
    {
        delete m_pConfigurable;
    }

    if (m_piInitRetryTimer != IMS_NULL)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piInitRetryTimer);
    }

    if (m_piStartRetryTimer != IMS_NULL)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piStartRetryTimer);
    }

    if (m_piIsim != IMS_NULL)
    {
        m_piIsim->RemoveListener(this);
    }

    if (m_pConfigHelper != IMS_NULL)
    {
        m_pConfigHelper->Unregister(this);
    }
}

PUBLIC VIRTUAL IImsSubscriberInfo* SubscriberConfig::GetSubscriberInfo(
        IN IMS_SINT32 nIndex /*= 0*/) const
{
    IMS_SINT32 nCount = GetSubscriberCount();

    if ((nIndex < 0) || (nIndex >= nCount))
    {
        return IMS_NULL;
    }

    if (m_objSubscriberInfos.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objSubscriberInfos.GetAt(nIndex);
}

PUBLIC VIRTUAL IMS_BOOL SubscriberConfig::IsAkaSupported() const
{
    const Credential& objCredential = GetCredential();

    return (objCredential.GetType() == Credential::TYPE_AKAv1_MD5) ||
            (objCredential.GetType() == Credential::TYPE_AKAv2_MD5);
}

PUBLIC VIRTUAL const Credential& SubscriberConfig::GetCredential() const
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();
    return (pSubsInfo != IMS_NULL) ? pSubsInfo->GetCredential() : Credential::ConstNull();
}

PUBLIC VIRTUAL const AString& SubscriberConfig::GetHomeDomainName() const
{
    IImsSubscriberInfo* piSubsInfo = GetSubscriberInfo();
    return (piSubsInfo != IMS_NULL) ? piSubsInfo->GetHomeDomainName() : AString::ConstNull();
}

PUBLIC VIRTUAL IMS_SINT32 SubscriberConfig::GetIndexOfPrimaryPublicUserId() const
{
    IImsSubscriberInfo* piSubsInfo = GetSubscriberInfo();
    return (piSubsInfo != IMS_NULL) ? piSubsInfo->GetIndexOfPrimaryPublicUserId() : (-1);
}

PUBLIC VIRTUAL const AString& SubscriberConfig::GetPhoneContext() const
{
    IImsSubscriberInfo* piSubsInfo = GetSubscriberInfo();
    return (piSubsInfo != IMS_NULL) ? piSubsInfo->GetPhoneContext() : AString::ConstNull();
}

PUBLIC VIRTUAL const AString& SubscriberConfig::GetPrivateUserId() const
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();
    return (pSubsInfo != IMS_NULL) ? pSubsInfo->GetPrivateUserId() : AString::ConstNull();
}

PUBLIC VIRTUAL const AString& SubscriberConfig::GetPublicUserId(
        IN IMS_SINT32 nImpuType /*= IImsSubscriberInfo::IMPU_REF_INDEX*/) const
{
    IImsSubscriberInfo* piSubsInfo = GetSubscriberInfo();
    return (piSubsInfo != IMS_NULL) ? piSubsInfo->GetPublicUserId(nImpuType) : AString::ConstNull();
}

PUBLIC VIRTUAL const AStringArray& SubscriberConfig::GetPublicUserIds() const
{
    IImsSubscriberInfo* piSubsInfo = GetSubscriberInfo();
    return (piSubsInfo != IMS_NULL) ? piSubsInfo->GetPublicUserIds() : AStringArray::ConstNull();
}

PUBLIC VIRTUAL IMS_BOOL SubscriberConfig::Init()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    piCc->AddListener(this);

    return ConfigBase::Init();
}

PUBLIC VIRTUAL void SubscriberConfig::Refresh()
{
    m_nSubscriptionAttributes = SUBSCRIPTION_ATTRIBUTE_IMS | SUBSCRIPTION_ATTRIBUTE_ISIM;
    m_bFlagRequestPending = IMS_FALSE;
    m_nConfiguredIsimRecords = ISIM_DONE;
    m_nIsimRecords = ISIM_NONE;
    m_byIst1 = IST_1_NONE;
    m_nIsimErrorCode = ISIM_NO_ERROR;

    m_nInitRetryCount = 0;
    m_nInitRetryCountByStartRetry = 0;
    m_nStartRetryCount = 0;

    if (m_piInitRetryTimer != IMS_NULL)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piInitRetryTimer);
        m_piInitRetryTimer = IMS_NULL;
    }

    if (m_piStartRetryTimer != IMS_NULL)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piStartRetryTimer);
        m_piStartRetryTimer = IMS_NULL;
    }

    SetState(STATE_INIT);

    UpdateAllConfigs();

    if (IsIsimSupported())
    {
        // FIXME: synchronization
        IMS_SYS_Sleep(10);
    }
}

PUBLIC
const AString& SubscriberConfig::GetScscfAddress() const
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();
    return (pSubsInfo != IMS_NULL) ? pSubsInfo->GetScscfAddress() : AString::ConstNull();
}

PUBLIC
IMS_BOOL SubscriberConfig::IsAuthRealmLenient() const
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();
    return (pSubsInfo != IMS_NULL) ? pSubsInfo->IsAuthRealmLenient() : IMS_FALSE;
}

PUBLIC GLOBAL const AString& SubscriberConfig::GetDefaultId()
{
    static const AString DEFAULT_ID("default");
    return DEFAULT_ID;
}

PUBLIC
ImsSubscriberInfo* SubscriberConfig::GetSubscriberInfoEx(IN IMS_SINT32 nIndex /*= 0*/) const
{
    IMS_SINT32 nCount = GetSubscriberCount();

    if ((nIndex < 0) || (nIndex >= nCount))
    {
        return IMS_NULL;
    }

    if (m_objSubscriberInfos.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objSubscriberInfos.GetAt(nIndex);
}

PROTECTED VIRTUAL void SubscriberConfig::HandleMessage(
        IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2)
{
    (void)nParam1;

    switch (nMsg)
    {
        case ACMSG_START:
        {
            m_pConfigHelper = DYNAMIC_CAST(AsyncConfigHelper*, nParam1);

            if (m_pConfigHelper != IMS_NULL)
            {
                m_pConfigHelper->Register(this);
            }

            // Initialize the provisioning ...
            InitProvisioning();
            break;
        }
        case ACMSG_START_PROVISIONING:
        {
            StartProvisioning(nParam1 == 1);
            break;
        }
        case ACMSG_READ_ISIM_RECORD:
        {
            if (IsIsimSupported())
            {
                if (!IsIsimProvisioningDone())
                {
                    // FIX_TIMING_ISSUE
                    if ((nParam1 == 1) && m_bFlagRequestPending)
                    {
                        IMS_TRACE_D("ISIM :: READ_ISIM_RECORD before read operation completion", 0,
                                0, 0);
                        m_bFlagRequestPending = IMS_FALSE;
                    }

                    ReadIsimProvisioning();
                }
                else
                {
                    if (IsProvisioningDone())
                    {
                        // If the provisioning is already done, do nothing...
                        break;
                    }

                    IMS_SINT32 nPrevState = GetState();

                    SetState(STATE_PROVISIONED);
                    ToDebugString();

                    WriteProvisioning();

                    CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);

                    if (m_objListeners.IsEmpty())
                    {
                        IMS_TRACE_D("No SubscriberConfig listener", 0, 0, 0);
                        break;
                    }

                    if (nPrevState == STATE_PROVISIONING)
                    {
                        NotifyInitCompleted();
                    }
                    else
                    {
                        NotifyRefreshCompleted();
                    }
                }
            }
            break;
        }
        case ACMSG_INIT_COMPLETED:
        {
            WriteProvisioning();

            CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);
            NotifyInitCompleted();
            break;
        }
        case ACMSG_REFRESH_COMPLETED:
        {
            WriteProvisioning();

            CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);
            NotifyRefreshCompleted();
            // Notify configuration updates
            NotifyUpdate(IConfigurable::CP_I_SUBSCRIBER_ALL, m_strConfName, m_strId);
            break;
        }
        case ACMSG_REFRESH_STARTED:
        {
            NotifyRefreshStarted();
            break;
        }
        case ACMSG_NOTIFY_ERROR:
        {
            if (nParam1 == 0)
            {
                NotifyError(LONG_TO_INT(nParam2));
            }
            else
            {
                ISubscriberConfigListener* piNewListener =
                        reinterpret_cast<ISubscriberConfigListener*>(nParam1);

                NotifyError(LONG_TO_INT(nParam2), piNewListener);
            }
            break;
        }
        case ACMSG_UPDATE_ALL_CONFIGS:
        {
            SetState(STATE_REFRESHING);
            NotifyRefreshStarted();

            // Read all the subscriber configuration...
            UpdateAllConfigs();

            if (!IsIsimSupported())
            {
                SendMessage(ACMSG_REFRESH_COMPLETED, 0, 0);
                // Notify configuration updates
                NotifyUpdate(IConfigurable::CP_I_SUBSCRIBER_ALL, m_strConfName, m_strId);
            }
            break;
        }
        case ACMSG_INIT_RETRY_TIMER:
        {
            if (m_piInitRetryTimer != IMS_NULL)
            {
                TimerService::GetTimerService()->DestroyTimer(m_piInitRetryTimer);
            }

            m_piInitRetryTimer = TimerService::GetTimerService()->CreateTimer();

            if (m_piInitRetryTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating a retry (Init) timer failed", 0, 0, 0);
                break;
            }

            m_piInitRetryTimer->SetTimer(LONG_TO_UINT(nParam1 * 1000), this);
            break;
        }
        case ACMSG_START_RETRY_TIMER:
        {
            if (m_piStartRetryTimer != IMS_NULL)
            {
                TimerService::GetTimerService()->DestroyTimer(m_piStartRetryTimer);
            }

            m_piStartRetryTimer = TimerService::GetTimerService()->CreateTimer();

            if (m_piStartRetryTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating a retry (Start) timer failed", 0, 0, 0);
                break;
            }

            m_piStartRetryTimer->SetTimer(LONG_TO_UINT(nParam1 * 1000), this);
            break;
        }
        case ACMSG_RECOVERY_REQUIRED:
        {
            if (nParam1 == 0)
            {
                m_nInitRetryCount = 0;
                m_nStartRetryCount = 0;

                // Initialize the provisioning ...
                InitProvisioning();
            }
            else
            {
                if (m_piIsim != IMS_NULL)
                {
                    m_piIsim->Release();
                }

                m_nInitRetryCount = 1;

                SendMessage(ACMSG_INIT_RETRY_TIMER, nParam1, 0);
                IMS_TRACE_D("ISIM initialization will be retry after %d s", nParam1, 0, 0);
            }
            break;
        }
        case ACMSG_REFRESH_ISIM_PROVISIONING:
        {
            RefreshIsimProvisioning();
            break;
        }
        default:
        {
            break;
        }
    }
}

PROTECTED VIRTUAL void SubscriberConfig::RemoveListener(
        IN ISubscriberConfigListener* piListener) const
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISubscriberConfigListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);

            IMS_TRACE_D("SubscriberConfig :: Listener (%p) is removed", piListener, 0, 0);
            return;
        }
    }
}

PROTECTED VIRTUAL void SubscriberConfig::SetListener(IN ISubscriberConfigListener* piListener) const
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISubscriberConfigListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            IMS_TRACE_D("SubscriberConfig :: Listener (%p) is already set", piListener, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);

    IMS_TRACE_D("SubscriberConfig :: Listener (%p) is set", piListener, 0, 0);

    if (IsServiceAllowed() && IsIsimSupported() && (m_nIsimErrorCode != ISIM_NO_ERROR))
    {
        SubscriberConfig* pSubsConfig = const_cast<SubscriberConfig*>(this);

        pSubsConfig->SendMessage(
                ACMSG_NOTIFY_ERROR, reinterpret_cast<IMS_SINTP>(piListener), 0 /*Error Code*/);
    }
}

PROTECTED VIRTUAL IMS_BOOL SubscriberConfig::ReadFrom()
{
    ICarrierConfig* piCc = GetCarrierConfig();

    m_nIsimErrorCode = ISIM_NO_ERROR;

    m_nSubscriptionAttributes = ReadSubscriptionAttributes(piCc);
    m_objPcscfDiscoveryMethods = ReadPcscfDiscoveryMethods(piCc);

    IMS_SINT32 nPort = piCc->GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT);
    IImsPrivateProperty* piProperty = GetPrivateProperty();
    AString strPcscfAddressList = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST, GetSlotId());
    IMSList<AString> objPcscfAddresses = strPcscfAddressList.Split(',');

    for (IMS_UINT32 i = 0; i < objPcscfAddresses.GetSize(); ++i)
    {
        ServerAddress* pSa = new ServerAddress(objPcscfAddresses.GetAt(i), nPort);

        if (pSa == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (m_objPcscfAddresses.Add(pSa) < 0)
        {
            delete pSa;
            return IMS_FALSE;
        }
    }

    piProperty->SetPersistentBoolean(
            ImsPrivateProperties::Persistent::KEY_ISIM_ENABLED, IsIsimSupported(), GetSlotId());
    piProperty->SetPersistentBoolean(
            ImsPrivateProperties::Persistent::KEY_USIM_ENABLED, IsUsimSupported(), GetSlotId());

    //
    // Device supports the ISIM application, so the subscriber info. should be read from ISIM.
    //
    if (IsServiceAllowed() && IsIsimSupported())
    {
        IMS_TRACE_D("ISIM is enabled", 0, 0, 0);

        ImsSubscriberInfo* pSubsInfo = new ImsSubscriberInfo();

        pSubsInfo->m_nRefIndexOfPrimaryImpu =
                piCc->GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT, 1);
        pSubsInfo->m_strPhoneContext =
                piCc->GetString(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING);
        pSubsInfo->m_bIsAuthRealmLenient = IMS_TRUE;
        pSubsInfo->m_objCredential.SetType(Credential::TYPE_AKAv1_MD5);
        pSubsInfo->m_objCredential.SetPassword(AString::ConstNull());

        if (!m_objSubscriberInfos.Append(pSubsInfo))
        {
            delete pSubsInfo;
            return IMS_FALSE;
        }

        if (GetState() == STATE_REFRESHING)
        {
            // Do not transit the state
        }
        else
        {
            SetState(STATE_PROVISIONING);
        }

        if (m_strId.Equals(GetDefaultId()))
        {
            // LOG_EXCLUDING_SERVER_INFO, only for default subscriber
            IMS_UTIL_SYS_PROP_SET_DEBUG_ON(IsDebugOn());
        }

        IMS_TRACE_D(
                "SubscriberConfig (%s:%s) is loaded", m_strConfName.GetStr(), m_strId.GetStr(), 0);

        return IMS_TRUE;
    }

    ImsSubscriberInfo* pSubsInfo = new ImsSubscriberInfo();

    pSubsInfo->m_strHomeDomainName = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME, GetSlotId());

    pSubsInfo->m_strPrivateUserId = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI, GetSlotId());

    pSubsInfo->m_nRefIndexOfPrimaryImpu = 0;
    AString strPublicUserIds = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST, GetSlotId());

    pSubsInfo->m_objPublicUserIds = strPublicUserIds.Split(',');

    if (pSubsInfo->m_objPublicUserIds.GetCount() > 0)
    {
        piProperty->SetPersistent(ImsPrivateProperties::Persistent::KEY_PRIMARY_IMPU,
                pSubsInfo->m_objPublicUserIds.GetElementAt(0), GetSlotId());
    }

    pSubsInfo->m_strPhoneContext =
            piCc->GetString(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING);

    pSubsInfo->m_bIsAuthRealmLenient = IMS_TRUE;
    pSubsInfo->m_objCredential.SetType(Credential::TYPE_AKAv1_MD5);
    pSubsInfo->m_objCredential.SetPassword(AString::ConstNull());
    pSubsInfo->m_objCredential.SetUsername(pSubsInfo->m_strPrivateUserId);
    pSubsInfo->m_objCredential.SetRealm(pSubsInfo->m_strHomeDomainName);

    pSubsInfo->m_strScscfAddress = pSubsInfo->m_strHomeDomainName;

    // Additional operation: set the primary IMPU
    SetPrimaryImpu(pSubsInfo);

    if (!m_objSubscriberInfos.Append(pSubsInfo))
    {
        delete pSubsInfo;
        return IMS_FALSE;
    }

    SetState(STATE_PROVISIONED);

    if (m_strId.Equals(GetDefaultId()))
    {
        // LOG_EXCLUDING_SERVER_INFO, only for default subscriber
        IMS_UTIL_SYS_PROP_SET_DEBUG_ON(IsDebugOn());
    }

    ToDebugString();

    IMS_TRACE_D("SubscriberConfig (%d, %s:%s) is loaded", GetSlotId(), m_strConfName.GetStr(),
            m_strId.GetStr());

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL SubscriberConfig::Update(
        IN IMS_SINT32 nCpi, IN const AString& strValue /*= AString::ConstNull()*/)
{
    IMS_BOOL bUpdateResult = IMS_TRUE;

    switch (nCpi)
    {
        case IConfigurable::CP_I_START_SUBSCRIBER:
        case IConfigurable::CP_I_END_SUBSCRIBER:
            // Control messages MUST be notified to the application...
            break;

        case IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ALL:
        {
            ICarrierConfig* piCc = GetCarrierConfig();

            m_nSubscriptionAttributes = ReadSubscriptionAttributes(piCc);
            m_objPcscfDiscoveryMethods = ReadPcscfDiscoveryMethods(piCc);

            // LOG_EXCLUDING_SERVER_INFO
            IMS_UTIL_SYS_PROP_SET_DEBUG_ON(IsDebugOn());

            IMS_TRACE_D("SUBSCRIPTION_ATTRIBUTE_ALL :: Updated (attributes=%08X)",
                    GetSubscriptionAttributes(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM:
        {
            if (strValue.GetLength() > 0)
            {
                SetOrClearSubscriptionAttributes(
                        strValue.EqualsIgnoreCase("true"), SUBSCRIPTION_ATTRIBUTE_ISIM);
            }
            else
            {
                IMS_SINT32 nSubsAttributes = ReadSubscriptionAttributes(GetCarrierConfig());

                SetOrClearSubscriptionAttributes(
                        (nSubsAttributes & SUBSCRIPTION_ATTRIBUTE_ISIM) != 0,
                        SUBSCRIPTION_ATTRIBUTE_ISIM);
            }

            IMS_TRACE_D("SUBSCRIPTION_ATTRIBUTE_ISIM :: %s", _TRACE_B_(IsIsimSupported()), 0, 0);
            break;
        }
        case IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM:
        {
            if (strValue.GetLength() > 0)
            {
                SetOrClearSubscriptionAttributes(
                        strValue.EqualsIgnoreCase("true"), SUBSCRIPTION_ATTRIBUTE_USIM);
            }
            else
            {
                IMS_SINT32 nSubsAttributes = ReadSubscriptionAttributes(GetCarrierConfig());

                SetOrClearSubscriptionAttributes(
                        (nSubsAttributes & SUBSCRIPTION_ATTRIBUTE_USIM) != 0,
                        SUBSCRIPTION_ATTRIBUTE_USIM);
            }

            IMS_TRACE_D("SUBSCRIPTION_ATTRIBUTE_USIM :: %s", _TRACE_B_(IsUsimSupported()), 0, 0);
            break;
        }
        case IConfigurable::CP_I_HOME_DOMAIN_NAME:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the home domain name (%s); ISIM supports",
                        GetLog(strValue, 4).GetStr(), 0, 0);
                return IMS_FALSE;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (strValue.GetLength() > 0)
            {
                pSubsInfo->m_strHomeDomainName = strValue;
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();

            pSubsInfo->m_strHomeDomainName = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME, GetSlotId());

            IMS_TRACE_D("HOME_DOMAIN_NAME :: %s",
                    GetLog(pSubsInfo->m_strHomeDomainName, 4).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_IMPI:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the IMPI (%s); ISIM supports",
                        GetLog(strValue, 6).GetStr(), 0, 0);
                return IMS_FALSE;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (strValue.GetLength() > 0)
            {
                pSubsInfo->m_strPrivateUserId = strValue;
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();

            pSubsInfo->m_strPrivateUserId = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI, GetSlotId());

            IMS_TRACE_D("IMPI :: %s", GetLog(pSubsInfo->m_strPrivateUserId, 6).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_IMPU_PRIMARY_REF_INDEX:
        {
            if (strValue.GetLength() > 0)
            {
                IMS_BOOL bOK = IMS_FALSE;
                IMS_SINT32 nIndex = strValue.ToInt32(&bOK);

                if (!bOK)
                {
                    IMS_TRACE_E(0, "Invalid index (%s, %d)", strValue.GetStr(), nIndex, 0);
                    return IMS_FALSE;
                }

                ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

                if (pSubsInfo == IMS_NULL)
                {
                    IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                    return IMS_FALSE;
                }

                pSubsInfo->m_nRefIndexOfPrimaryImpu = nIndex;

                IMS_TRACE_D(
                        "IMPU_PRIMARY_REF_INDEX :: %d", pSubsInfo->m_nRefIndexOfPrimaryImpu, 0, 0);
                break;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            ICarrierConfig* piCc = GetCarrierConfig();

            pSubsInfo->m_nRefIndexOfPrimaryImpu =
                    piCc->GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT, 1);

            IMS_TRACE_D("IMPU_PRIMARY_REF_INDEX :: %d", pSubsInfo->m_nRefIndexOfPrimaryImpu, 0, 0);
            break;
        }
        case IConfigurable::CP_I_IMPU_0:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_1:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_2:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_3:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_4:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_5:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_6:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_7:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_8:  // FALL-THROUGH
        case IConfigurable::CP_I_IMPU_9:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the IMPU (%s); ISIM supports",
                        GetLog(strValue, 10).GetStr(), 0, 0);
                return IMS_FALSE;
            }

            IMS_SINT32 nIndex = nCpi - IConfigurable::CP_I_IMPU_0;
            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (nIndex > pSubsInfo->m_objPublicUserIds.GetCount())
            {
                IMS_TRACE_E(0, "Invalid index (%d) of IMPUs (%d)", nIndex,
                        pSubsInfo->m_objPublicUserIds.GetCount(), 0);
                return IMS_FALSE;
            }

            if (!strValue.IsNULL())
            {
                if (m_piSubsInfoListener != IMS_NULL)
                {
                    const AString& strOldId = (nIndex != pSubsInfo->m_objPublicUserIds.GetCount())
                            ? pSubsInfo->m_objPublicUserIds.GetElementAt(nIndex)
                            : AString::ConstNull();

                    m_piSubsInfoListener->SubscriberInfo_UpdateImpu(
                            GetSlotId(), m_strId, strOldId, strValue);
                }

                if (nIndex == pSubsInfo->m_objPublicUserIds.GetCount())
                {
                    pSubsInfo->m_objPublicUserIds.AddElement(strValue);
                }
                else
                {
                    pSubsInfo->m_objPublicUserIds.SetElementAt(strValue, nIndex);
                }

                IMS_TRACE_D("IMPU (%d) :: %s", nIndex, GetLog(strValue, 10).GetStr(), 0);
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();

            AString strPublicUserIds = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST, GetSlotId());
            AStringArray objPublicUserIds = strPublicUserIds.Split(',');

            const AString& strUserId = (nIndex < objPublicUserIds.GetCount())
                    ? objPublicUserIds.GetElementAt(nIndex)
                    : AString::ConstNull();

            if (m_piSubsInfoListener != IMS_NULL)
            {
                if (nIndex == pSubsInfo->m_objPublicUserIds.GetCount())
                {
                    m_piSubsInfoListener->SubscriberInfo_UpdateImpu(
                            GetSlotId(), m_strId, AString::ConstNull(), strUserId);
                }
                else
                {
                    m_piSubsInfoListener->SubscriberInfo_UpdateImpu(GetSlotId(), m_strId,
                            pSubsInfo->m_objPublicUserIds.GetElementAt(nIndex), strUserId);
                }
            }

            if (nIndex == pSubsInfo->m_objPublicUserIds.GetCount())
            {
                pSubsInfo->m_objPublicUserIds.AddElement(strUserId);
            }
            else
            {
                pSubsInfo->m_objPublicUserIds.SetElementAt(strUserId, nIndex);
            }

            // Update the primary IMPU
            SetPrimaryImpu(pSubsInfo);

            IMS_TRACE_D("IMPU (%d) :: %s", nIndex, GetLog(strUserId, 10).GetStr(), 0);
            break;
        }
        case IConfigurable::CP_I_PHONE_CONTEXT:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the phone context URI (%s); ISIM supports",
                        GetLog(strValue, 4).GetStr(), 0, 0);
                return IMS_FALSE;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (!strValue.IsNULL())
            {
                pSubsInfo->m_strPhoneContext = strValue;
                break;
            }

            ICarrierConfig* piCc = GetCarrierConfig();

            pSubsInfo->m_strPhoneContext = (piCc != IMS_NULL)
                    ? piCc->GetString(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING)
                    : AString::ConstNull();

            IMS_TRACE_D(
                    "PHONE_CONTEXT :: %s", GetLog(pSubsInfo->m_strPhoneContext, 4).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_AUTH_USERNAME:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the AUTH_USERNAME (%s); ISIM supports",
                        GetLog(strValue, 6).GetStr(), 0, 0);
                return IMS_FALSE;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (!strValue.IsNULL())
            {
                pSubsInfo->m_objCredential.SetUsername(strValue);
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();

            AString strPrivateUserId = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI, GetSlotId());

            pSubsInfo->m_objCredential.SetUsername(strPrivateUserId);

            IMS_TRACE_D("AUTH_USERNAME :: %s",
                    GetLog(pSubsInfo->m_objCredential.GetUsername(), 6).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_AUTH_PASSWORD:
        {
            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (!strValue.IsNULL())
            {
                pSubsInfo->m_objCredential.SetPassword(strValue);
                break;
            }

            pSubsInfo->m_objCredential.SetPassword(AString::ConstNull());

            IMS_TRACE_D("AUTH_PASSWORD :: %s",
                    GetLog(pSubsInfo->m_objCredential.GetPassword(), 3).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_AUTH_REALM:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the AUTH_REALM (%s); ISIM supports",
                        GetLog(strValue, 4).GetStr(), 0, 0);
                return IMS_FALSE;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (!strValue.IsNULL())
            {
                pSubsInfo->m_objCredential.SetRealm(strValue);
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();

            AString strHomeDomainName = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME, GetSlotId());

            pSubsInfo->m_objCredential.SetRealm(strHomeDomainName);

            IMS_TRACE_D("AUTH_REALM :: %s",
                    GetLog(pSubsInfo->m_objCredential.GetRealm(), 4).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_AUTH_ALGORITHM:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the AUTH_ALGORITHM (%s); ISIM supports",
                        strValue.GetStr(), 0, 0);
                return IMS_FALSE;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            if (strValue.GetLength() > 0)
            {
                if (strValue.EqualsIgnoreCase(Credential::STR_AKAv1_MD5))
                {
                    pSubsInfo->m_objCredential.SetType(Credential::TYPE_AKAv1_MD5);
                }
                else if (strValue.EqualsIgnoreCase(Credential::STR_AKAv2_MD5))
                {
                    pSubsInfo->m_objCredential.SetType(Credential::TYPE_AKAv2_MD5);
                }
                else
                {
                    pSubsInfo->m_objCredential.SetType(Credential::TYPE_MD5);
                }
                break;
            }

            pSubsInfo->m_objCredential.SetType(Credential::TYPE_AKAv1_MD5);

            IMS_TRACE_D("AUTH_ALGORITHM :: %d", pSubsInfo->m_objCredential.GetType(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_SERVER_SCSCF:
        {
            if (IsIsimSupported())
            {
                IMS_TRACE_D("Do not update the server scscf (%s); ISIM supports",
                        GetLog(strValue, 4).GetStr(), 0, 0);
                return IMS_FALSE;
            }

            ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

            if (pSubsInfo == IMS_NULL)
            {
                IMS_TRACE_E(0, "No subscriber info.", 0, 0, 0);
                return IMS_FALSE;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();

            pSubsInfo->m_strScscfAddress = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME, GetSlotId());

            IMS_TRACE_D(
                    "SERVER_SCSCF :: %s", GetLog(pSubsInfo->m_strScscfAddress, 4).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_PCSCF_DISCOVERY_METHODS:
        {
            if (strValue.GetLength() > 0)
            {
                m_objPcscfDiscoveryMethods.Clear();

                IMSList<AString> objDiscoveryMethods = strValue.Split(',');

                for (IMS_UINT32 i = 0; i < objDiscoveryMethods.GetSize(); ++i)
                {
                    const AString& strDiscoveryMethod = objDiscoveryMethods.GetAt(i);
                    IMS_SINT32 nDiscoveryMethod = strDiscoveryMethod.ToInt32();

                    if (nDiscoveryMethod >= 0)
                    {
                        m_objPcscfDiscoveryMethods.Add(nDiscoveryMethod);
                    }
                }

                if (m_objPcscfDiscoveryMethods.IsEmpty())
                {
                    m_objPcscfDiscoveryMethods.Add(PCSCF_DISCOVERY_METHOD_PCO);
                }
            }
            else
            {
                m_objPcscfDiscoveryMethods = ReadPcscfDiscoveryMethods(GetCarrierConfig());
            }
            break;
        }
        case IConfigurable::CP_I_PCSCF_ADDRESS_0:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_1:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_2:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_3:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_4:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_5:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_6:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_7:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_8:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_ADDRESS_9:
        {
            IMSVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

            if (pPcscfAddresses->IsEmpty())
            {
                IMS_TRACE_E(0, "No Proxy-CSCFs", 0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                IMS_UINT32 nIndex = nCpi - IConfigurable::CP_I_PCSCF_ADDRESS_0;

                if (nIndex >= pPcscfAddresses->GetSize())
                {
                    IMS_TRACE_E(0, "Invalid index (%d) of P-CSCF", nIndex, 0, 0);
                    return IMS_FALSE;
                }

                ServerAddress* pSa = pPcscfAddresses->GetAt(nIndex);

                if (pSa == IMS_NULL)
                {
                    IMS_TRACE_E(0, "No server address", 0, 0, 0);
                    return IMS_FALSE;
                }

                if (!strValue.IsNULL())
                {
                    pSa->SetAddress(strValue);

                    IMS_TRACE_D(
                            "PCSCF_ADDRESS (%d) :: %s", nIndex, GetLog(strValue, 5).GetStr(), 0);
                }
                else
                {
                    IImsPrivateProperty* piProperty = GetPrivateProperty();
                    AString strPcscfAddressList = piProperty->GetPersistent(
                            ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST,
                            GetSlotId());
                    IMSList<AString> objPcscfAddresses = strPcscfAddressList.Split(',');
                    const AString& strPcscfAddress = (nIndex < objPcscfAddresses.GetSize())
                            ? objPcscfAddresses.GetAt(nIndex)
                            : AString::ConstNull();
                    pSa->SetAddress(strPcscfAddress);

                    IMS_TRACE_D("PCSCF_ADDRESS (%d) :: %s", nIndex,
                            GetLog(pSa->GetAddress(), 5).GetStr(), 0);
                }
            }
            break;
        }
        case IConfigurable::CP_I_PCSCF_PORT_0:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_1:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_2:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_3:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_4:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_5:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_6:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_7:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_8:  // FALL-THROUGH
        case IConfigurable::CP_I_PCSCF_PORT_9:
        {
            IMSVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

            if (pPcscfAddresses->IsEmpty())
            {
                IMS_TRACE_E(0, "No Proxy-CSCFs", 0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                IMS_UINT32 nIndex = (nCpi - IConfigurable::CP_I_PCSCF_PORT_0);

                if (nIndex >= pPcscfAddresses->GetSize())
                {
                    IMS_TRACE_E(0, "Invalid index (%d) of P-CSCF", nIndex, 0, 0);
                    return IMS_FALSE;
                }

                ServerAddress* pSa = pPcscfAddresses->GetAt(nIndex);

                if (pSa == IMS_NULL)
                {
                    IMS_TRACE_E(0, "No server address", 0, 0, 0);
                    return IMS_FALSE;
                }

                if (strValue.GetLength() > 0)
                {
                    IMS_BOOL bOK = IMS_FALSE;
                    IMS_SINT32 nPort = strValue.ToInt32(&bOK);

                    if (!bOK)
                    {
                        IMS_TRACE_E(0, "Invalid Port (%s, %d)", strValue.GetStr(), nPort, 0);
                        return IMS_FALSE;
                    }

                    if (nPort <= 0)
                    {
                        nPort = ServerAddress::PORT_UNSPECIFIED;
                    }

                    pSa->SetPort(nPort);

                    IMS_TRACE_D("PCSCF_PORT (%d) :: %d", nIndex, nPort, 0);
                }
                else
                {
                    ICarrierConfig* piCc = GetCarrierConfig();
                    IMS_SINT32 nPort = (piCc != IMS_NULL)
                            ? piCc->GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT)
                            : (-1);
                    pSa->SetPort(nPort);

                    IMS_TRACE_D("PCSCF_PORT (%d) :: %d", nIndex, pSa->GetPort(), 0);
                }
            }
            break;
        }
        case IConfigurable::CP_I_PCSCF_ALL:
        {
            IMSVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

            if (pPcscfAddresses->IsEmpty())
            {
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();
            AString strPcscfAddressList = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST, GetSlotId());
            IMSList<AString> objPcscfAddresses = strPcscfAddressList.Split(',');

            ICarrierConfig* piCc = GetCarrierConfig();
            IMS_SINT32 nPort = (piCc != IMS_NULL)
                    ? piCc->GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT)
                    : (-1);

            IMS_UINT32 nSize = pPcscfAddresses->GetSize();

            for (IMS_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
            {
                ServerAddress* pSa = pPcscfAddresses->GetAt(nIndex);
                const AString& strAddress = (nIndex < objPcscfAddresses.GetSize())
                        ? objPcscfAddresses.GetAt(nIndex)
                        : AString::ConstNull();

                pSa->SetAddress(strAddress);
                pSa->SetPort(nPort);

                IMS_TRACE_D("PCSCF_ADDRESS_PORT (%d) :: %s (%d)", nIndex,
                        GetLog(pSa->GetAddress(), 5).GetStr(), pSa->GetPort());
            }
            break;
        }
        case IConfigurable::CP_I_SUBSCRIBER_ALL:
        {
            SendMessage(ACMSG_UPDATE_ALL_CONFIGS, 0, 0);
            break;
        }
        case IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER:
        {
            // Write the provisioning data to the storage medium
            WriteProvisioning();
            break;
        }
        default:
        {
            bUpdateResult = IMS_FALSE;
            IMS_TRACE_D("No configurable parameter item (%d)", nCpi, 0, 0);
            break;
        }
    }

    if (bUpdateResult)
    {
        if (nCpi != IConfigurable::CP_I_SUBSCRIBER_ALL)
        {
            NotifyUpdate(nCpi, m_strConfName, m_strId);
        }
    }

    return bUpdateResult;
}

PROTECTED VIRTUAL void SubscriberConfig::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    if (nSlotId != GetSlotId())
    {
        return;
    }

    IMS_TRACE_D(
            "SubscriberConfig: CarrierConfigChanged(%d) on %s", nSlotId, m_strConfName.GetStr(), 0);

    IMS_SINT32 nOldState = GetState();

    UpdateAllConfigs();

    if (!IsIsimSupported())
    {
        CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);
    }

    if (nOldState != STATE_INIT)
    {
        SetState(nOldState);
    }
}

PRIVATE VIRTUAL void SubscriberConfig::Isim_OnField(
        IN IMS_SINT32 nField, IN const IMSList<ByteArray>& objValues)
{
    switch (nField)
    {
        case IIsim::FIELD_IST:
        {
            m_bFlagRequestPending = IMS_FALSE;

            if (!objValues.IsEmpty())
            {
                const ByteArray& objIst = objValues.GetAt(0);

                if (objIst.GetLength() > 0)
                {
                    m_byIst1 = objIst[0];
                }
            }

            SetIsimRecord(ISIM_IST);
            SendMessage(ACMSG_READ_ISIM_RECORD, 1, 0);
            break;
        }
        case IIsim::FIELD_PCSCF_ADDRESS:
        {
            m_bFlagRequestPending = IMS_FALSE;

            if (!objValues.IsEmpty())
            {
                IMS_UINT32 nServerCount = m_objPcscfAddresses.GetSize();
                IMS_UINT32 nValueCount = objValues.GetSize();

                if (nServerCount > nValueCount)
                {
                    nServerCount = nValueCount;
                }

                for (IMS_UINT32 i = 0; i < nServerCount; ++i)
                {
                    const ByteArray& objValue = objValues.GetAt(i);
                    ServerAddress* pSa = m_objPcscfAddresses.GetAt(i);

                    if (pSa == IMS_NULL)
                    {
                        IMS_TRACE_E(0, "No server address", 0, 0, 0);
                        continue;
                    }

                    pSa->SetAddress(objValue.ToString());

                    IMS_TRACE_D("PCSCF_ADDRESS :: %s", GetLog(pSa->GetAddress(), 5).GetStr(), 0, 0);
                }

                nServerCount = m_objPcscfAddresses.GetSize();

                if (nServerCount > nValueCount)
                {
                    for (IMS_UINT32 i = nValueCount; i < nServerCount; ++i)
                    {
                        ServerAddress* pSa = m_objPcscfAddresses.GetAt(i);

                        if (pSa == IMS_NULL)
                        {
                            IMS_TRACE_E(0, "No server address", 0, 0, 0);
                            continue;
                        }

                        IMS_TRACE_D("PCSCF_ADDRESS :: %s >> empty",
                                GetLog(pSa->GetAddress(), 5).GetStr(), 0, 0);

                        pSa->SetAddress(AString::ConstNull());
                    }
                }
            }
            else
            {
                IMS_UINT32 nServerCount = m_objPcscfAddresses.GetSize();

                for (IMS_UINT32 i = 0; i < nServerCount; ++i)
                {
                    ServerAddress* pSa = m_objPcscfAddresses.GetAt(i);

                    if (pSa == IMS_NULL)
                    {
                        IMS_TRACE_E(0, "No server address", 0, 0, 0);
                        continue;
                    }

                    IMS_TRACE_D("PCSCF_ADDRESS :: %s >> empty",
                            GetLog(pSa->GetAddress(), 5).GetStr(), 0, 0);

                    pSa->SetAddress(AString::ConstNull());
                }
            }

            SetIsimRecord(ISIM_PCSCF);

            SendMessage(ACMSG_READ_ISIM_RECORD, 1, 0);
            break;
        }
        default:
        {
            IMS_TRACE_E(0, "Unsupported field : %d", nField, 0, 0);
            break;
        }
    }
}

PRIVATE VIRTUAL void SubscriberConfig::Isim_OnHomeDomainName(IN const ByteArray& objDomainName)
{
    if (!IsIsimSupported())
    {
        IMS_TRACE_E(0, "ISIM is not supported", 0, 0, 0);
        return;
    }

    m_bFlagRequestPending = IMS_FALSE;

    ImsSubscriberInfo* pSubsInfo = m_objSubscriberInfos.GetAt(0);

    if (pSubsInfo == IMS_NULL)
    {
        return;
    }

    // Updates the home domain name
    pSubsInfo->m_strHomeDomainName = objDomainName.ToString();

    // Updates the domain name related parameters
    pSubsInfo->m_strScscfAddress = pSubsInfo->m_strHomeDomainName;

    pSubsInfo->m_objCredential.SetUsername(pSubsInfo->m_strPrivateUserId);
    pSubsInfo->m_objCredential.SetRealm(pSubsInfo->m_strHomeDomainName);

    SetIsimRecord(ISIM_DOMAIN);

    SendMessage(ACMSG_READ_ISIM_RECORD, 1, 0);
}

PRIVATE VIRTUAL void SubscriberConfig::Isim_OnImpi(IN const ByteArray& objPrivateUserId)
{
    if (!IsIsimSupported())
    {
        IMS_TRACE_E(0, "ISIM is not supported", 0, 0, 0);
        return;
    }

    m_bFlagRequestPending = IMS_FALSE;

    ImsSubscriberInfo* pSubsInfo = m_objSubscriberInfos.GetAt(0);

    if (pSubsInfo == IMS_NULL)
    {
        return;
    }

    pSubsInfo->m_strPrivateUserId = objPrivateUserId.ToString();

    SetIsimRecord(ISIM_IMPI);

    SendMessage(ACMSG_READ_ISIM_RECORD, 1, 0);
}

PRIVATE VIRTUAL void SubscriberConfig::Isim_OnImpu(IN const IMSList<ByteArray>& objPublicUserIds)
{
    if (!IsIsimSupported())
    {
        IMS_TRACE_E(0, "ISIM is not supported", 0, 0, 0);
        return;
    }

    m_bFlagRequestPending = IMS_FALSE;

    ImsSubscriberInfo* pSubsInfo = m_objSubscriberInfos.GetAt(0);

    if (pSubsInfo == IMS_NULL)
    {
        return;
    }

    pSubsInfo->m_objPublicUserIds.RemoveAllElements();

    IMS_BOOL bUriSchemeRequiredWhenAbsent = IMS_TRUE;

    for (IMS_UINT32 i = 0; i < objPublicUserIds.GetSize(); ++i)
    {
        const ByteArray& objPublicUserId = objPublicUserIds.GetAt(i);
        AString strUserId = objPublicUserId.ToString();

        // Remove the leading & trailing white spaces
        strUserId = strUserId.Trim();

        if (objPublicUserId.GetLength() != strUserId.GetLength())
        {
            IMS_TRACE_E(0, "IMPU :: IMPU (in record at %d) contains the white spaces (%d >> %d)", i,
                    objPublicUserId.GetLength(), strUserId.GetLength());
        }

        if (strUserId.GetLength() == 0)
        {
            pSubsInfo->m_objPublicUserIds.AddElement(strUserId);
            continue;
        }

        AString strScheme = AString::ConstNull();
        IMS_SINT32 nStartIndex = strUserId.StartsWith('<') ? 1 : 0;

        if (strUserId.GetLength() >= (4 + nStartIndex))
        {
            strScheme = strUserId.GetSubStr(nStartIndex, 4);

            if (strScheme.EqualsIgnoreCase("sips") && (strUserId.GetLength() >= (5 + nStartIndex)))
            {
                strScheme = strUserId.GetSubStr(nStartIndex, 5);
            }
        }

        // As a default URI scheme, append "sip" URI scheme
        if (bUriSchemeRequiredWhenAbsent && !strScheme.EqualsIgnoreCase("sip:") &&
                !strScheme.EqualsIgnoreCase("tel:") && !strScheme.EqualsIgnoreCase("sips:"))
        {
            strUserId.Prepend("sip:");
            IMS_TRACE_D("Add a default URI scheme (sip) at %d", i, 0, 0);
        }

        pSubsInfo->m_objPublicUserIds.AddElement(strUserId);
    }

    SetIsimRecord(ISIM_IMPU);

    // Additional operation: set the primary IMPU
    SetPrimaryImpu(pSubsInfo);

    SendMessage(ACMSG_READ_ISIM_RECORD, 1, 0);
}

PRIVATE VIRTUAL void SubscriberConfig::Isim_OnError(IN IMS_SINT32 nErrorCode)
{
    if (!IsIsimSupported())
    {
        IMS_TRACE_E(0, "ISIM is not supported", 0, 0, 0);
        return;
    }

    if (nErrorCode == IIsim::ERROR_REFRESH_REG_FAILED)
    {
        IMS_TRACE_D("Registering ISIM refresh callback failed; "
                    "in this moment, ignore this error code...",
                0, 0, 0);
        return;
    }
    else if (nErrorCode == IIsim::ERROR_INTERFACE_CHANNEL_ERROR)
    {
        IMS_TRACE_D("ISIM communication channel has an error; initializing ISIM ...", 0, 0, 0);

        SendMessage(ACMSG_RECOVERY_REQUIRED, 0, 0);
        return;
    }
    else if (nErrorCode == IIsim::ERROR_REFRESH_ERROR)
    {
        IMS_TRACE_D("ISIM refresh error; waits for ISIM initialization...", 0, 0, 0);

        if (GetState() == STATE_REFRESHING)
        {
            SetState(STATE_PROVISIONED);
        }
        return;
    }

    m_bFlagRequestPending = IMS_FALSE;

    // Reset the all items
    ResetIsimRecord(m_nConfiguredIsimRecords);

    if (nErrorCode == IIsim::ERROR_READ_DENIED)
    {
        IMS_TRACE_D("ISIM read denied - wait for PIN enabling ...", 0, 0, 0);
        return;
    }

    IMS_SINT32 nSubsConfigErrorCode = 0;

    if ((nErrorCode == IIsim::ERROR_START_FAILED) || (nErrorCode == IIsim::ERROR_CARD_ERROR) ||
            (nErrorCode == IIsim::ERROR_CARD_REMOVED))
    {
        if (GetState() == STATE_PROVISIONED)
        {
            // No state transition
            // SetState(STATE_REFRESHING);
        }
        else
        {
            SetState(STATE_PROVISIONING);
        }
    }
    else if (nErrorCode == IIsim::ERROR_NO_ISIM_APPLICATION)
    {
        nSubsConfigErrorCode = ERROR_NO_ISIM_APPLICATION;
    }

    IMS_TRACE_D("ISIM operation failed - Error Code (%d)", nErrorCode, 0, 0);

    RecoverIsimProvisioning(nErrorCode);

    m_nIsimErrorCode = nErrorCode;

    // 4 According to the case, notify the error to the application
    SendMessage(ACMSG_NOTIFY_ERROR, 0, nSubsConfigErrorCode);
}

PRIVATE VIRTUAL void SubscriberConfig::Isim_OnStateChanged(IN IMS_SINT32 nState)
{
    if (!IsIsimSupported())
    {
        IMS_TRACE_E(0, "ISIM is not supported", 0, 0, 0);
        return;
    }

    AString strIdForLog;
    strIdForLog.Sprintf("%d, %s", GetSlotId(), GetId().GetStr());

    IMS_TRACE_I("SubsConfig (%s) :: %s on %s", strIdForLog.GetStr(), IsimStateToString(nState),
            StateToString(GetState()));

    switch (nState)
    {
        case IIsim::STATE_IDLE:
        {
            IMS_TRACE_D("ISIM is idle", 0, 0, 0);
            break;
        }
        case IIsim::STATE_INIT:
        {
            IMS_TRACE_D("ISIM initialization is completed", 0, 0, 0);

            if (GetState() == STATE_REFRESHING)
            {
                IMS_TRACE_D("ISIM is now refreshing; so, ignore STATE_INIT", 0, 0, 0);

                // We can consider that "NO ISIM APPLICATION" is a temporary error
                // such as subsystem silent restart, ...
                if (m_nIsimErrorCode != IIsim::ERROR_NO_ISIM_APPLICATION)
                {
                    break;
                }
            }
            else if (GetState() == STATE_PROVISIONED)
            {
                // MODEM_RESTART :: Reset the all items
                ResetIsimRecord(m_nConfiguredIsimRecords);
                CallSubscriberInfoListener(SUBSCRIBER_INFO_REMOVE_ALL);

                SetState(STATE_REFRESHING);
                SendMessage(ACMSG_REFRESH_STARTED, 0, 0);

                m_nInitRetryCount = 0;
                m_nIsimErrorCode = ISIM_NO_ERROR;
                m_bFlagRequestPending = IMS_FALSE;

                // Start ISIM provisioning ...
                SendMessage(ACMSG_START_PROVISIONING, 1, 0);
                break;
            }

            m_nInitRetryCount = 0;
            m_nIsimErrorCode = ISIM_NO_ERROR;
            m_bFlagRequestPending = IMS_FALSE;

            // Start ISIM provisioning ...
            SendMessage(ACMSG_START_PROVISIONING, 0, 0);
            break;
        }
        case IIsim::STATE_READY:
        {
            IMS_TRACE_D("ISIM is ready, the subscriber config can read a data from ISIM", 0, 0, 0);
            m_nStartRetryCount = 0;
            m_nIsimErrorCode = ISIM_NO_ERROR;
            SendMessage(ACMSG_READ_ISIM_RECORD, 0, 0);
            break;
        }
        case IIsim::STATE_REFRESHING:
        {
            IMS_TRACE_D("ISIM refresh is started", 0, 0, 0);
            m_nIsimErrorCode = ISIM_NO_ERROR;

            if (GetState() == STATE_PROVISIONING)
            {
                // Waits for ISIM refresh completed.
                // InitCompleted is not invoked yet.
            }
            else
            {
                SetState(STATE_REFRESHING);
            }

            SendMessage(ACMSG_REFRESH_STARTED, 0, 0);
            break;
        }
        case IIsim::STATE_REFRESHED:
        {
            IMS_TRACE_D("ISIM refresh is invoked, so the subscriber config will try "
                        "to read a new records",
                    0, 0, 0);

            m_nStartRetryCount = 0;
            m_nIsimErrorCode = ISIM_NO_ERROR;
            m_bFlagRequestPending = IMS_FALSE;

            if (GetState() == STATE_PROVISIONED)
            {
                SetState(STATE_REFRESHING);
                SendMessage(ACMSG_REFRESH_STARTED, 0, 0);
            }

            // Reset the all items
            ResetIsimRecord(m_nConfiguredIsimRecords);

            CallSubscriberInfoListener(SUBSCRIBER_INFO_REMOVE_ALL);

            // 4 Clear the ImsSubscriberInfo class

            SendMessage(ACMSG_START_PROVISIONING, 1, 0);
            break;
        }
        default:
        {
            break;
        }
    }
}

PRIVATE VIRTUAL void SubscriberConfig::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piInitRetryTimer == piTimer)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piInitRetryTimer);

        if (m_nInitRetryCount > 0)
        {
            // Initialize the provisioning ...
            InitProvisioning();
        }
    }
    else if (m_piStartRetryTimer == piTimer)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piStartRetryTimer);

        if (m_nStartRetryCount > 0)
        {
            // Start the provisioning ...
            StartProvisioning();
        }
    }
}

PRIVATE
void SubscriberConfig::SetOrClearSubscriptionAttributes(
        IN IMS_BOOL bEnabled, IN IMS_SINT32 nAttributes)
{
    if (bEnabled)
    {
        SetSubscriptionAttributes(nAttributes);
    }
    else
    {
        ClearSubscriptionAttributes(nAttributes);
    }
}

PRIVATE
void SubscriberConfig::CallSubscriberInfoListener(IN IMS_SINT32 nSubsInfo)
{
    if (m_piSubsInfoListener == IMS_NULL)
    {
        return;
    }

    if (nSubsInfo == SUBSCRIBER_INFO_ADD)
    {
        // ADD operation
        for (IMS_UINT32 i = 0; i < m_objSubscriberInfos.GetSize(); ++i)
        {
            ImsSubscriberInfo* pSubsInfo = m_objSubscriberInfos.GetAt(i);
            const AStringArray& objPublicUserIds = pSubsInfo->GetPublicUserIds();

            for (IMS_SINT32 j = 0; j < objPublicUserIds.GetCount(); ++j)
            {
                const AString& strUserId = objPublicUserIds.GetElementAt(j);

                if (strUserId.GetLength() == 0)
                {
                    continue;
                }

                m_piSubsInfoListener->SubscriberInfo_UpdateImpu(
                        GetSlotId(), m_strId, AString::ConstNull(), strUserId);
            }
        }
    }
    else if (nSubsInfo == SUBSCRIBER_INFO_REMOVE)
    {
        // REMOVE operation
        for (IMS_UINT32 i = 0; i < m_objSubscriberInfos.GetSize(); ++i)
        {
            ImsSubscriberInfo* pSubsInfo = m_objSubscriberInfos.GetAt(i);
            const AStringArray& objPublicUserIds = pSubsInfo->GetPublicUserIds();

            for (IMS_SINT32 j = 0; j < objPublicUserIds.GetCount(); ++j)
            {
                const AString& strUserId = objPublicUserIds.GetElementAt(j);

                if (strUserId.GetLength() == 0)
                {
                    continue;
                }

                m_piSubsInfoListener->SubscriberInfo_UpdateImpu(
                        GetSlotId(), m_strId, strUserId, AString::ConstNull());
            }
        }
    }
    else if (nSubsInfo == SUBSCRIBER_INFO_REMOVE_ALL)
    {
        m_piSubsInfoListener->SubscriberInfo_UpdateImpu(
                GetSlotId(), m_strId, AString::ConstNull(), AString::ConstNull());
    }
}

PRIVATE
void SubscriberConfig::ClearPcscfAddressAndSubscriberInfo()
{
    if (!m_objPcscfAddresses.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objPcscfAddresses.GetSize(); ++i)
        {
            ServerAddress* pSa = m_objPcscfAddresses.GetAt(i);

            if (pSa != IMS_NULL)
            {
                delete pSa;
            }
        }

        m_objPcscfAddresses.Clear();
    }

    if (!m_objSubscriberInfos.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSubscriberInfos.GetSize(); ++i)
        {
            ImsSubscriberInfo* pSubsInfo = m_objSubscriberInfos.GetAt(i);

            if (pSubsInfo != IMS_NULL)
            {
                delete pSubsInfo;
            }
        }

        m_objSubscriberInfos.Clear();
    }
}

PRIVATE
const AString& SubscriberConfig::GetLog(IN const AString& strValue, IN IMS_SINT32 nCount) const
{
    return UtilService::GetLogString(strValue, m_strLog, nCount);
}

PRIVATE
void SubscriberConfig::InitProvisioning()
{
    // If IMS is disabled, do not process any operation...
    if (!IsServiceAllowed())
    {
        // All the provisioning values will be filled by default...
        IMS_TRACE_D("IMS is disabled on INIT", 0, 0, 0);

        m_nInitRetryCount = 0;
        return;
    }

    // If the device uses ISIM and it is not provisioned,
    // try to read the IMS-related parameters from the ISIM.
    if (IsIsimSupported())
    {
        m_nIsimErrorCode = ISIM_NO_ERROR;

        IMS_TRACE_D("ISIM is enabled on INIT", 0, 0, 0);

        if (m_piIsim != IMS_NULL)
        {
            m_piIsim->RemoveListener(this);
        }

        m_piIsim = PhoneInfoService::GetPhoneInfoService()->GetIsim(GetSlotId());

        if (m_piIsim == IMS_NULL)
        {
            IMS_TRACE_E(0, "ISIM is null", 0, 0, 0);
            return;
        }

        m_piIsim->AddListener(this);

        if (m_piIsim->Init() != IMS_SUCCESS)
        {
            if (m_nInitRetryCount >= 0)
            {
                // 2s, 4s, 8s, 16s, 32s, 1m, 1m, ...
                static const IMS_SINT32 RETRY_INTERVAL[6] = {2, 2, 4, 8, 16, 32};
                IMS_SINT32 nRetryInterval =
                        (m_nInitRetryCount > 5) ? 60 : RETRY_INTERVAL[m_nInitRetryCount];

                m_nInitRetryCount++;

                // Start a retry timer
                SendMessage(ACMSG_INIT_RETRY_TIMER, nRetryInterval, 0);

                IMS_TRACE_D("ISIM initialization will be retry after %d s", nRetryInterval, 0, 0);
                return;
            }

            IMS_TRACE_E(0,
                    "Initializing ISIM failed; No operations "
                    "until power cycle or UICC card removed & inserted",
                    0, 0, 0);
            return;
        }
    }
    else
    {
        IMS_TRACE_D("ISIM is disabled on INIT", 0, 0, 0);
    }
}

PRIVATE
void SubscriberConfig::SetPrimaryImpu(IN ImsSubscriberInfo* pSubsInfo)
{
    pSubsInfo->m_strPrimaryImpuSipUri = AString::ConstEmpty();
    pSubsInfo->m_strPrimaryImpuTelUri = AString::ConstEmpty();

    if (pSubsInfo->m_objPublicUserIds.IsEmpty())
    {
        return;
    }

    // SIP URIs only
    //    -> First element is a default SIP URI.
    // SIP URIs & TEL URIs
    //    -> First element MUST be a SIP URI and second element MUST be a TEL URI.
    //    -> Both URIs will be a default URI for each format.
    IMS_BOOL bSipUriFound = IMS_FALSE;
    IMS_BOOL bTelUriFound = IMS_FALSE;

    for (IMS_SINT32 i = 0; i < pSubsInfo->m_objPublicUserIds.GetCount(); ++i)
    {
        AString strUserId = pSubsInfo->m_objPublicUserIds.GetElementAt(i).MakeLower();

        if (!bSipUriFound && strUserId.Contains("sip:"))
        {
            pSubsInfo->m_strPrimaryImpuSipUri = pSubsInfo->m_objPublicUserIds.GetElementAt(i);
            bSipUriFound = IMS_TRUE;
        }

        // If the multiple IMPUs exist, then check the second element is a TEL URI or not.
        if (!bTelUriFound && strUserId.Contains("tel:"))
        {
            pSubsInfo->m_strPrimaryImpuTelUri = pSubsInfo->m_objPublicUserIds.GetElementAt(i);
            bTelUriFound = IMS_TRUE;
        }

        if (bSipUriFound && bTelUriFound)
        {
            break;
        }
    }
}

PRIVATE
void SubscriberConfig::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("SubscriberConfig(%d) :: %s to %s", GetSlotId(), StateToString(m_nState),
            StateToString(nState));

    m_nState = nState;
}

PRIVATE
void SubscriberConfig::StartProvisioning(IN IMS_BOOL bIsRefresh /*= IMS_FALSE*/)
{
    // Initialize the request state
    m_bFlagRequestPending = IMS_FALSE;

    // If IMS is disabled, do not process any operation...
    if (!IsServiceAllowed())
    {
        // All the provisioning values will be filled by default...
        IMS_TRACE_D("IMS is disabled on %s", (!bIsRefresh) ? "START" : "REFRESH", 0, 0);
        return;
    }

    // If the device uses ISIM and it is not provisioned,
    // try to read the IMS-related parameters from the ISIM.
    if (IsIsimSupported())
    {
        IMS_TRACE_D("ISIM is enabled on %s", (!bIsRefresh) ? "START" : "REFRESH", 0, 0);

        if (m_piIsim == IMS_NULL)
        {
            IMS_TRACE_E(0, "ISIM is null", 0, 0, 0);
            return;
        }

        IMS_SINT32 nEFs = (IIsim::EF_IMPI | IIsim::EF_DOMAIN | IIsim::EF_IMPU);

        if ((m_nConfiguredIsimRecords & ISIM_IST) == ISIM_IST)
        {
            nEFs |= IIsim::EF_IST;
        }

        if ((m_nConfiguredIsimRecords & ISIM_PCSCF) == ISIM_PCSCF)
        {
            nEFs |= IIsim::EF_PCSCF;
        }

        if (m_piIsim->Start(nEFs) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Starting ISIM provisioning failed", 0, 0, 0);

            IMS_SINT32 nISIMState = m_piIsim->GetState();

            if ((nISIMState == IIsim::STATE_INIT) || (nISIMState == IIsim::STATE_REFRESHED))
            {
                // 1s, 1s, 2s, 2s, 4s, ...
                static const IMS_SINT32 RETRY_INTERVAL[5] = {1, 1, 2, 2, 4};
                IMS_SINT32 nRetryInterval =
                        (m_nStartRetryCount < 5) ? RETRY_INTERVAL[m_nStartRetryCount] : 10;

                m_nStartRetryCount++;

                // Start a retry timer
                if (m_nStartRetryCount > 5)
                {
                    m_nInitRetryCountByStartRetry++;

                    // Release ISIM resource and re-initialize the ISIM interface...
                    m_piIsim->Release();

                    m_nInitRetryCount = 1;

                    if (m_nInitRetryCountByStartRetry > 6)
                    {
                        // 60s
                        nRetryInterval = 60;
                    }
                    else
                    {
                        // 10s, 20s, 30s, 40s, 50s, 60s
                        nRetryInterval = nRetryInterval * m_nInitRetryCountByStartRetry;
                    }

                    m_nStartRetryCount = 0;

                    SendMessage(ACMSG_INIT_RETRY_TIMER, nRetryInterval, 0);
                    IMS_TRACE_D(
                            "ISIM initialization will be retry after %d s", nRetryInterval, 0, 0);
                }
                else
                {
                    SendMessage(ACMSG_START_RETRY_TIMER, nRetryInterval, 0);
                    IMS_TRACE_D("ISIM start will be retry after %d s", nRetryInterval, 0, 0);
                }
            }
            return;
        }

        m_nInitRetryCountByStartRetry = 0;

        if (!m_piIsim->IsReady())
        {
            // Waits for the state changed callback
            IMS_TRACE_D("Waits for the state changed notification from ISIM...", 0, 0, 0);
            return;
        }

        if (!IsIsimProvisioningDone())
        {
            ReadIsimProvisioning();
        }
    }
    else
    {
        IMS_TRACE_D("ISIM is disabled on %s", (!bIsRefresh) ? "START" : "REFRESH", 0, 0);
    }
}

PRIVATE
void SubscriberConfig::ReadIsimProvisioning()
{
    if (m_bFlagRequestPending)
    {
        IMS_TRACE_D("ISIM record is reading, so do not request to get the record...", 0, 0, 0);
        return;
    }

    if (m_piIsim->GetState() != IIsim::STATE_READY)
    {
        IMS_TRACE_D("ISIM is not ready; waits for the readiness of ISIM ...", 0, 0, 0);
        return;
    }

    // IMPI
    if (!IsIsimRecordSet(ISIM_IMPI))
    {
        IMS_RESULT nResult = m_piIsim->GetImpi();

        IMS_TRACE_D("Reading ISIM provisioning - IMPI ... result=%d", nResult, 0, 0);

        if (nResult == IMS_FAILURE)
        {
            RecoverIsimProvisioning(IIsim::ERROR_READ_IMPI_FAILED);

            m_nIsimErrorCode = IIsim::ERROR_READ_IMPI_FAILED;
            SendMessage(ACMSG_NOTIFY_ERROR, 0, 0 /* Error Code */);
            return;
        }
        else if (nResult == IIsim::RESULT_NO_RECORDS)
        {
            SetIsimRecord(ISIM_IMPI);
            SendMessage(ACMSG_READ_ISIM_RECORD, 0, 0);
            return;
        }

        m_bFlagRequestPending = IMS_TRUE;
        return;
    }

    // IMPU
    if (!IsIsimRecordSet(ISIM_IMPU))
    {
        IMS_RESULT nResult = m_piIsim->GetImpu();

        IMS_TRACE_D("Reading ISIM provisioning - IMPU ... result=%d", nResult, 0, 0);

        if (nResult == IMS_FAILURE)
        {
            RecoverIsimProvisioning(IIsim::ERROR_READ_IMPU_FAILED);

            m_nIsimErrorCode = IIsim::ERROR_READ_IMPU_FAILED;
            SendMessage(ACMSG_NOTIFY_ERROR, 0, 0 /* Error Code */);
            return;
        }
        else if (nResult == IIsim::RESULT_NO_RECORDS)
        {
            SetIsimRecord(ISIM_IMPU);
            SendMessage(ACMSG_READ_ISIM_RECORD, 0, 0);
            return;
        }

        m_bFlagRequestPending = IMS_TRUE;
        return;
    }

    // Home Domain Name
    if (!IsIsimRecordSet(ISIM_DOMAIN))
    {
        IMS_RESULT nResult = m_piIsim->GetHomeDomainName();

        IMS_TRACE_D("Reading ISIM provisioning - DOMAIN ... result=%d", nResult, 0, 0);

        if (nResult == IMS_FAILURE)
        {
            RecoverIsimProvisioning(IIsim::ERROR_READ_DOMAIN_FAILED);

            m_nIsimErrorCode = IIsim::ERROR_READ_DOMAIN_FAILED;
            SendMessage(ACMSG_NOTIFY_ERROR, 0, 0 /* Error Code */);
            return;
        }
        else if (nResult == IIsim::RESULT_NO_RECORDS)
        {
            SetIsimRecord(ISIM_DOMAIN);
            SendMessage(ACMSG_READ_ISIM_RECORD, 0, 0);
            return;
        }

        m_bFlagRequestPending = IMS_TRUE;
        return;
    }

    // IST (ISIM Service Table)
    if (((m_nConfiguredIsimRecords & ISIM_IST) == ISIM_IST) && !IsIsimRecordSet(ISIM_IST))
    {
        IMS_RESULT nResult = m_piIsim->GetField(IIsim::FIELD_IST);

        IMS_TRACE_D("Reading ISIM provisioning - IST ... result=%d", nResult, 0, 0);

        if (nResult == IMS_FAILURE)
        {
            m_nIsimErrorCode = IIsim::ERROR_READ_IST_FAILED;
            SendMessage(ACMSG_NOTIFY_ERROR, 0, 0 /* Error Code */);
            return;
        }
        else if (nResult == IIsim::RESULT_NO_RECORDS)
        {
            SetIsimRecord(ISIM_IST);
            SendMessage(ACMSG_READ_ISIM_RECORD, 0, 0);
            return;
        }

        m_bFlagRequestPending = IMS_TRUE;
        return;
    }

    // Optional field : P-CSCF address
    if (((m_nConfiguredIsimRecords & ISIM_PCSCF) == ISIM_PCSCF) && !IsIsimRecordSet(ISIM_PCSCF))
    {
        if ((m_nConfiguredIsimRecords & ISIM_IST) == ISIM_IST)
        {
            if ((m_byIst1 & IST_1_P_CSCF) != IST_1_P_CSCF)
            {
                for (IMS_UINT32 i = 0; i < m_objPcscfAddresses.GetSize(); ++i)
                {
                    ServerAddress* pSa = m_objPcscfAddresses.GetAt(i);

                    if (pSa == IMS_NULL)
                    {
                        IMS_TRACE_E(0, "No server address", 0, 0, 0);
                        continue;
                    }

                    IMS_TRACE_D("READ - PCSCF_ADDRESS :: %s >> empty",
                            GetLog(pSa->GetAddress(), 5).GetStr(), 0, 0);

                    pSa->SetAddress(AString::ConstNull());
                }

                SetIsimRecord(ISIM_PCSCF);
                SendMessage(ACMSG_READ_ISIM_RECORD, 0, 0);
                return;
            }
        }

        IMS_RESULT nResult = m_piIsim->GetField(IIsim::FIELD_PCSCF_ADDRESS);

        IMS_TRACE_D("Reading ISIM provisioning - P-CSCF address ... result=%d", nResult, 0, 0);

        if (nResult == IMS_FAILURE)
        {
            m_nIsimErrorCode = IIsim::ERROR_READ_PCSCF_ADDRESS_FAILED;
            SendMessage(ACMSG_NOTIFY_ERROR, 0, 0 /* Error Code */);
            return;
        }
        else if (nResult == IIsim::RESULT_NO_RECORDS)
        {
            for (IMS_UINT32 i = 0; i < m_objPcscfAddresses.GetSize(); ++i)
            {
                ServerAddress* pSa = m_objPcscfAddresses.GetAt(i);

                if (pSa == IMS_NULL)
                {
                    IMS_TRACE_E(0, "No server address", 0, 0, 0);
                    continue;
                }

                IMS_TRACE_D("READ - PCSCF_ADDRESS :: %s >> empty",
                        GetLog(pSa->GetAddress(), 5).GetStr(), 0, 0);

                pSa->SetAddress(AString::ConstNull());
            }

            SetIsimRecord(ISIM_PCSCF);
            SendMessage(ACMSG_READ_ISIM_RECORD, 0, 0);
            return;
        }

        m_bFlagRequestPending = IMS_TRUE;
        return;
    }
}

PRIVATE
void SubscriberConfig::RecoverIsimProvisioning(IN IMS_SINT32 nErrorCode)
{
    if ((nErrorCode == IIsim::ERROR_START_FAILED) ||
            (nErrorCode == IIsim::ERROR_READ_IMPI_FAILED) ||
            (nErrorCode == IIsim::ERROR_READ_IMPU_FAILED) ||
            (nErrorCode == IIsim::ERROR_READ_DOMAIN_FAILED))
    {
        if ((m_nIsimErrorCode == IIsim::ERROR_START_FAILED) ||
                (m_nIsimErrorCode == IIsim::ERROR_READ_IMPI_FAILED) ||
                (m_nIsimErrorCode == IIsim::ERROR_READ_IMPU_FAILED) ||
                (m_nIsimErrorCode == IIsim::ERROR_READ_DOMAIN_FAILED))
        {
            IMS_TRACE_D("ISIM recovery is already installed ...", 0, 0, 0);
            return;
        }

        if (GetState() == STATE_REFRESHING)
        {
            SetState(STATE_PROVISIONED);
        }
    }
}

PRIVATE
void SubscriberConfig::RefreshIsimProvisioning()
{
    IMS_TRACE_I("RefreshIsimProvisioning :: isim=%s", _TRACE_B_(IsIsimSupported()), 0, 0);

    if (IsIsimSupported())
    {
        if (m_piIsim == IMS_NULL)
        {
            InitProvisioning();
        }
        else
        {
            if (m_piIsim->IsReady())
            {
                // Clears all the EF records
                m_piIsim->ClearRecords();

                // Reset the all items
                ResetIsimRecord(m_nConfiguredIsimRecords);
                ReadIsimProvisioning();
            }
            else
            {
                // Waits for ISIM ready
            }
        }
    }
}

PRIVATE
void SubscriberConfig::NotifyInitCompleted()
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISubscriberConfigListener* piListener = m_objListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->SubscriberConfig_InitCompleted();
        }
    }
}

PRIVATE
void SubscriberConfig::NotifyRefreshCompleted()
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISubscriberConfigListener* piListener = m_objListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->SubscriberConfig_RefreshCompleted();
        }
    }
}

PRIVATE
void SubscriberConfig::NotifyRefreshStarted()
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISubscriberConfigListener* piListener = m_objListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->SubscriberConfig_RefreshStarted();
        }
    }
}

PRIVATE
void SubscriberConfig::NotifyError(
        IN IMS_SINT32 nErrorCode, IN ISubscriberConfigListener* piTargetListener /*= IMS_NULL*/)
{
    if (piTargetListener == IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
        {
            ISubscriberConfigListener* piListener = m_objListeners.GetAt(i);

            if (piListener != IMS_NULL)
            {
                piListener->SubscriberConfig_NotifyError(nErrorCode);
            }
        }

        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISubscriberConfigListener* piListener = m_objListeners.GetAt(i);

        if (piListener == piTargetListener)
        {
            piListener->SubscriberConfig_NotifyError(nErrorCode);
            break;
        }
    }
}

PRIVATE
void SubscriberConfig::SendMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2)
{
    if (m_pConfigHelper != IMS_NULL)
    {
        const IAsyncConfig* piAsyncConfig = this;

        m_pConfigHelper->SendTo(const_cast<IAsyncConfig*>(piAsyncConfig), nMsg, nParam1, nParam2);
    }
    else
    {
        HandleMessage(nMsg, nParam1, nParam2);
    }
}

PRIVATE
void SubscriberConfig::UpdateAllConfigs()
{
    CallSubscriberInfoListener(SUBSCRIBER_INFO_REMOVE_ALL);
    ClearPcscfAddressAndSubscriberInfo();

    if (!ReadFrom())
    {
        IMS_TRACE_E(0, "Updating all the configs (%s) failed", m_strConfName.GetStr(), 0, 0);
    }

    // Read ISIM records
    if (IsIsimSupported())
    {
        SendMessage(ACMSG_REFRESH_ISIM_PROVISIONING, 0, 0);
    }
}

PRIVATE
void SubscriberConfig::WriteProvisioning()
{
    IMS_TRACE_D("WriteProvisioning...", 0, 0, 0);

    IImsPrivateProperty* piProperty = GetPrivateProperty();
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

    if (pSubsInfo != IMS_NULL)
    {
        if (pSubsInfo->m_strHomeDomainName.GetLength() > 0)
        {
            piProperty->SetPersistent(ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME,
                    pSubsInfo->m_strHomeDomainName, GetSlotId());
        }

        if (pSubsInfo->m_strPrivateUserId.GetLength() > 0)
        {
            piProperty->SetPersistent(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI,
                    pSubsInfo->m_strPrivateUserId, GetSlotId());
        }

        AString strPublicUserIds;

        for (IMS_SINT32 i = 0; i < pSubsInfo->m_objPublicUserIds.GetCount(); ++i)
        {
            strPublicUserIds.Append(pSubsInfo->m_objPublicUserIds.GetElementAt(i));
            strPublicUserIds.Append(',');
        }

        if (strPublicUserIds.GetLength() > 0)
        {
            strPublicUserIds.Chop(1);
        }

        if (strPublicUserIds.GetLength() > 0)
        {
            piProperty->SetPersistent(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST,
                    strPublicUserIds, GetSlotId());
        }

        IMS_SINT32 nPrimaryImpuIndex = 0;

        if (IsIsimSupported())
        {
            nPrimaryImpuIndex = pSubsInfo->m_nRefIndexOfPrimaryImpu;

            if (pSubsInfo->m_objPublicUserIds.GetCount() <= nPrimaryImpuIndex)
            {
                nPrimaryImpuIndex = 0;
            }
        }

        if (pSubsInfo->m_objPublicUserIds.GetCount() > 0)
        {
            piProperty->SetPersistent(ImsPrivateProperties::Persistent::KEY_PRIMARY_IMPU,
                    pSubsInfo->m_objPublicUserIds.GetElementAt(nPrimaryImpuIndex), GetSlotId());
        }
    }

    piProperty->SetPersistentBoolean(
            ImsPrivateProperties::Persistent::KEY_ISIM_ENABLED, IsIsimSupported(), GetSlotId());
    piProperty->SetPersistentBoolean(
            ImsPrivateProperties::Persistent::KEY_USIM_ENABLED, IsUsimSupported(), GetSlotId());
}

PRIVATE
void SubscriberConfig::ToDebugString()
{
    if (!IsProvisioningDone())
    {
        return;
    }

    IMS_TRACE_D("IMS subscriber's info (%d, %s) -- starts", GetSlotId(), GetId().GetStr(), 0);
    IMS_TRACE_D("Subscription-attributes=%08X", GetSubscriptionAttributes(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objPcscfDiscoveryMethods.GetSize(); ++i)
    {
        IMS_SINT32 nDiscoveryMethod = m_objPcscfDiscoveryMethods.GetAt(i);
        IMS_TRACE_D("P-CSCF discovery method(%d)=%s", i,
                PcscfDiscoveryMethodToString(nDiscoveryMethod), 0);
    }

    // LOG_EXCLUDING_SERVER_INFO
    if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG() || IsDebugOn())
    {
        IMSVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

        for (IMS_UINT32 i = 0; i < pPcscfAddresses->GetSize(); ++i)
        {
            const ServerAddress* pSa = pPcscfAddresses->GetAt(i);

            if (pSa != IMS_NULL)
            {
                IMS_TRACE_D("P-CSCF(%d)=%s:%d", i, GetLog(pSa->GetAddress(), 5).GetStr(),
                        pSa->GetPort());
            }
        }
    }

    AString strDbgString;

    for (IMS_UINT32 i = 0; i < m_objSubscriberInfos.GetSize(); ++i)
    {
        const ImsSubscriberInfo* pSubsInfo = m_objSubscriberInfos.GetAt(i);

        if (pSubsInfo == IMS_NULL)
        {
            continue;
        }

        IMS_TRACE_D("HomeDomainName: %s", GetLog(pSubsInfo->m_strHomeDomainName, 4).GetStr(), 0, 0);
        IMS_TRACE_D("IMPI: %s", GetLog(pSubsInfo->m_strPrivateUserId, 6).GetStr(), 0, 0);

        strDbgString = GetLog(pSubsInfo->m_strPrimaryImpuTelUri, 10);
        IMS_TRACE_D("Primary IMPU(%d) - sip=%s, tel=%s", pSubsInfo->m_nRefIndexOfPrimaryImpu,
                GetLog(pSubsInfo->m_strPrimaryImpuSipUri, 10).GetStr(), strDbgString.GetStr());

        for (IMS_SINT32 j = 0; j < pSubsInfo->m_objPublicUserIds.GetCount(); ++j)
        {
            IMS_TRACE_D("IMPU(%d)=%s", j,
                    GetLog(pSubsInfo->m_objPublicUserIds.GetElementAt(j), 10).GetStr(), 0);
        }

        IMS_TRACE_D("Phone-context: %s", GetLog(pSubsInfo->m_strPhoneContext, 4).GetStr(), 0, 0);

        IMS_TRACE_D("Credential - username: %s",
                GetLog(pSubsInfo->m_objCredential.GetUsername(), 7).GetStr(), 0, 0);
        // LOG_EXCLUDING_SERVER_INFO
        if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG() || IsDebugOn())
        {
            IMS_TRACE_D("Credential - password: %s",
                    pSubsInfo->m_objCredential.GetPassword().GetStr(), 0, 0);
        }
        else
        {
            IMS_TRACE_D("Credential - password: %s", "xxx", 0, 0);
        }
        IMS_TRACE_D("Credential - realm: %s",
                GetLog(pSubsInfo->m_objCredential.GetRealm(), 4).GetStr(), 0, 0);

        IMS_SINT32 nAlgorithm = pSubsInfo->m_objCredential.GetType();
        const IMS_CHAR* pszAlgorithm = Credential::STR_MD5;

        if (nAlgorithm == Credential::TYPE_AKAv1_MD5)
        {
            pszAlgorithm = Credential::STR_AKAv1_MD5;
        }
        else if (nAlgorithm == Credential::TYPE_AKAv2_MD5)
        {
            pszAlgorithm = Credential::STR_AKAv2_MD5;
        }

        IMS_TRACE_D("Credential - algorithm: %d, %s", nAlgorithm, pszAlgorithm, 0);

        IMS_TRACE_D("S-CSCF: %s", GetLog(pSubsInfo->m_strScscfAddress, 4).GetStr(), 0, 0);
    }

    IMS_TRACE_D("IMS subscriber's info (%d, %s) -- ends", GetSlotId(), GetId().GetStr(), 0);
}

PRIVATE
IMS_SINT32 SubscriberConfig::ReadSubscriptionAttributes(IN ICarrierConfig* piCc)
{
    IMS_SINT32 nSubsAttributes = SUBSCRIPTION_ATTRIBUTE_IMS;

    if (!m_strId.Equals(GetDefaultId()))
    {
        return nSubsAttributes;
    }

    IMSVector<IMS_SINT32> objIdentityPriorities =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY);

    for (IMS_UINT32 i = 0; i < objIdentityPriorities.GetSize(); ++i)
    {
        IMS_SINT32 nPriority = objIdentityPriorities.GetAt(i);

        if (nPriority == CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM ||
                nPriority == CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI)
        {
            nSubsAttributes |= SUBSCRIPTION_ATTRIBUTE_ISIM;
        }
        else if (nPriority == CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM)
        {
            nSubsAttributes |= SUBSCRIPTION_ATTRIBUTE_USIM;
        }
    }

    return nSubsAttributes;
}

PRIVATE GLOBAL IMSVector<IMS_SINT32> SubscriberConfig::ReadPcscfDiscoveryMethods(
        IN ICarrierConfig* piCc)
{
    IMSVector<IMS_SINT32> objPcscfDiscoveryMethods =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY);

    if (objPcscfDiscoveryMethods.IsEmpty())
    {
        objPcscfDiscoveryMethods.Add(PCSCF_DISCOVERY_METHOD_PCO);
    }

    return objPcscfDiscoveryMethods;
}

PRIVATE GLOBAL const IMS_CHAR* SubscriberConfig::IsimStateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case IIsim::STATE_IDLE:
            return "IDLE";
        case IIsim::STATE_INIT:
            return "INIT";
        case IIsim::STATE_READY:
            return "READY";
        case IIsim::STATE_REFRESHING:
            return "REFRESHING";
        case IIsim::STATE_REFRESHED:
            return "REFRESHED";
        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* SubscriberConfig::PcscfDiscoveryMethodToString(IN IMS_SINT32 nMethod)
{
    switch (nMethod)
    {
        case PCSCF_DISCOVERY_METHOD_PCO:
            return "PCO";
        case PCSCF_DISCOVERY_METHOD_CONFIG:
            return "CONFIG";
        default:
            return "UNKNOWN";
    }
}

PRIVATE GLOBAL const IMS_CHAR* SubscriberConfig::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_PROVISIONING:
            return "STATE_PROVISIONING";
        case STATE_REFRESHING:
            return "STATE_REFRESHING";
        case STATE_PROVISIONED:
            return "STATE_PROVISIONED";
        default:
            return "__INVALID__";
    }
}
