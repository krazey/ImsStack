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
#include "ITimer.h"

#include "CarrierConfig.h"
#include "ISubscriberConfig.h"
#include "private/ConfigBase.h"
#include "private/ImsSubscriberInfo.h"

class IImsPrivateProperty;
class IIsim;

class AsyncConfigHelper;
class ICarrierConfig;
class ISubscriberInfoListener;

class SubscriberConfig :
        public ConfigBase,
        public ISubscriberConfig,
        public IIsimListener,
        public ITimerListener
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
    inline const IMSVector<ServerAddress*>& GetPcscfAddresses() const override
    {
        return m_objPcscfAddresses;
    }
    inline IMS_SINT32 GetPcscfDiscoveryMethod() const override
    {
        return m_objPcscfDiscoveryMethods.IsEmpty() ? PCSCF_DISCOVERY_METHOD_PCO
                                                    : m_objPcscfDiscoveryMethods.GetAt(0);
    }
    inline const IMSVector<IMS_SINT32>& GetPcscfDiscoveryMethods() const override
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

    static const AString& GetDefaultId();

protected:
    // IAsyncConfig class
    void HandleMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2) override;

    // ISubscriberConfig class
    inline IMS_SINT32 GetSubscriptionAttributes() const override
    {
        return m_nSubscriptionAttributes;
    }
    inline IConfigurable* GetConfigurable() const override { return m_pConfigurable; }
    void RemoveListener(IN ISubscriberConfigListener* piListener) const override;
    void SetListener(IN ISubscriberConfigListener* piListener) const override;

    // ConfigBase class
    IMS_BOOL ReadFrom() override;
    IMS_BOOL Update(IN IMS_SINT32 nCpi, IN const AString& strValue = AString::ConstNull()) override;
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

