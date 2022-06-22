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
#ifndef SIP_CONFIG_V_H_
#define SIP_CONFIG_V_H_

#include "AStringArray.h"

#include "ISipConfigV.h"
#include "private/ConfigBase.h"

class SipConfigV : public ConfigBase, public ISipConfigV
{
private:
    // Session parameters
    struct Session
    {
        IMS_BOOL bSessionTimerSupported;
        IMS_SINT32 nRefresher;
        IMS_SINT32 nRefreshMethod;
        IMS_SINT32 nMinSe;
        IMS_SINT32 nSessionExpires;
        IMS_SINT32 nHeaders;
        IMS_BOOL bNoRefreshByReInvite;
        // This value will not be managed in the non-volatile memory
        IMS_BOOL b100TryingNotification;
        IMS_BOOL bSdpVersionCheckSupported;
        IMS_BOOL bSdpNonRprAllowed;

        inline Session() :
                bSessionTimerSupported(IMS_TRUE),
                nRefresher(SESSION_REFRESHER_LOCAL),
                nRefreshMethod(SESSION_REFRESH_UPDATE),
                nMinSe(90),
                nSessionExpires(3600),
                nHeaders(SESSION_HEADER_ALL),
                bNoRefreshByReInvite(IMS_FALSE),
                b100TryingNotification(IMS_FALSE),
                bSdpVersionCheckSupported(IMS_TRUE),
                bSdpNonRprAllowed(IMS_FALSE)
        {
        }
    };

public:
    explicit SipConfigV(IN IMS_SINT32 nSlotId);
    virtual ~SipConfigV();

    SipConfigV(IN const SipConfigV&) = delete;
    SipConfigV& operator=(IN const SipConfigV&) = delete;

public:
    // ConfigBase class
    IMS_BOOL Init() override;
    inline IMS_UINT32 GetFeatureTagOptions() const override { return m_nFeatureTagOptions; }
    inline IMS_SINT32 GetTargetNumberFormat() const override { return m_nTargetNumberFormat; }
    inline IMS_SINT32 GetTargetScheme() const override { return m_nTargetScheme; }
    inline IMS_SINT32 GetTimerValue(IN IMS_SINT32 nType) const override;

    inline IMS_SINT32 GetPreferredId() const { return m_nPreferredId; }
    inline const AStringArray& GetAllowMethods() const { return m_objAllowMethods; }
    inline const AString& GetServiceVersion() const { return m_strServiceVersion; }

    inline const AString& GetPredefinedPaniForEutran() const
    {
        return m_strPredefinedPaniForEutran;
    }
    inline const AString& GetPredefinedPaniForWlan() const { return m_strPredefinedPaniForWlan; }
    inline const AString& GetPredefinedPaniForUtran() const { return m_strPredefinedPaniForUtran; }

    inline IMS_SINT32 GetTimerValueT1() const { return m_nTimerValueT1; }
    inline IMS_SINT32 GetTimerValueT2() const { return m_nTimerValueT2; }
    inline IMS_SINT32 GetTimerValueT4() const { return m_nTimerValueT4; }
    inline IMS_SINT32 GetTimerValueA() const { return m_nTimerValueA; }
    inline IMS_SINT32 GetTimerValueB() const { return m_nTimerValueB; }
    inline IMS_SINT32 GetTimerValueC() const { return m_nTimerValueC; }
    inline IMS_SINT32 GetTimerValueD() const { return m_nTimerValueD; }
    inline IMS_SINT32 GetTimerValueE() const { return m_nTimerValueE; }
    inline IMS_SINT32 GetTimerValueF() const { return m_nTimerValueF; }
    inline IMS_SINT32 GetTimerValueG() const { return m_nTimerValueG; }
    inline IMS_SINT32 GetTimerValueH() const { return m_nTimerValueH; }
    inline IMS_SINT32 GetTimerValueI() const { return m_nTimerValueI; }
    inline IMS_SINT32 GetTimerValueJ() const { return m_nTimerValueJ; }
    inline IMS_SINT32 GetTimerValueK() const { return m_nTimerValueK; }
    inline IMS_BOOL IsTimerValueConfiguredOnRuntime() const { return m_bIsTimerValueConfigured; }

    // "session"
    inline IMS_SINT32 GetSessionExpires() const { return m_objSession.nSessionExpires; }
    inline IMS_SINT32 GetSessionHeaders() const { return m_objSession.nHeaders; }
    inline IMS_SINT32 GetSessionRefresher() const { return m_objSession.nRefresher; }
    inline IMS_SINT32 GetSessionMethod() const { return m_objSession.nRefreshMethod; }
    inline IMS_SINT32 GetSessionMinSe() const { return m_objSession.nMinSe; }
    inline IMS_BOOL Is100TryingNotificationRequired() const
    {
        return m_objSession.b100TryingNotification;
    }
    inline IMS_BOOL IsSessionNoRefreshByReInvite() const
    {
        return m_objSession.bNoRefreshByReInvite;
    }
    inline IMS_BOOL IsSessionTimerSupported() const { return m_objSession.bSessionTimerSupported; }
    inline IMS_BOOL IsSessionSdpNonRprAllowed() const { return m_objSession.bSdpNonRprAllowed; }
    inline IMS_BOOL IsSessionSdpVersionCheckSupported() const
    {
        return m_objSession.bSdpVersionCheckSupported;
    }

