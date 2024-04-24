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
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "AsyncConfigHelper.h"
#include "Credential.h"
#include "ISubscriberConfigListener.h"
#include "ISubscriberInfoListener.h"
#include "ImsIdentity.h"
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
        m_nState(STATE_INIT),
        m_piSubsInfoListener(IMS_NULL),
        m_pConfigurable(IMS_NULL),
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
    IMS_TRACE_D("Refresh(%s)", GetId().GetStr(), 0, 0);

    m_nSubscriptionAttributes = SUBSCRIPTION_ATTRIBUTE_IMS | SUBSCRIPTION_ATTRIBUTE_ISIM;

    SetState(STATE_INIT);
    UpdateAllConfigs();

    if (IsIsimSupported())
    {
        SendMessage(ACMSG_REFRESH_ISIM_RECORDS, 0, 0);
    }
    else
    {
        CompleteProvisioning();
        SendMessage(ACMSG_INIT_COMPLETED, 0, 0);
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
    IMS_TRACE_D(
            "HandleMessage: msg=%d, param1=%" PFLS_u ", param2=%" PFLS_u, nMsg, nParam1, nParam2);

    switch (nMsg)
    {
        case ACMSG_START:
        {
            m_pConfigHelper = DYNAMIC_CAST(AsyncConfigHelper*, nParam1);

            if (m_pConfigHelper != IMS_NULL)
            {
                m_pConfigHelper->Register(this);
            }

            InitIsim();
            break;
        }
        case ACMSG_UPDATE_ISIM_RECORDS:
        {
            UpdateIsimRecords();
            break;
        }
        case ACMSG_REFRESH_ISIM_RECORDS:
        {
            RefreshIsimRecords();
            break;
        }
        case ACMSG_INIT_COMPLETED:
        {
            NotifyInitCompleted();
            break;
        }
        case ACMSG_REFRESH_COMPLETED:
        {
            if (nParam1 == 1)
            {
                SetState(STATE_PROVISIONED);
            }
            NotifyRefreshCompleted();
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

            // Read all the subscriber configuration again.
            UpdateAllConfigs();

            if (IsIsimSupported())
            {
                SendMessage(ACMSG_REFRESH_ISIM_RECORDS, 0, 0);
            }
            else
            {
                CompleteProvisioning();
                SendMessage(ACMSG_REFRESH_COMPLETED, 0, 0);
            }
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

            IMS_TRACE_D("SubscriberConfig: Listener (%p) is removed", piListener, 0, 0);
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
            IMS_TRACE_D("SubscriberConfig: Listener (%p) is already set", piListener, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);

    IMS_TRACE_D("SubscriberConfig: Listener (%p) is set", piListener, 0, 0);

    if (IsServiceAllowed() && IsIsimSupported() &&
            (m_piIsim != IMS_NULL && m_piIsim->GetState() == IIsim::STATE_NOT_PRESENT))
    {
        SubscriberConfig* pSubsConfig = const_cast<SubscriberConfig*>(this);

        pSubsConfig->SendMessage(ACMSG_NOTIFY_ERROR, reinterpret_cast<IMS_SINTP>(piListener),
                ERROR_NO_ISIM_APPLICATION);
    }
}

PROTECTED VIRTUAL IMS_BOOL SubscriberConfig::ReadFrom()
{
    ICarrierConfig* piCc = GetCarrierConfig();

    m_nSubscriptionAttributes = ReadSubscriptionAttributes(piCc);
    m_objPcscfDiscoveryMethods = ReadPcscfDiscoveryMethods(piCc);

    IMS_SINT32 nPort = piCc->GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT);
    IImsPrivateProperty* piProperty = GetPrivateProperty();
    AString strPcscfAddressList = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST, GetSlotId());
    ImsList<AString> objPcscfAddresses = strPcscfAddressList.Split(',');

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

    if (IsDefaultConfig())
    {
        piProperty->SetPersistentBoolean(
                ImsPrivateProperties::Persistent::KEY_ISIM_ENABLED, IsIsimSupported(), GetSlotId());
        piProperty->SetPersistentBoolean(
                ImsPrivateProperties::Persistent::KEY_USIM_ENABLED, IsUsimSupported(), GetSlotId());

        // LOG_EXCLUDING_SERVER_INFO, only for default subscriber
        IMS_UTIL_SYS_PROP_SET_DEBUG_ON(IsDebugOn());
    }

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

        IMS_TRACE_D(
                "SubscriberConfig(%s:%s) is loaded", m_strConfName.GetStr(), m_strId.GetStr(), 0);

        return IMS_TRUE;
    }

    ImsSubscriberInfo* pSubsInfo = new ImsSubscriberInfo();

    if (IsDefaultConfig())
    {
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
    }
    else
    {
        const AString& strAnonymousUserId = ImsIdentity::GetAnonymousUserId();
        AString strTemp;
        strAnonymousUserId.SplitF('@', strTemp, pSubsInfo->m_strHomeDomainName, IMS_FALSE);
        strAnonymousUserId.SplitF(':', strTemp, pSubsInfo->m_strPrivateUserId, IMS_FALSE);
        pSubsInfo->m_nRefIndexOfPrimaryImpu = 0;
        pSubsInfo->m_objPublicUserIds.AddElement(strAnonymousUserId);
    }

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

            IMS_TRACE_D("SUBSCRIPTION_ATTRIBUTE_ALL: Updated (attributes=%08X)",
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

            IMS_TRACE_D("SUBSCRIPTION_ATTRIBUTE_ISIM: %s", _TRACE_B_(IsIsimSupported()), 0, 0);
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

            IMS_TRACE_D("SUBSCRIPTION_ATTRIBUTE_USIM: %s", _TRACE_B_(IsUsimSupported()), 0, 0);
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

            IMS_TRACE_D("HOME_DOMAIN_NAME: %s", GetLog(pSubsInfo->m_strHomeDomainName, 4).GetStr(),
                    0, 0);
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

            IMS_TRACE_D("IMPI: %s", GetLog(pSubsInfo->m_strPrivateUserId, 6).GetStr(), 0, 0);
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
                        "IMPU_PRIMARY_REF_INDEX: %d", pSubsInfo->m_nRefIndexOfPrimaryImpu, 0, 0);
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

            IMS_TRACE_D("IMPU_PRIMARY_REF_INDEX: %d", pSubsInfo->m_nRefIndexOfPrimaryImpu, 0, 0);
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

                IMS_TRACE_D("IMPU(%d): %s", nIndex, GetLog(strValue, 10).GetStr(), 0);
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

            IMS_TRACE_D("IMPU(%d): %s", nIndex, GetLog(strUserId, 10).GetStr(), 0);
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
                    "PHONE_CONTEXT: %s", GetLog(pSubsInfo->m_strPhoneContext, 4).GetStr(), 0, 0);
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

            IMS_TRACE_D("AUTH_USERNAME: %s",
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

            IMS_TRACE_D("AUTH_PASSWORD: %s",
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

            IMS_TRACE_D("AUTH_REALM: %s", GetLog(pSubsInfo->m_objCredential.GetRealm(), 4).GetStr(),
                    0, 0);
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

            IMS_TRACE_D("AUTH_ALGORITHM: %d", pSubsInfo->m_objCredential.GetType(), 0, 0);
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

            if (!strValue.IsNull())
            {
                pSubsInfo->m_strScscfAddress = strValue;
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();

            pSubsInfo->m_strScscfAddress = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME, GetSlotId());

            IMS_TRACE_D("SERVER_SCSCF: %s", GetLog(pSubsInfo->m_strScscfAddress, 4).GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_PCSCF_DISCOVERY_METHODS:
        {
            if (strValue.GetLength() > 0)
            {
                m_objPcscfDiscoveryMethods.Clear();

                ImsList<AString> objDiscoveryMethods = strValue.Split(',');

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
            ImsVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

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
                    IMS_TRACE_D("PCSCF_ADDRESS(%d): %s", nIndex, GetLog(strValue, 5).GetStr(), 0);
                }
                else
                {
                    IImsPrivateProperty* piProperty = GetPrivateProperty();
                    AString strPcscfAddressList = piProperty->GetPersistent(
                            ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST,
                            GetSlotId());
                    ImsList<AString> objPcscfAddresses = strPcscfAddressList.Split(',');
                    const AString& strPcscfAddress = (nIndex < objPcscfAddresses.GetSize())
                            ? objPcscfAddresses.GetAt(nIndex)
                            : AString::ConstNull();
                    pSa->SetAddress(strPcscfAddress);

                    IMS_TRACE_D("PCSCF_ADDRESS(%d): %s", nIndex,
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
            ImsVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

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

                    IMS_TRACE_D("PCSCF_PORT(%d): %d", nIndex, nPort, 0);
                }
                else
                {
                    ICarrierConfig* piCc = GetCarrierConfig();
                    IMS_SINT32 nPort = (piCc != IMS_NULL)
                            ? piCc->GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT)
                            : (-1);
                    pSa->SetPort(nPort);

                    IMS_TRACE_D("PCSCF_PORT(%d): %d", nIndex, pSa->GetPort(), 0);
                }
            }
            break;
        }
        case IConfigurable::CP_I_PCSCF_ALL:
        {
            ImsVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

            if (pPcscfAddresses->IsEmpty())
            {
                break;
            }

            IImsPrivateProperty* piProperty = GetPrivateProperty();
            AString strPcscfAddressList = piProperty->GetPersistent(
                    ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST, GetSlotId());
            ImsList<AString> objPcscfAddresses = strPcscfAddressList.Split(',');

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

                IMS_TRACE_D("PCSCF_ADDRESS_PORT(%d): %s (%d)", nIndex,
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

    ICarrierConfig* piCc = GetCarrierConfig();
    IMS_SINT32 nSubscriptionAttributes = ReadSubscriptionAttributes(piCc);
    IMS_BOOL bNewIsimSupported = (nSubscriptionAttributes & SUBSCRIPTION_ATTRIBUTE_ISIM) != 0;
    IMS_BOOL bOldIsimSupported = IsIsimSupported();

    if (bNewIsimSupported != bOldIsimSupported)
    {
        IMS_TRACE_I("ISIM enablement changed", 0, 0, 0);

        IMS_SINT32 nOldState = GetState();
        UpdateAllConfigs();

        if (IsIsimSupported())
        {
            if (nOldState == STATE_PROVISIONED)
            {
                IMS_TRACE_I("SubscriberConfig(%d): ISIM refresh started", nSlotId, 0, 0);
                SetState(STATE_REFRESHING);
                SendMessage(ACMSG_REFRESH_STARTED, 0, 0);
            }

            SendMessage(ACMSG_REFRESH_ISIM_RECORDS, 0, 0);
        }
        else
        {
            CompleteProvisioning();

            if (nOldState == STATE_PROVISIONED || nOldState == STATE_REFRESHING)
            {
                IMS_TRACE_I("SubscriberConfig(%d): Refresh started", nSlotId, 0, 0);
                SetState(STATE_REFRESHING);
                SendMessage(ACMSG_REFRESH_STARTED, 0, 0);
                SendMessage(ACMSG_REFRESH_COMPLETED, 1 /* with state change */, 0);
            }
            else
            {
                SendMessage(ACMSG_INIT_COMPLETED, 0, 0);
            }
        }
    }
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

    IMS_TRACE_I("SubscriberConfig(%s): Isim_OnStateChanged - %s on %s", strIdForLog.GetStr(),
            IsimStateToString(nState), StateToString(GetState()));

    switch (nState)
    {
        case IIsim::STATE_IDLE:
        {
            if (GetState() != STATE_INIT)
            {
                SetState(STATE_INACTIVE);
            }
            break;
        }
        case IIsim::STATE_NOT_PRESENT:
        {
            SetState(STATE_PROVISIONED);
            SendMessage(ACMSG_NOTIFY_ERROR, 0, ERROR_NO_ISIM_APPLICATION);
            break;
        }
        case IIsim::STATE_LOADED:
        {
            if (GetState() == STATE_REFRESHING)
            {
                UpdateIsimRecords();
            }
            else if (GetState() == STATE_PROVISIONED)
            {
                CallSubscriberInfoListener(SUBSCRIBER_INFO_REMOVE_ALL);
                SetState(STATE_REFRESHING);
                SendMessage(ACMSG_REFRESH_STARTED, 0, 0);
                SendMessage(ACMSG_UPDATE_ISIM_RECORDS, 0, 0);
            }
            else
            {
                if (GetState() == STATE_INACTIVE)
                {
                    SetState(STATE_PROVISIONING);
                }
                UpdateIsimRecords();
            }
            break;
        }
        case IIsim::STATE_REFRESHING:
        {
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
        default:
        {
            break;
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
void SubscriberConfig::InitIsim()
{
    IMS_TRACE_D("InitIsim: serviceAllowed=%s, isimSupported=%s", _TRACE_B_(IsServiceAllowed()),
            _TRACE_B_(IsIsimSupported()), 0);

    if (IsServiceAllowed() && IsIsimSupported())
    {
        if (m_piIsim == IMS_NULL)
        {
            m_piIsim = PhoneInfoService::GetPhoneInfoService()->GetIsim(GetSlotId());

            if (m_piIsim == IMS_NULL)
            {
                IMS_TRACE_E(0, "ISIM is null", 0, 0, 0);
                return;
            }

            m_piIsim->AddListener(this);
            // Initializes the ISIM module and waits for the ISIM state change.
            m_piIsim->Init();
        }
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
    IMS_TRACE_I("SubscriberConfig(%d): %s to %s", GetSlotId(), StateToString(m_nState),
            StateToString(nState));

    m_nState = nState;
}

PRIVATE
void SubscriberConfig::CompleteProvisioning()
{
    WriteProvisioning();
    CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);

    if (!IsIsimSupported())
    {
        // Notify configuration updates
        NotifyUpdate(IConfigurable::CP_I_SUBSCRIBER_ALL, m_strConfName, m_strId);
    }
}

/**
 * Refreshes the ISIM records when ISIM enablement is changed or ISIM refresh occurs.
 */
PRIVATE
void SubscriberConfig::RefreshIsimRecords()
{
    if (IsServiceAllowed() && IsIsimSupported())
    {
        if (m_piIsim == IMS_NULL)
        {
            InitIsim();
        }
        else if (m_piIsim->IsLoadCompleted())
        {
            UpdateIsimRecords();
        }
    }
}

/**
 * Updates the ISIM records when ISIM is in LOADED.
 */
PRIVATE
void SubscriberConfig::UpdateIsimRecords()
{
    IMS_TRACE_D("UpdateIsimRecords: serviceAllowed=%s, isimSupported=%s",
            _TRACE_B_(IsServiceAllowed()), _TRACE_B_(IsIsimSupported()), 0);

    if (IsServiceAllowed() && IsIsimSupported())
    {
        IMS_SINT32 nPrevState = GetState();

        UpdatePrivateUserIdentity();
        UpdatePublicUserIdentities();
        UpdateHomeDomainName();
        SetState(STATE_PROVISIONED);

        ToDebugString();
        CompleteProvisioning();

        if (nPrevState == STATE_PROVISIONING)
        {
            SendMessage(ACMSG_INIT_COMPLETED, 0, 0);
        }
        else
        {
            SendMessage(ACMSG_REFRESH_COMPLETED, 0, 0);
        }
    }
}

PRIVATE
void SubscriberConfig::UpdateHomeDomainName()
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

    if (pSubsInfo == IMS_NULL)
    {
        return;
    }

    pSubsInfo->m_strHomeDomainName = m_piIsim->GetHomeDomainName();
    pSubsInfo->m_strScscfAddress = pSubsInfo->m_strHomeDomainName;
    pSubsInfo->m_objCredential.SetRealm(pSubsInfo->m_strHomeDomainName);
}

PRIVATE
void SubscriberConfig::UpdatePrivateUserIdentity()
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

    if (pSubsInfo == IMS_NULL)
    {
        return;
    }

    pSubsInfo->m_strPrivateUserId = m_piIsim->GetImpi();
    pSubsInfo->m_objCredential.SetUsername(pSubsInfo->m_strPrivateUserId);
}

PRIVATE
void SubscriberConfig::UpdatePublicUserIdentities()
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

    if (pSubsInfo == IMS_NULL)
    {
        return;
    }

    AStringArray objPublicUserIds = m_piIsim->GetImpu();

    pSubsInfo->m_objPublicUserIds.RemoveAllElements();

    IMS_BOOL bUriSchemeRequiredWhenAbsent = IMS_TRUE;

    for (IMS_SINT32 i = 0; i < objPublicUserIds.GetCount(); ++i)
    {
        const AString& strPublicUserId = objPublicUserIds.GetElementAt(i);
        AString strUserId = strPublicUserId;

        // Remove the leading & trailing white spaces
        strUserId = strUserId.Trim();

        if (strPublicUserId.GetLength() != strUserId.GetLength())
        {
            IMS_TRACE_E(0, "IMPU: IMPU (in record at %d) contains the white spaces (%d >> %d)", i,
                    strPublicUserId.GetLength(), strUserId.GetLength());
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

    // Additional operation: set the primary IMPU
    SetPrimaryImpu(pSubsInfo);
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
}

PRIVATE
void SubscriberConfig::WriteProvisioning()
{
    if (!IsDefaultConfig())
    {
        return;
    }

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

        AString strPublicUserIds = pSubsInfo->m_objPublicUserIds.ToString();

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
        ImsVector<ServerAddress*>* pPcscfAddresses = &m_objPcscfAddresses;

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

    if (!IsDefaultConfig())
    {
        return nSubsAttributes;
    }

    ImsVector<IMS_SINT32> objIdentityPriorities =
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

PRIVATE GLOBAL ImsVector<IMS_SINT32> SubscriberConfig::ReadPcscfDiscoveryMethods(
        IN ICarrierConfig* piCc)
{
    ImsVector<IMS_SINT32> objPcscfDiscoveryMethods =
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
            return "ISIM_IDLE";
        case IIsim::STATE_NOT_PRESENT:
            return "ISIM_NOT_PRESENT";
        case IIsim::STATE_LOADED:
            return "ISIM_LOADED";
        case IIsim::STATE_REFRESHING:
            return "ISIM_REFRESHING";
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
        case STATE_PROVISIONED:
            return "STATE_PROVISIONED";
        case STATE_REFRESHING:
            return "STATE_REFRESHING";
        case STATE_INACTIVE:
            return "STATE_INACTIVE";
        default:
            return "__INVALID__";
    }
}
