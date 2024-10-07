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

    m_strId = (nIndex == AString::NPOS) ? GetDefaultId() : strConfName.GetSubStr(nIndex + 1);
    m_objPcscfDiscoveryMethods.Add(PCSCF_DISCOVERY_METHOD_PCO);

    m_pConfigurable = new Configurable(this);
}

PUBLIC VIRTUAL SubscriberConfig::~SubscriberConfig()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    piCc->RemoveListener(this);

    ClearPcscfAddresses();
    ClearSubscriberInfos();

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

PUBLIC VIRTUAL void SubscriberConfig::RemoveListener(IN ISubscriberConfigListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ImsList<ISubscriberConfigListener*>& objListeners = m_objListeners.GetValueAt(i);

        if (objListeners.Remove(piListener))
        {
            IMS_TRACE_D("SubscriberConfig: Listener (%p, 0x%x) is removed", piListener,
                    m_objListeners.GetKeyAt(i), 0);
        }
    }
}

PUBLIC VIRTUAL void SubscriberConfig::SetListener(
        IN ISubscriberConfigListener* piListener, IN IMS_SINT32 nEvents /*= LISTEN_EVENT_DEFAULT*/)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    ImsList<IMS_SINT32> objEvents = GetListenEvents(nEvents);

    for (IMS_UINT32 i = 0; i < objEvents.GetSize(); ++i)
    {
        IMS_SINT32 nEvent = objEvents.GetAt(i);
        IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nEvent);

        if (nIndex >= 0)
        {
            ImsList<ISubscriberConfigListener*>& objListeners = m_objListeners.GetValueAt(nIndex);

            if (!objListeners.Contains(piListener))
            {
                objListeners.Append(piListener);
            }
        }
        else
        {
            ImsList<ISubscriberConfigListener*> objListeners;
            objListeners.Append(piListener);
            m_objListeners.Add(nEvent, objListeners);
        }
    }

    IMS_TRACE_D("SubscriberConfig: Listener (%p, 0x%x) is set", piListener, nEvents, 0);

    if (objEvents.Contains(LISTEN_EVENT_ISIM_PROVISIONING) && IsServiceAllowed() &&
            IsIsimSupported() &&
            (m_piIsim != IMS_NULL && m_piIsim->GetState() == IIsim::STATE_NOT_PRESENT))
    {
        SendMessage(ACMSG_NOTIFY_ERROR, reinterpret_cast<IMS_SINTP>(piListener),
                ERROR_NO_ISIM_APPLICATION);
    }
}

PUBLIC VIRTUAL void SubscriberConfig::EnableIsim()
{
    IMS_BOOL bOldIsimSupported = IsIsimSupported();
    SetOrClearSubscriptionAttributes(IMS_TRUE, SUBSCRIPTION_ATTRIBUTE_ISIM);
    SetOrClearSubscriptionAttributes(IMS_FALSE, SUBSCRIPTION_ATTRIBUTE_USIM);

    if (!bOldIsimSupported && IsIsimSupported())
    {
        CallSubscriberInfoListener(SUBSCRIBER_INFO_REMOVE_ALL);
        ClearSubscriberInfos();

        // Start ISIM provisioning.
        if (GetState() == STATE_PROVISIONED)
        {
            IMS_TRACE_I("SubscriberConfig(%d): EnableIsim", GetSlotId(), 0, 0);
            SetState(STATE_REFRESHING);
            SendMessage(ACMSG_REFRESH_STARTED, LISTEN_EVENT_ISIM_PROVISIONING, 0);
        }

        SendMessage(ACMSG_REFRESH_ISIM_RECORDS, 0, 0);
    }
}

PUBLIC VIRTUAL void SubscriberConfig::UpdateSubscriberInfo(IN const AString& strHomeDomainName,
        IN const AString& strPrivateUserId, IN const AString& strPublicUserId,
        IN IMS_BOOL bIsimEnabled /*= IMS_FALSE*/)
{
    AStringArray objPublicUserIds;
    objPublicUserIds.AddElement(strPublicUserId);
    UpdateSubscriberInfo(strHomeDomainName, strPrivateUserId, objPublicUserIds, bIsimEnabled);
}