private:
    // IIsimListener class
    void Isim_OnField(IN IMS_SINT32 nField, IN const IMSList<ByteArray>& objValues) override;
    void Isim_OnHomeDomainName(IN const ByteArray& objDomainName) override;
    void Isim_OnImpi(IN const ByteArray& objPrivateUserId) override;
    void Isim_OnImpu(IN const IMSList<ByteArray>& objPublicUserIds) override;
    void Isim_OnError(IN IMS_SINT32 nErrorCode) override;
    void Isim_OnStateChanged(IN IMS_SINT32 nState) override;

    // ITimerListener class
    void Timer_TimerExpired(IN ITimer* piTimer) override;

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
    void ClearPcscfAddressAndSubscriberInfo();
    const AString& GetLog(IN const AString& strValue, IN IMS_SINT32 nCount) const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    void InitProvisioning();
    void SetPrimaryImpu(IN ImsSubscriberInfo* pSubsInfo);
    void SetState(IN IMS_SINT32 nState);
    void StartProvisioning(IN IMS_BOOL bIsRefresh = IMS_FALSE);

    inline IMS_BOOL IsIsimProvisioningDone() const
    {
        return (m_nIsimRecords & m_nConfiguredIsimRecords) == m_nConfiguredIsimRecords;
    }
    void ReadIsimProvisioning();
    void RecoverIsimProvisioning(IN IMS_SINT32 nErrorCode);
    void RefreshIsimProvisioning();

    inline IMS_BOOL IsIsimRecordSet(IN IMS_SINT32 nRecord) const
    {
        return ((m_nIsimRecords & nRecord) == nRecord);
    }
    inline void ResetIsimRecord(IN IMS_SINT32 nRecord) { m_nIsimRecords &= (~nRecord); }
    inline void SetIsimRecord(IN IMS_SINT32 nRecord) { m_nIsimRecords |= nRecord; }

    void NotifyInitCompleted();
    void NotifyRefreshCompleted();
    void NotifyRefreshStarted();
    void NotifyError(
            IN IMS_SINT32 nErrorCode, IN ISubscriberConfigListener* piTargetListener = IMS_NULL);

    void SendMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2);
    void UpdateAllConfigs();
    void WriteProvisioning();
    void ToDebugString();

    IMS_SINT32 ReadSubscriptionAttributes(IN ICarrierConfig* piCc);
    static IMSVector<IMS_SINT32> ReadPcscfDiscoveryMethods(IN ICarrierConfig* piCc);

    static const IMS_CHAR* IsimStateToString(IN IMS_SINT32 nState);
    static const IMS_CHAR* PcscfDiscoveryMethodToString(IN IMS_SINT32 nMethod);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    /// State of SubscriberConfig
    enum
    {
        /// Initial state
        STATE_INIT = 0,
        /// It is in the STATE_PROVISIONING, while reading the provisioning data from ISIM.
        STATE_PROVISIONING,
        /// It is in the STATE_REFRESHING, while reading the provisioning data from ISIM
        /// by ISIM refresh.
        STATE_REFRESHING,
        /// All the provisioning data is read and IMS service can be started.
        STATE_PROVISIONED
    };

    /// ISIM records to be read
    /// Read order: IMPI -> IMPU -> DOMAIN
    enum
    {
        ISIM_NONE = 0x0000,
        ISIM_IMPI = 0x0001,
        ISIM_IMPU = 0x0002,
        ISIM_DOMAIN = 0x0004,
        ISIM_IST = 0x0008,
        ISIM_PCSCF = 0x0010,
        ISIM_DONE = (ISIM_IMPI | ISIM_IMPU | ISIM_DOMAIN)
    };

    enum
    {
        ACMSG_START_PROVISIONING = (ACMSG_USER + 1),
        ACMSG_READ_ISIM_RECORD,
        ACMSG_INIT_COMPLETED,
        ACMSG_REFRESH_COMPLETED,
        ACMSG_REFRESH_STARTED,
        ACMSG_NOTIFY_ERROR,
        ACMSG_UPDATE_ALL_CONFIGS,
        ACMSG_INIT_RETRY_TIMER,
        ACMSG_START_RETRY_TIMER,
        ACMSG_RECOVERY_REQUIRED,
        ACMSG_UPDATE_ALL_PCSCF,
        ACMSG_REFRESH_ISIM_PROVISIONING
    };

    /// SubscriberInfo types
    enum
    {
        SUBSCRIBER_INFO_NONE = 0,
        SUBSCRIBER_INFO_ADD,
        SUBSCRIBER_INFO_REMOVE,
        SUBSCRIBER_INFO_REMOVE_ALL
    };

    enum
    {
        ISIM_NO_ERROR = (-1)
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
    IMSVector<IMS_SINT32> m_objPcscfDiscoveryMethods;
    // The list of P-CSCF address is provided when the P-CSCF discovery method
    // is PCSCF_DISCOVERY_METHOD_CONFIG.
    IMSVector<ServerAddress*> m_objPcscfAddresses;

    IIsim* m_piIsim;
    IMS_BOOL m_bFlagRequestPending;
    IMS_SINT32 m_nConfiguredIsimRecords;
    IMS_SINT32 m_nIsimRecords;
    // For tracking of ISIM error code
    IMS_SINT32 m_nIsimErrorCode;
    // ISIM Service Table
    IMS_BYTE m_byIst1;

    IMS_SINT32 m_nState;
    IMSList<ImsSubscriberInfo*> m_objSubscriberInfos;

    mutable IMSList<ISubscriberConfigListener*> m_objListeners;
    ISubscriberInfoListener* m_piSubsInfoListener;

    // Configurable class
    Configurable* m_pConfigurable;

    // Retry timer when the initialization is failed
    IMS_SINT32 m_nInitRetryCount;
    ITimer* m_piInitRetryTimer;
    // Retry timer when the start operation is failed
    IMS_SINT32 m_nInitRetryCountByStartRetry;
    IMS_SINT32 m_nStartRetryCount;
    ITimer* m_piStartRetryTimer;

    // Logging
    mutable AString m_strLog;
};

#endif