    // "capabilities"
    inline IMS_BOOL IsCapabilitiesRespByApp() const { return m_bRespByAppForCapabilities; }
    // "pagemessage"
    inline IMS_BOOL IsPageMessageRespByApp() const { return m_bRespByAppForPageMessage; }
    // "reference"
    inline IMS_BOOL IsReferenceRespByApp() const { return m_bRespByAppForReference; }

protected:
    // ISipConfigV class
    inline IConfigurable* GetConfigurable() const override { return m_pConfigurable; }

    // ConfigBase class
    IMS_BOOL ReadFrom() override;
    IMS_BOOL Update(IN IMS_SINT32 nCpi, IN const AString& strValue = AString::ConstNull()) override;
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

private:
    IMS_BOOL GetTimerValueForUpdate(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue,
            IN const AString& strUpdateTimerValue, OUT IMS_SINT32& nTimerValue);
    void UpdateAllConfigs();

    static IMS_SINT32 GetTimerValue(
            IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue);

public:
    enum
    {
        DEFAULT_TIMER_T1 = 2000,
        DEFAULT_TIMER_T2 = 16000,
        DEFAULT_TIMER_T4 = 17000,
        DEFAULT_TIMER_C = 182000,
        DEFAULT_TIMER_D = 130000
    };

    enum
    {
        PREFERRED_ID_ALL = 0,
        PREFERRED_ID_SIP,
        PREFERRED_ID_SIP_TEL,
        PREFERRED_ID_TEL,
        PREFERRED_ID_TEL_SIP,
        /// default value - topmost id from P-Associated-URI
        PREFERRED_ID_DEFAULT
    };

    enum
    {
        SESSION_REFRESHER_NONE = 0,
        SESSION_REFRESHER_LOCAL,
        SESSION_REFRESHER_REMOTE
    };

    enum
    {
        SESSION_REFRESH_INVITE = 0,
        SESSION_REFRESH_UPDATE,
        /// Depends on the context
        SESSION_REFRESH_ANY
    };

    enum
    {
        SESSION_HEADER_NONE = 0x00,
        /// For header forming
        SESSION_HEADER_SESSION_EXPIRES = 0x01,
        SESSION_HEADER_MIN_SE = 0x02,
        SESSION_HEADER_TIMER_OPTION = 0x04,
        /// For header checking in MT
        /// (to check if the peer supports the session timer extension or not)
        SESSION_HEADER_CHECK_SESSION_EXPIRES = 0x10,
        /// Even though the session timer is not supported (no Session-Expires),
        /// if it sets, UE MUST start the session timer using the configured session interval.
        SESSION_HEADER_LOCAL_TIMER_REQUIRED = 0x20,
        /// It indicates that "refresher" parameter can be updated
        /// when early UPDATE doesn't contain "refresher" parameter.
        SESSION_HEADER_NO_REFRESHER_CONTROLLED_ON_EARLY_UPDATE = 0x40,
        SESSION_HEADER_ALL = 0xFF
    };

private:
    // Target number format
    IMS_SINT32 m_nTargetNumberFormat;
    // Target (Request-URI) scheme
    IMS_SINT32 m_nTargetScheme;
    // preferred user identity
    IMS_SINT32 m_nPreferredId;
    // allow-methods
    AStringArray m_objAllowMethods;
    // service version
    AString m_strServiceVersion;

    AString m_strPredefinedPaniForEutran;
    AString m_strPredefinedPaniForWlan;
    AString m_strPredefinedPaniForUtran;

    // Optional feature tags for caller capability & preference
    // ALL : 0xFF, EVENT = 0x01, MEDIA_BASIC = 0x02, MEDIA_FRAMED = 0x04, MEDIA_STREAM = 0x08
    IMS_UINT32 m_nFeatureTagOptions;

    // Timer values for SIP transaction
    IMS_BOOL m_bIsTimerValueConfigured;
    IMS_SINT32 m_nTimerValueT1;
    IMS_SINT32 m_nTimerValueT2;
    IMS_SINT32 m_nTimerValueT4;
    IMS_SINT32 m_nTimerValueA;
    IMS_SINT32 m_nTimerValueB;
    IMS_SINT32 m_nTimerValueC;
    IMS_SINT32 m_nTimerValueD;
    IMS_SINT32 m_nTimerValueE;
    IMS_SINT32 m_nTimerValueF;
    IMS_SINT32 m_nTimerValueG;
    IMS_SINT32 m_nTimerValueH;
    IMS_SINT32 m_nTimerValueI;
    IMS_SINT32 m_nTimerValueJ;
    IMS_SINT32 m_nTimerValueK;

    Session m_objSession;

    // Capabilities
    IMS_BOOL m_bRespByAppForCapabilities;
    // PageMessage
    IMS_BOOL m_bRespByAppForPageMessage;
    // Reference
    IMS_BOOL m_bRespByAppForReference;

    // Configurable class
    Configurable* m_pConfigurable;
};

#endif