PUBLIC VIRTUAL void SubscriberConfig::UpdateSubscriberInfo(IN const AString& strHomeDomainName,
        IN const AString& strPrivateUserId, IN const AStringArray& objPublicUserIds,
        IN IMS_BOOL bIsimEnabled /*= IMS_FALSE*/)
{
    CallSubscriberInfoListener(SUBSCRIBER_INFO_REMOVE_ALL);
    ClearSubscriberInfos();

    SetOrClearSubscriptionAttributes(bIsimEnabled, SUBSCRIPTION_ATTRIBUTE_ISIM);
    SetOrClearSubscriptionAttributes(!bIsimEnabled, SUBSCRIPTION_ATTRIBUTE_USIM);

    ImsSubscriberInfo* pSubsInfo = CreateSubscriberInfo();

    UpdatePrivateUserId(pSubsInfo, strPrivateUserId);
    UpdatePublicUserIds(pSubsInfo, objPublicUserIds);
    UpdateHomeDomainName(pSubsInfo, strHomeDomainName);

    if (bIsimEnabled)
    {
        // Even though ISIM is enabled, the index of the primary IMPU indicates a first position.
        pSubsInfo->m_nRefIndexOfPrimaryImpu = 0;
    }

    IMS_SINT32 nOldState = GetState();
    SetState(STATE_PROVISIONED);
    StoreSubscriberInfo();
    CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);

    IMS_TRACE_I("SubscriberConfig(%d): UpdateSubscriberInfo", GetSlotId(), 0, 0);

    if (nOldState == STATE_PROVISIONED || nOldState == STATE_REFRESHING)
    {
        SendMessage(ACMSG_REFRESH_STARTED_N_COMPLETED, LISTEN_EVENT_MANUAL_PROVISIONING, 0);
    }
    else
    {
        SendMessage(ACMSG_INIT_COMPLETED, LISTEN_EVENT_MANUAL_PROVISIONING, 0);
    }
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
            NotifyInitCompleted(LONG_TO_SINT(nParam1));
            break;
        }
        case ACMSG_REFRESH_COMPLETED:
        {
            NotifyRefreshCompleted(LONG_TO_SINT(nParam1));
            break;
        }
        case ACMSG_REFRESH_STARTED:
        {
            NotifyRefreshStarted(LONG_TO_SINT(nParam1));
            break;
        }
        case ACMSG_REFRESH_STARTED_N_COMPLETED:
        {
            NotifyRefreshStarted(LONG_TO_SINT(nParam1));
            NotifyRefreshCompleted(LONG_TO_SINT(nParam1));
            break;
        }
        case ACMSG_NOTIFY_ERROR:
        {
            if (nParam1 == 0)
            {
                NotifyError(LISTEN_EVENT_ISIM_PROVISIONING, LONG_TO_SINT(nParam2));
            }
            else
            {
                ISubscriberConfigListener* piNewListener =
                        reinterpret_cast<ISubscriberConfigListener*>(nParam1);

                NotifyError(LISTEN_EVENT_ISIM_PROVISIONING, LONG_TO_SINT(nParam2), piNewListener);
            }
            break;
        }
        case ACMSG_UPDATE_ALL_CONFIGS:
        {
            SetState(STATE_REFRESHING);
            NotifyRefreshStarted(LISTEN_EVENT_MANUAL_PROVISIONING);

            // Read all the subscriber configuration again.
            UpdateAllConfigs();
            break;
        }
        default:
        {
            break;
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL SubscriberConfig::ReadFrom()
{
    ICarrierConfig* piCc = GetCarrierConfig();

    m_nSubscriptionAttributes = ReadSubscriptionAttributes(piCc);
    m_objPcscfDiscoveryMethods = ReadPcscfDiscoveryMethods(piCc);
    UpdatePcscfAddresses();

    if (IsDefaultConfig())
    {
        // LOG_EXCLUDING_SERVER_INFO, only for default subscriber
        IMS_UTIL_SYS_PROP_SET_DEBUG_ON(IsDebugOn());
    }

    ImsSubscriberInfo* pSubsInfo = CreateSubscriberInfo();

    // Device supports the ISIM application, so the subscriber information should be read from ISIM.
    if (IsServiceAllowed() && IsIsimSupported())
    {
        IMS_TRACE_D("ISIM is enabled", 0, 0, 0);

        StoreSubscriberInfo();

        if (GetState() != STATE_REFRESHING)
        {
            SetState(STATE_PROVISIONING);
        }

        IMS_TRACE_D(
                "SubscriberConfig(%s:%s) is loaded", m_strConfName.GetStr(), m_strId.GetStr(), 0);

        return IMS_TRUE;
    }

    AString strHomeDomainName;
    AString strPrivateUserId;
    AStringArray objPublicUserIds;

    if (IsDefaultConfig())
    {
        IImsPrivateProperty* piProperty = GetPrivateProperty();

        strHomeDomainName = piProperty->GetPersistent(
                ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME, GetSlotId());

        strPrivateUserId = piProperty->GetPersistent(
                ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI, GetSlotId());

        AString strPublicUserIds = piProperty->GetPersistent(
                ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST, GetSlotId());

        objPublicUserIds = strPublicUserIds.Split(',');
    }
    else
    {
        const AString& strAnonymousUserId = ImsIdentity::GetAnonymousUserId();
        AString strTemp;
        strAnonymousUserId.SplitF('@', strTemp, strHomeDomainName, IMS_FALSE);
        strAnonymousUserId.SplitF(':', strTemp, strPrivateUserId, IMS_FALSE);

        AString strPublicUserId;
        strPublicUserId.Sprintf("\"Anonymous\" <%s>", strAnonymousUserId.GetStr());
        objPublicUserIds.AddElement(strPublicUserId);
    }

    UpdatePrivateUserId(pSubsInfo, strPrivateUserId);
    UpdatePublicUserIds(pSubsInfo, objPublicUserIds);
    UpdateHomeDomainName(pSubsInfo, strHomeDomainName);

    SetState(STATE_PROVISIONED);
    StoreSubscriberInfo();
    CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);
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
            StoreSubscriberInfo();
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

    m_objPcscfDiscoveryMethods = ReadPcscfDiscoveryMethods(piCc);
    ClearPcscfAddresses();
    UpdatePcscfAddresses();

    IMS_SINT32 nRefIndexOfPrimaryImpu = IsIsimSupported()
            ? piCc->GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT, 1)
            : 0;
    AString strPhoneContext = IsDefaultConfig()
            ? piCc->GetString(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING)
            : AString::ConstNull();

    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

    if (pSubsInfo != IMS_NULL)
    {
        pSubsInfo->m_strPhoneContext = strPhoneContext;

        if (pSubsInfo->m_nRefIndexOfPrimaryImpu != nRefIndexOfPrimaryImpu)
        {
            pSubsInfo->m_nRefIndexOfPrimaryImpu = nRefIndexOfPrimaryImpu;

            if (IsDefaultConfig() && pSubsInfo->m_objPublicUserIds.GetCount() > 0)
            {
                StorePrimaryPublicUserId();
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
                SendMessage(ACMSG_REFRESH_STARTED, LISTEN_EVENT_ISIM_PROVISIONING, 0);
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

            SendMessage(ACMSG_REFRESH_STARTED, LISTEN_EVENT_ISIM_PROVISIONING, 0);
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
void SubscriberConfig::ClearPcscfAddresses()
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
}

PRIVATE
void SubscriberConfig::ClearSubscriberInfos()
{
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
ImsSubscriberInfo* SubscriberConfig::CreateSubscriberInfo()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    ImsSubscriberInfo* pSubsInfo = new ImsSubscriberInfo();

    pSubsInfo->m_nRefIndexOfPrimaryImpu = IsIsimSupported()
            ? piCc->GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT, 1)
            : 0;
    pSubsInfo->m_strPhoneContext = IsDefaultConfig()
            ? piCc->GetString(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING)
            : AString::ConstNull();
    pSubsInfo->m_bIsAuthRealmLenient = IMS_TRUE;
    pSubsInfo->m_objCredential.SetType(Credential::TYPE_AKAv1_MD5);
    pSubsInfo->m_objCredential.SetPassword(AString::ConstNull());

    m_objSubscriberInfos.Append(pSubsInfo);

    return pSubsInfo;
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
        ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();

        if (pSubsInfo == IMS_NULL)
        {
            pSubsInfo = CreateSubscriberInfo();
        }

        UpdatePrivateUserId(pSubsInfo, m_piIsim->GetImpi());
        UpdatePublicUserIds(pSubsInfo, m_piIsim->GetImpu());
        UpdateHomeDomainName(pSubsInfo, m_piIsim->GetHomeDomainName());

        IMS_SINT32 nOldState = GetState();
        SetState(STATE_PROVISIONED);
        StoreSubscriberInfo();
        CallSubscriberInfoListener(SUBSCRIBER_INFO_ADD);
        ToDebugString();

        if (nOldState == STATE_PROVISIONING)
        {
            SendMessage(ACMSG_INIT_COMPLETED, LISTEN_EVENT_ISIM_PROVISIONING, 0);
        }
        else
        {
            SendMessage(ACMSG_REFRESH_COMPLETED, LISTEN_EVENT_ISIM_PROVISIONING, 0);
        }
    }
}

