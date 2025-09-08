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
#ifndef SIP_CONFIG_H_
#define SIP_CONFIG_H_

#include "AStringArray.h"
#include "ImsMap.h"

#include "CarrierConfig.h"
#include "Credential.h"
#include "ISipConfig.h"
#include "private/ConfigBase.h"
#include "private/SipConfigV.h"

class SipConfig : public ConfigBase, public ISipConfig
{
public:
    class TcpTimerValues
    {
    public:
        TcpTimerValues();
        TcpTimerValues(IN const TcpTimerValues& other);
        ~TcpTimerValues();

    public:
        TcpTimerValues& operator=(IN const TcpTimerValues& other);

    public:
        enum
        {
            PERMANENT = -2
        };

        // Unit is milli-seconds
        // It MUST be greater than 0
        IMS_SINT32 m_nTvConnectionWaiting;
        // 0 : Do not start timer (transaction-based tcp socket management)
        // -2 : Keep infinitely
        // greater than 0 : Start timer
        IMS_SINT32 m_nTvKeepAlive;
        // 0 : Wait infinitely
        // greater than 0 : Start timer
        IMS_SINT32 m_nTvWouldblockWaiting;
    };

public:
    explicit SipConfig(IN IMS_SINT32 nSlotId);
    virtual ~SipConfig();

    SipConfig(IN const SipConfig&) = delete;
    SipConfig& operator=(IN const SipConfig&) = delete;

public:
    // ISipConfig interface
    inline IMS_SINT32 GetPort() const override
    {
        return (m_nListenChannel == INVALID_CHANNEL) ? DEFAULT_CHANNEL : m_nListenChannel;
    }
    inline const ISipConfigV* GetSipConfigV() const override { return GetServiceConfig(); }
    inline IMS_UINT32 GetSipFeatureCaps() const override { return m_nSipFeatureCaps; }
    const AString& GetUaVersion() const override;
    inline IMS_BOOL IsGruuConfigured() const override { return HasFeature(SIP_FEATURE_CAPS_GRUU); }

    // ConfigBase class
    IMS_BOOL Init() override;
    void Refresh() override;

    inline const AString& GetDefaultScheme() const { return m_strScheme; }
    inline IMS_SINT32 GetDeviceId() const { return m_nDeviceId; }
    inline const AString& GetPredefinedDeviceId() const { return m_strDeviceId; }
    inline const AString& GetTagPrefix() const { return m_strTagPrefix; }
    inline IMS_SINT32 GetTcpCriterionLength() const { return m_nTcpCriterionLength; }
    inline IMS_SINT32 GetTransportType() const { return m_nTransportType; }
    IMS_SINT32 GetTimerValueT1() const;
    IMS_SINT32 GetTimerValueT2() const;
    inline IMS_SINT32 GetTimerValue100Trying() const { return m_nTimerValue100Trying; }
    inline const TcpTimerValues& GetTimerValueTcp() const { return m_objTcpTimerValues; }

