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
#ifndef SUBSCRIBER_CONFIG_H_
#define SUBSCRIBER_CONFIG_H_

#include "IIsimListener.h"

#include "CarrierConfig.h"
#include "ISubscriberConfig.h"
#include "private/ConfigBase.h"
#include "private/ImsSubscriberInfo.h"

class IIsim;

class AsyncConfigHelper;
class ISubscriberInfoListener;

class SubscriberConfig : public ConfigBase, public ISubscriberConfig, public IIsimListener
{
public:
    explicit SubscriberConfig(IN IMS_SINT32 nSlotId, IN const AString& strConfName);
    virtual ~SubscriberConfig();

    SubscriberConfig(IN const SubscriberConfig&) = delete;
    SubscriberConfig& operator=(IN const SubscriberConfig&) = delete;

public:
    // ISubscriberConfig class
    inline ServerAddress* GetPcscfAddress() const override
    {
        return m_objPcscfAddresses.IsEmpty() ? IMS_NULL : m_objPcscfAddresses.GetAt(0);
    }
    inline const ImsVector<ServerAddress*>& GetPcscfAddresses() const override
    {
        return m_objPcscfAddresses;
    }
    inline IMS_SINT32 GetPcscfDiscoveryMethod() const override
    {
        return m_objPcscfDiscoveryMethods.IsEmpty() ? PCSCF_DISCOVERY_METHOD_PCO
                                                    : m_objPcscfDiscoveryMethods.GetAt(0);
    }
    inline const ImsVector<IMS_SINT32>& GetPcscfDiscoveryMethods() const override
    {
        return m_objPcscfDiscoveryMethods;
    }
    inline IMS_SINT32 GetSubscriberCount() const override
    {
        return static_cast<IMS_SINT32>(m_objSubscriberInfos.GetSize());
    }
    IImsSubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nIndex = 0) const override;
    IMS_BOOL IsAkaSupported() const override;
    inline IMS_BOOL IsDebugOn() const override
    {
        return IsSubscriptionAttributeEnabled(SUBSCRIPTION_ATTRIBUTE_DEBUG);
    }
    inline IMS_BOOL IsServiceAllowed() const override
    {
        return IsSubscriptionAttributeEnabled(SUBSCRIPTION_ATTRIBUTE_IMS);
    }
    inline IMS_BOOL IsIsimSupported() const override
    {
        return IsSubscriptionAttributeEnabled(SUBSCRIPTION_ATTRIBUTE_ISIM);
    }
    inline IMS_BOOL IsProvisioningDone() const override
    {
        return (GetState() == STATE_PROVISIONED);
    }
    inline IMS_BOOL IsUsimSupported() const override
    {
        return IsSubscriptionAttributeEnabled(SUBSCRIPTION_ATTRIBUTE_USIM);
    }
    inline IMS_BOOL IsTestMode() const override
    {
        return IsSubscriptionAttributeEnabled(SUBSCRIPTION_ATTRIBUTE_TESTMODE);
    }
    inline IMS_SINT32 GetSubscriptionAttributes() const override
    {
        return m_nSubscriptionAttributes;
    }
    inline IConfigurable* GetConfigurable() const override { return m_pConfigurable; }
    void RemoveListener(IN ISubscriberConfigListener* piListener) override;
    void SetListener(IN ISubscriberConfigListener* piListener,
            IN IMS_SINT32 nEvents = LISTEN_EVENT_DEFAULT) override;
    void EnableIsim() override;
    void UpdateSubscriberInfo(IN const AString& strHomeDomainName,
            IN const AString& strPrivateUserId, IN const AString& strPublicUserId,
            IN IMS_BOOL bIsimEnabled = IMS_FALSE) override;
    void UpdateSubscriberInfo(IN const AString& strHomeDomainName,
            IN const AString& strPrivateUserId, IN const AStringArray& objPublicUserIds,
            IN IMS_BOOL bIsimEnabled = IMS_FALSE) override;

    //// APIs for the values of a default ImsSubscriberInfo
    const Credential& GetCredential() const override;
    const AString& GetHomeDomainName() const override;
    IMS_SINT32 GetIndexOfPrimaryPublicUserId() const override;
    const AString& GetPhoneContext() const override;
    const AString& GetPrivateUserId() const override;
    const AString& GetPublicUserId(
            IN IMS_SINT32 nImpuType = IImsSubscriberInfo::IMPU_REF_INDEX) const override;
    const AStringArray& GetPublicUserIds() const override;

    // ConfigBase class
    IMS_BOOL Init() override;
    void Refresh() override;

    const AString& GetScscfAddress() const;
    IMS_BOOL IsAuthRealmLenient() const;
    ImsSubscriberInfo* GetSubscriberInfoEx(IN IMS_SINT32 nIndex = 0) const;
    inline const AString& GetConfName() const { return m_strConfName; }
    inline const AString& GetId() const { return m_strId; }
    inline void SetSubscriberInfoListener(IN ISubscriberInfoListener* piListener)
    {
        m_piSubsInfoListener = piListener;
    }

    inline IMS_BOOL IsDefaultConfig() const { return m_strId.Equals(GetDefaultId()); }
    static const AString& GetDefaultId();

protected:
    // IAsyncConfig class
    void HandleMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2) override;

    // ConfigBase class
    IMS_BOOL ReadFrom() override;
    IMS_BOOL Update(IN IMS_SINT32 nCpi, IN const AString& strValue = AString::ConstNull()) override;
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