PRIVATE
void SubscriberConfig::UpdateHomeDomainName(
        IN ImsSubscriberInfo* pSubsInfo, IN const AString& strHomeDomainName)
{
    pSubsInfo->m_strHomeDomainName = strHomeDomainName;
    pSubsInfo->m_strScscfAddress = pSubsInfo->m_strHomeDomainName;
    pSubsInfo->m_objCredential.SetRealm(pSubsInfo->m_strHomeDomainName);
}

PRIVATE
void SubscriberConfig::UpdatePrivateUserId(
        IN ImsSubscriberInfo* pSubsInfo, IN const AString& strPrivateUserId)
{
    pSubsInfo->m_strPrivateUserId = strPrivateUserId;
    pSubsInfo->m_objCredential.SetUsername(pSubsInfo->m_strPrivateUserId);
}

PRIVATE
void SubscriberConfig::UpdatePublicUserIds(
        IN ImsSubscriberInfo* pSubsInfo, IN const AStringArray& objPublicUserIds)
{
    ICarrierConfig* piCc = GetCarrierConfig();
    IMS_SINT32 nRefIndexOfPrimaryImpu = IsIsimSupported()
            ? piCc->GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT, 1)
            : 0;
    pSubsInfo->m_nRefIndexOfPrimaryImpu = nRefIndexOfPrimaryImpu;

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
        IMS_SINT32 nStartIndex = strUserId.GetIndexOf('<') + 1;

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
void SubscriberConfig::UpdatePcscfAddresses()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    IMS_SINT32 nPort = piCc->GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT);
    IImsPrivateProperty* piProperty = GetPrivateProperty();
    AString strPcscfAddressList = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST, GetSlotId());
    ImsList<AString> objPcscfAddresses = strPcscfAddressList.Split(',');

    for (IMS_UINT32 i = 0; i < objPcscfAddresses.GetSize(); ++i)
    {
        m_objPcscfAddresses.Add(new ServerAddress(objPcscfAddresses.GetAt(i), nPort));
    }
}