    inline IMS_BOOL HasFeature(IN IMS_SINT32 nFeature) const
    {
        return ((m_nSipFeatureCaps & nFeature) == nFeature);
    }
    inline IMS_BOOL IsAuthenticationAlgorithmRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER);
    }
    inline IMS_BOOL IsCompactFormConfigured() const { return m_bCompactFormConfigured; }
    inline IMS_BOOL IsContactInAll1xxRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX);
    }
    inline IMS_BOOL IsDisplayNameDquotRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT);
    }
    inline IMS_BOOL IsExpiresHeaderInRegRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG);
    }
    inline IMS_BOOL IsIpSecConfigured() const { return HasFeature(SIP_FEATURE_CAPS_IPSEC); }
    inline IMS_BOOL IsKeepAliveConfigured() const { return HasFeature(SIP_FEATURE_CAPS_KEEP); }
    inline IMS_BOOL IsMultipleRegConfigured() const
    {
        return m_nSupportMultipleReg == CarrierConfig::Ims::MULTIPLE_REGISTRATION_FULL;
    }
    inline IMS_BOOL IsRegIdParameterConfigured() const
    {
        return m_nSupportMultipleReg == CarrierConfig::Ims::MULTIPLE_REGISTRATION_REG_ID_ONLY ||
                m_nSupportMultipleReg == CarrierConfig::Ims::MULTIPLE_REGISTRATION_FULL;
    }
    inline IMS_BOOL IsNoAcceptContactHeaderInBYE() const
    {
        return HasFeature(SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE);
    }
    inline IMS_BOOL IsPaniHeaderInInitialRegRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG);
    }
    inline IMS_BOOL IsPermissionSipConfigured() const { return IMS_TRUE; }
    inline IMS_BOOL IsPermissionSipsConfigured() const { return IMS_FALSE; }
    inline IMS_BOOL IsPPreferredIdInRegSubRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB);
    }
    inline IMS_BOOL IsRouteHeaderInRegRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG);
    }
    inline IMS_BOOL IsRportConfigured() const { return HasFeature(SIP_FEATURE_CAPS_RPORT); }
    inline IMS_BOOL IsTransportErrorReportOnTxnRequired() const
    {
        return HasFeature(SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN);
    }
    inline IMS_BOOL IsTrustDomainConfigured() const
    {
        return HasFeature(SIP_FEATURE_CAPS_TRUST_DOMAIN);
    }
    inline IMS_BOOL IsUdpFallbackConfigured() const
    {
        return HasFeature(SIP_FEATURE_CAPS_UDP_FALLBACK);
    }
    inline IMS_BOOL IsUserAgentConfigured() const
    {
        return HasFeature(SIP_FEATURE_CAPS_USER_AGENT);
    }
    inline IMS_BOOL IsUserAgentSetByContext() const
    {
        return HasFeature(SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT);
    }

    inline IMS_BOOL IsSdpNegotiationRequiredForNonRpr() const
    {
        return HasFeature(SIP_FEATURE_CAPS_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR);
    }
    inline IMS_BOOL IsRequestUriValidationRequiredInMidDialog() const
    {
        return HasFeature(SIP_FEATURE_CAPS_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG);
    }
    inline IMS_BOOL IsSessionTimerUpdateRequiredByReInvite() const
    {
        return HasFeature(SIP_FEATURE_CAPS_SESSION_TIMER_UPDATE_REQUIRED_BY_REINVITE);
    }
    inline IMS_BOOL IsSipInstanceParamRequiredInContactForNonRegisterRequest() const
    {
        return HasFeature(
                SIP_FEATURE_CAPS_SIP_INSTANCE_PARAM_REQUIRED_IN_CONTACT_FOR_NON_REGISTER_REQUEST);
    }
    inline IMS_BOOL IsSessionIdHeaderSupported() const
    {
        return HasFeature(SIP_FEATURE_CAPS_SUPPORT_SESSION_ID_HEADER);
    }
    inline IMS_BOOL IsLocalTimezoneParameterSupportedInPaniHeader() const
    {
        return HasFeature(SIP_FEATURE_CAPS_LOCAL_TIMEZONE_PARAM_IN_PANI_HEADER);
    }
    inline IMS_SINT32 GetHideMacInPaniHeaderPolicy() const { return m_nHideMacInPaniHeader; }
    inline IMS_SINT32 GetRegContactUserInfoPart() const { return m_nRegContactUserInfoPart; }
    inline IMS_SINT32 GetRegExpiration() const
    {
        return !IsRegExpirationConfigured() ? INVALID_EXPIRATION : m_nRegExpiration;
    }
    inline const AStringArray& GetRegAllowMethods() const { return m_objAllowMethods; }
    inline IMS_BOOL IsRegExpirationConfigured() const
    {
        return ((m_nRegExpiresMask & EXPIRES_REG) == EXPIRES_REG);
    }
    inline IMS_BOOL IsRegSubscriptionConfigured() const { return m_bRegSubscription; }
    inline IMS_BOOL IsRegSubExpirationConfigured() const
    {
        return ((m_nRegExpiresMask & EXPIRES_REG_SUB) == EXPIRES_REG_SUB);
    }
    inline IMS_SINT32 GetRegSubExpiration() const
    {
        return ((m_nRegExpiresMask & EXPIRES_REG_SUB) != EXPIRES_REG_SUB) ? INVALID_EXPIRATION
                                                                          : m_nRegSubExpiration;
    }

    inline const SipConfigV* GetServiceConfig() const { return m_pSipConfigV; }

