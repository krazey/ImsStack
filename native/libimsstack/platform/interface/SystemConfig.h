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
#ifndef SYSTEM_CONFIG_H_
#define SYSTEM_CONFIG_H_

#include "AString.h"

#define IMS_SC_SIZE_8 8
#define IMS_SC_SIZE_16 16
#define IMS_SC_SIZE_32 32

struct __SystemConfig
{
    // Slot id
    IMS_SINT32 nSlotId;

    // Platform configuration to identify the operator
    IMS_CHAR acOperator[IMS_SC_SIZE_16 + 1];
    IMS_CHAR acCountry[IMS_SC_SIZE_8 + 1];

    // Enabler configuration
    IMS_CHAR acEnablerType[IMS_SC_SIZE_16 + 1];
    IMS_SINT32 nExtraInfo;

    // Runtime features
    IMS_SINT32 nFeatures;
    IMS_SINT32 nServiceFeatures;
};

class SystemConfig
{
public:
    /// Indicates that which event is occurred
    enum
    {
        EVENT_DEVICE_CONFIG = 0,
        EVENT_ON_BOOT = 1,
        /// When system configuration & subscriber are changed
        ///    - Operator/country changed
        EVENT_SUBSCRIPTION_CHANGED = 2,
        /// When all the configuration need to be reset including system configuration
        EVENT_ALL_CONFIGURATION_CHANGED = 3,
        /// When service features are changed
        EVENT_FEATURE_CHANGED = 4,
        /// Special case: DDS device (non-multi-IMS) : When DDS is changed
        EVENT_DDS_CHANGED = 5,
        /// Special case: IMS feature permissions changed (i.e. when Google Fi SIM inserted)
        /// (System configuration is not required)
        EVENT_FEATURE_PERMISSIONS_CHANGED = 6
    };

    /// Extra information
    enum
    {
        EXTRA_INFO_NONE = 0,
        EXTRA_INFO_SIM_MOBILITY = 0x00000001,
        EXTRA_INFO_KR_ENABLER = 0x00000002,

        /// Special case: UE capability - VoNR enabled (not EPS-FB)
        EXTRA_INFO_NR_UE_CAPABILITY_VONR = 0x01000000,
        /// Special case: NSA mode when VoNR is enabled
        EXTRA_INFO_NR_NSA_MODE = 0x10000000,
        /// Special case: DDS device (non-multi-IMS)
        EXTRA_INFO_DDS = 0x20000000,
        /// Special case: To identify no SIM or SIM-REMOVED status
        EXTRA_INFO_NO_UICC = 0x40000000
    };

    /// Features for functional level
    enum
    {
        FEATURE_IPSEC = 0x00000001,
        FEATURE_TLS = 0x00000002,
        FEATURE_AUTH_SIP_DIGEST = 0x00000004,
        FEATURE_SDP_PRECONDITION = 0x00000008,
        FEATURE_GRUU = 0x00000010,
        FEATURE_MULTIPLE_REGISTRATION = 0x00000020,
        FEATURE_REQUEST_URI_VALIDATION_IN_MID_DIALOG = 0x00000040,
        FEATURE_NO_SESSION_REFRESH_BY_REINVITE = 0x00000080,
        FEATURE_INVITE_TXN_HANDLING_CORRECTION = 0x00000100,
        FEATURE_GEOLOCATION = 0x00000200,
        FEATURE_VOLTE_IN_ROAMING = 0x00000400,
        FEATURE_VT_IN_ROAMING = 0x00000800
    };

    /// Features for service level
    enum
    {
        FEATURE_S_VOLTE = 0x00000001,
        FEATURE_S_VOWIFI = 0x00000002,
        FEATURE_S_VT = 0x00000004,
        FEATURE_S_SMS = 0x00000008,
        FEATURE_S_VOLTE_EMERGENCY = 0x00000010,

        FEATURE_S_UCE = 0x00000100
    };

public:
    SystemConfig();
    SystemConfig(IN const __SystemConfig* pConfig);
    SystemConfig(IN const SystemConfig& other);
    ~SystemConfig();

public:
    SystemConfig& operator=(IN const SystemConfig& other);

public:
    IMS_BOOL Equals(IN const SystemConfig& other) const;

    inline IMS_SINT32 GetSlotId() const;

    inline const AString& GetOperator() const;
    inline const AString& GetCountry() const;