PRIVATE
void SubscriberConfig::NotifyInitCompleted(IN IMS_SINT32 nEvent)
{
    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nEvent);

    if (nIndex >= 0)
    {
        ImsList<ISubscriberConfigListener*>& objListeners = m_objListeners.GetValueAt(nIndex);

        for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
        {
            ISubscriberConfigListener* piListener = objListeners.GetAt(i);
            piListener->SubscriberConfig_InitCompleted();
        }
    }
}

PRIVATE
void SubscriberConfig::NotifyRefreshCompleted(IN IMS_SINT32 nEvent)
{
    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nEvent);

    if (nIndex >= 0)
    {
        ImsList<ISubscriberConfigListener*>& objListeners = m_objListeners.GetValueAt(nIndex);

        for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
        {
            ISubscriberConfigListener* piListener = objListeners.GetAt(i);
            piListener->SubscriberConfig_RefreshCompleted();
        }
    }
}

PRIVATE
void SubscriberConfig::NotifyRefreshStarted(IN IMS_SINT32 nEvent)
{
    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nEvent);

    if (nIndex >= 0)
    {
        ImsList<ISubscriberConfigListener*>& objListeners = m_objListeners.GetValueAt(nIndex);

        for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
        {
            ISubscriberConfigListener* piListener = objListeners.GetAt(i);
            piListener->SubscriberConfig_RefreshStarted();
        }
    }
}