private:
    // IIsimListener class
    void Isim_OnStateChanged(IN IMS_SINT32 nState) override;

    inline IMS_BOOL IsSubscriptionAttributeEnabled(IN IMS_SINT32 nAttribute) const
    {
        return (m_nSubscriptionAttributes & nAttribute) != 0;
    }
    inline void ClearSubscriptionAttributes(IN IMS_SINT32 nAttributes)
    {
        m_nSubscriptionAttributes &= (~nAttributes);
    }
    inline void SetSubscriptionAttributes(IN IMS_SINT32 nAttributes)
    {
        m_nSubscriptionAttributes |= nAttributes;
    }
    void SetOrClearSubscriptionAttributes(IN IMS_BOOL bEnabled, IN IMS_SINT32 nAttributes);

    void CallSubscriberInfoListener(IN IMS_SINT32 nSubsInfo);
    void ClearPcscfAddresses();
    void ClearSubscriberInfos();
    const AString& GetLog(IN const AString& strValue, IN IMS_SINT32 nCount) const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    ImsSubscriberInfo* CreateSubscriberInfo();
    void InitIsim();
    void SetPrimaryImpu(IN ImsSubscriberInfo* pSubsInfo);
    void SetState(IN IMS_SINT32 nState);
    void RefreshIsimRecords();
    void UpdateIsimRecords();
    void UpdateHomeDomainName(IN ImsSubscriberInfo* pSubsInfo, IN const AString& strHomeDomainName);
    void UpdatePrivateUserId(IN ImsSubscriberInfo* pSubsInfo, IN const AString& strPrivateUserId);
    void UpdatePublicUserIds(
            IN ImsSubscriberInfo* pSubsInfo, IN const AStringArray& objPublicUserIds);
    void UpdatePcscfAddresses();
    void NotifyInitCompleted(IN IMS_SINT32 nEvent);
    void NotifyRefreshCompleted(IN IMS_SINT32 nEvent);
    void NotifyRefreshStarted(IN IMS_SINT32 nEvent);
    void NotifyError(IN IMS_SINT32 nEvent, IN IMS_SINT32 nErrorCode,
            IN ISubscriberConfigListener* piTargetListener = IMS_NULL);

    void SendMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2);
    void UpdateAllConfigs();
    void StorePrimaryPublicUserId();
    void StoreSubscriberInfo();
    void ToDebugString();

    IMS_SINT32 ReadSubscriptionAttributes(IN ICarrierConfig* piCc);
    static ImsVector<IMS_SINT32> ReadPcscfDiscoveryMethods(IN ICarrierConfig* piCc);
    static ImsList<IMS_SINT32> GetListenEvents(IN IMS_SINT32 nEvents);

    static const IMS_CHAR* IsimStateToString(IN IMS_SINT32 nState);
    static const IMS_CHAR* PcscfDiscoveryMethodToString(IN IMS_SINT32 nMethod);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    /// State of SubscriberConfig
    enum
    {
        /// Initial state.
        STATE_INIT = 0,
        /// When ISIM is enabled and reading the ISIM records is in progress.
        STATE_PROVISIONING,
        /// When all the provisioning data is completely read and IMS service can be started.
        STATE_PROVISIONED,
        /// When ISIM refresh occurs or SIM is swapped on the runtime.
        STATE_REFRESHING,
        /// When IMS service of a specific slot is stopped.
        STATE_INACTIVE
    };

    enum
    {
        ACMSG_UPDATE_ISIM_RECORDS = (ACMSG_USER + 1),
        ACMSG_REFRESH_ISIM_RECORDS,
        ACMSG_INIT_COMPLETED,
        ACMSG_REFRESH_COMPLETED,
        ACMSG_REFRESH_STARTED,
        ACMSG_REFRESH_STARTED_N_COMPLETED,
        ACMSG_NOTIFY_ERROR,
        ACMSG_UPDATE_ALL_CONFIGS
    };

    /// SubscriberInfo types
    enum
    {
        SUBSCRIBER_INFO_NONE = 0,
        SUBSCRIBER_INFO_ADD,
        SUBSCRIBER_INFO_REMOVE,
        SUBSCRIBER_INFO_REMOVE_ALL
    };

private:
    AsyncConfigHelper* m_pConfigHelper;

    // Identifier of this subscriber configuration
    AString m_strId;
    // To support a multiple subscriber configuration
    AString m_strConfName;

    // ISubscriberConfig#SUBSCRIPTION_ATTRIBUTE_XXX
    IMS_SINT32 m_nSubscriptionAttributes;
    // P-CSCF discovery methods w/ priority (provisioned by int-array)
    // : first one is top priority
    ImsVector<IMS_SINT32> m_objPcscfDiscoveryMethods;
    // The list of P-CSCF address is provided when the P-CSCF discovery method
    // is PCSCF_DISCOVERY_METHOD_CONFIG.
    ImsVector<ServerAddress*> m_objPcscfAddresses;

    IIsim* m_piIsim;
    IMS_SINT32 m_nState;
    ImsList<ImsSubscriberInfo*> m_objSubscriberInfos;

    // <Event, List of ISubscriberConfigListener>
    ImsMap<IMS_SINT32, ImsList<ISubscriberConfigListener*>> m_objListeners;
    ISubscriberInfoListener* m_piSubsInfoListener;

    // Configurable class
    Configurable* m_pConfigurable;

    // Logging
    mutable AString m_strLog;
};

#endif