protected:
    // ISipConfig class
    inline IConfigurable* GetConfigurable() const override { return m_pConfigurable; }

    // ConfigBase class
    IMS_BOOL ReadFrom() override;
    IMS_BOOL Update(IN IMS_SINT32 nCpi, IN const AString& strValue = AString::ConstNull()) override;
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

private:
    void Clear();
    IMS_BOOL GetTimerValueForUpdate(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue,
            IN const AString& strUpdateTimerValue, OUT IMS_SINT32& nTimerValue);
    void UpdateTcpTimerValues();
    static IMS_SINT32 ReadSipFeatureCaps(IN const ICarrierConfig* piCc);

public:
    enum
    {
        INVALID_EXPIRATION = (-1),
        DEFAULT_EXPIRATION = 600000
    };

    enum
    {
        TRANSPORT_TYPE_UDP = CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP,
        TRANSPORT_TYPE_TCP = CarrierConfig::Ims::PREFERRED_TRANSPORT_TCP,
        TRANSPORT_TYPE_DYNAMIC_UDP_TCP = CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP,
        TRANSPORT_TYPE_TLS = CarrierConfig::Ims::PREFERRED_TRANSPORT_TLS
    };

private:
    static const IMS_CHAR TAG_PREFIX[];

    enum
    {
        INVALID_CHANNEL = (-1),
        DEFAULT_CHANNEL = 5060
    };

    // Flags for SIP signalling
    IMS_BOOL m_bCompactFormConfigured;
    IMS_SINT32 m_nSipFeatureCaps;
    // For GRUU, type of device id (+sip.instance, URN)
    // (-1) (GRUU is not supported)
    // 0 (GSMA_IMEI)
    // 1 (UUID_IMEI_MD5)
    // 2 (UUID_IMEI_SHA1)
    // 3 (UUID_IMEI_NAMED_V3)
    // 4 (UUID_IMEI_NAMED_V5)
    // 5 (UUID_IMEI_V4)
    // 6 (PREDEFINED)
    IMS_SINT32 m_nDeviceId;
    // Pre-defined UUID
    AString m_strDeviceId;

    AString m_strScheme;
    AString m_strTagPrefix;
    // SIP listen channel - default port
    IMS_SINT32 m_nListenChannel;
    IMS_SINT32 m_nTransportType;

    // Timer to send 100 Trying response (milli-seconds)
    IMS_SINT32 m_nTimerValue100Trying;

    // TCP criterion length on run-time
    //     A default threshold value when MTU size is unknown; The default MTU size will be 1500.
    //     200 bytes is a buffer for collecting the Record-Route.
    enum
    {
        TCP_CRITERION_LEN = 1300
    };
    IMS_SINT32 m_nTcpCriterionLength;
    TcpTimerValues m_objTcpTimerValues;

    IMS_SINT32 m_nHideMacInPaniHeader;
    IMS_SINT32 m_nSupportMultipleReg;

    // Registration parameters
    enum
    {
        EXPIRES_NONE = 0x00,
        EXPIRES_REG = 0x01,
        EXPIRES_REG_SUB = 0x02
    };

    IMS_SINT32 m_nRegContactUserInfoPart;
    IMS_SINT32 m_nRegExpiresMask;
    IMS_SINT32 m_nRegExpiration;
    AStringArray m_objAllowMethods;
    IMS_BOOL m_bRegSubscription;
    IMS_SINT32 m_nRegSubExpiration;

    // SERVICE SPECIFIC SIP CONFIGURATIONS
    SipConfigV* m_pSipConfigV;

    // Configurable class
    Configurable* m_pConfigurable;
};

#endif