PRIVATE
void SubscriberConfig::NotifyError(IN IMS_SINT32 nEvent, IN IMS_SINT32 nErrorCode,
        IN ISubscriberConfigListener* piTargetListener /*= IMS_NULL*/)
{
    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nEvent);

    if (nIndex >= 0)
    {
        ImsList<ISubscriberConfigListener*>& objListeners = m_objListeners.GetValueAt(nIndex);

        if (piTargetListener == IMS_NULL)
        {
            for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
            {
                ISubscriberConfigListener* piListener = objListeners.GetAt(i);
                piListener->SubscriberConfig_NotifyError(nErrorCode);
            }
        }
        else if (objListeners.Contains(piTargetListener))
        {
            piTargetListener->SubscriberConfig_NotifyError(nErrorCode);
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
    ClearPcscfAddresses();
    ClearSubscriberInfos();

    IMS_SINT32 nOldState = GetState();

    if (!ReadFrom())
    {
        IMS_TRACE_E(0, "Updating all the configs (%s) failed", m_strConfName.GetStr(), 0, 0);
    }

    if (IsIsimSupported())
    {
        SendMessage(ACMSG_REFRESH_ISIM_RECORDS, 0, 0);
    }
    else
    {
        if (nOldState == STATE_INIT)
        {
            SendMessage(ACMSG_INIT_COMPLETED, LISTEN_EVENT_MANUAL_PROVISIONING, 0);
        }
        else
        {
            NotifyUpdate(IConfigurable::CP_I_SUBSCRIBER_ALL, m_strConfName, m_strId);
            SendMessage(ACMSG_REFRESH_COMPLETED, LISTEN_EVENT_MANUAL_PROVISIONING, 0);
        }
    }
}

PRIVATE
void SubscriberConfig::StorePrimaryPublicUserId()
{
    ImsSubscriberInfo* pSubsInfo = GetSubscriberInfoEx();
    IMS_SINT32 nPrimaryImpuIndex = 0;

    if (IsIsimSupported())
    {
        nPrimaryImpuIndex = pSubsInfo->m_nRefIndexOfPrimaryImpu;

        if (pSubsInfo->m_objPublicUserIds.GetCount() <= nPrimaryImpuIndex)
        {
            nPrimaryImpuIndex = 0;
        }
    }

    IImsPrivateProperty* piProperty = GetPrivateProperty();
    piProperty->SetPersistent(ImsPrivateProperties::Persistent::KEY_PRIMARY_IMPU,
            pSubsInfo->m_objPublicUserIds.GetElementAt(nPrimaryImpuIndex), GetSlotId());
}

PRIVATE
void SubscriberConfig::StoreSubscriberInfo()
{
    if (!IsDefaultConfig())
    {
        return;
    }

    IMS_TRACE_D("StoreSubscriberInfo", 0, 0, 0);

    IImsPrivateProperty* piProperty = GetPrivateProperty();

    piProperty->SetPersistentBoolean(
            ImsPrivateProperties::Persistent::KEY_ISIM_ENABLED, IsIsimSupported(), GetSlotId());
    piProperty->SetPersistentBoolean(
            ImsPrivateProperties::Persistent::KEY_USIM_ENABLED, IsUsimSupported(), GetSlotId());

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

        if (pSubsInfo->m_objPublicUserIds.GetCount() > 0)
        {
            AString strPublicUserIds = pSubsInfo->m_objPublicUserIds.ToString();

            piProperty->SetPersistent(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST,
                    strPublicUserIds, GetSlotId());

            StorePrimaryPublicUserId();
        }
    }
}

PRIVATE
void SubscriberConfig::ToDebugString()
{
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

PRIVATE GLOBAL ImsList<IMS_SINT32> SubscriberConfig::GetListenEvents(IN IMS_SINT32 nEvents)
{
    ImsList<IMS_SINT32> objEvents;

    if ((nEvents & LISTEN_EVENT_ISIM_PROVISIONING) != 0)
    {
        objEvents.Append(LISTEN_EVENT_ISIM_PROVISIONING);
    }

    if ((nEvents & LISTEN_EVENT_MANUAL_PROVISIONING) != 0)
    {
        objEvents.Append(LISTEN_EVENT_MANUAL_PROVISIONING);
    }

    return objEvents;
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