    inline const AString& GetEnablerType() const;
    inline IMS_SINT32 GetExtraInfo() const;
    inline IMS_BOOL IsSimMobilityEnabled() const;
    inline IMS_BOOL IsKREnablerEnabled() const;
    inline IMS_BOOL IsDds() const;
    inline IMS_BOOL IsNoUicc() const;
    inline IMS_BOOL IsNrNsaModeEnabled() const;
    inline IMS_BOOL IsUeCapabilityVoNrEnabled() const;

    inline IMS_SINT32 GetFeatures() const;
    inline IMS_SINT32 GetServiceFeatures() const;

    AString ToString() const;
    // The caller MUTS free the memory of the pointer variable
    // which returns from this method.
    __SystemConfig* ToSystemConfig() const;

    static const AString& GetPackageName();
    static IMS_SINT32 GetMaxSimSlot();
    static IMS_BOOL IsMultiImsEnabled();
    // DSSV-DV (Dual SIM Single VoLTE - Dual VoLTE for emergency
    static IMS_BOOL IsMultiImsEnabledOnDssv();
    static IMS_BOOL IsMultiLteEnabled();
    static IMS_BOOL IsMultiSimEnabled();

    static IMS_BOOL IsOperatorChanged(IN const SystemConfig* pOldConfig,
            IN const SystemConfig* pNewConfig);
    static IMS_BOOL IsServiceFeatureChanged(IN const SystemConfig* pOldConfig,
            IN const SystemConfig* pNewConfig);
    static IMS_BOOL IsDdsChanged(IN const SystemConfig* pOldConfig,
            IN const SystemConfig* pNewConfig);
    static IMS_BOOL IsSimMobilityChanged(IN const SystemConfig* pOldConfig,
            IN const SystemConfig* pNewConfig);

private:
    IMS_BOOL IsExtraInfoSet(IN IMS_SINT32 nExtraInfo) const;

    static void CacheGlobalConfigs();
    static void UpdateGlobalConfigsOnFeatureChanged();

private:
    /// Global configurations
    enum
    {
        CONFIG_MULTI_IMS = 0x0001,
        CONFIG_MULTI_LTE = 0x0002
    };

    friend class SystemConfigManager;

    static IMS_SINT32 s_nGlobalConfigs;

    // ImsStack's package name
    static AString s_strPackageName;

    // Slot id
    IMS_SINT32 m_nSlotId;

    // Platform configuration to identify the operator
    AString m_strOperator;
    AString m_strCountry;

    // Enabler configuration
    AString m_strEnablerType;
    IMS_SINT32 m_nExtraInfo;

    // Runtime features
    IMS_SINT32 m_nFeatures;
    IMS_SINT32 m_nServiceFeatures;
};

inline IMS_SINT32 SystemConfig::GetSlotId() const
{ return m_nSlotId; }

inline const AString& SystemConfig::GetOperator() const
{ return m_strOperator; }
inline const AString& SystemConfig::GetCountry() const
{ return m_strCountry; }

inline const AString& SystemConfig::GetEnablerType() const
{ return m_strEnablerType; }
inline IMS_SINT32 SystemConfig::GetExtraInfo() const
{ return m_nExtraInfo; }
inline IMS_BOOL SystemConfig::IsSimMobilityEnabled() const
{ return IsExtraInfoSet(EXTRA_INFO_SIM_MOBILITY); }
inline IMS_BOOL SystemConfig::IsKREnablerEnabled() const
{ return IsExtraInfoSet(EXTRA_INFO_KR_ENABLER); }
inline IMS_BOOL SystemConfig::IsDds() const
{ return IsExtraInfoSet(EXTRA_INFO_DDS); }
inline IMS_BOOL SystemConfig::IsNoUicc() const
{ return IsExtraInfoSet(EXTRA_INFO_NO_UICC); }
inline IMS_BOOL SystemConfig::IsNrNsaModeEnabled() const
{ return IsExtraInfoSet(EXTRA_INFO_NR_NSA_MODE); }
inline IMS_BOOL SystemConfig::IsUeCapabilityVoNrEnabled() const
{ return IsExtraInfoSet(EXTRA_INFO_NR_UE_CAPABILITY_VONR); }

inline IMS_SINT32 SystemConfig::GetFeatures() const
{ return m_nFeatures; }
inline IMS_SINT32 SystemConfig::GetServiceFeatures() const
{ return m_nServiceFeatures; }

#endif
