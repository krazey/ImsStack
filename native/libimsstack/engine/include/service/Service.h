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
#ifndef SERVICE_H_
#define SERVICE_H_

#include "IpAddress.h"

#include "IConfigUpdateListener.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"
#include "private/SipConfigV.h"

#include "Connection.h"
#include "FeatureCaps.h"
#include "IRegBindingListener.h"
#include "Sip.h"
#include "SipAddress.h"
#include "SipMethod.h"
#include "SipProfile.h"
#include "ServiceFilterCriteria.h"

class CallerCapability;
class Capabilities;
class IRegBinding;
class IRegInfo;
class IServiceCloseListener;
class ISipClientConnection;
class ISipDialog;
class ISipMessage;
class ISipServerConnection;
class Method;
class MethodManager;
class PageMessage;
class PreferenceHeader;
class Reference;
class Session;

/**
 * @brief Service is the base class for IMS services.
 */
class Service : public Connection, public IConfigUpdateListener, public IRegBindingListener
{
public:
    Service(IN const AString& strScheme, IN const AString& strAppId, IN const AString& strServiceId,
            IN const SipAddress* pImpu = IMS_NULL);
    virtual ~Service();

    Service(IN const Service&) = delete;
    Service& operator=(IN const Service&) = delete;

public:
    inline virtual const AString& GetAppId() const { return m_strAppId; }
    inline virtual const AString& GetScheme() const { return m_strScheme; }

    virtual IMS_BOOL CreateConfig(IN const AppConfig& objAppConfig);
    inline virtual void HandleSessionInvitationReceived(IN Session* /*pSession*/)
    {
        // The subclass MUST implement this method to handle an incoming INVITE request
    }
    virtual void HandlePageMessageReceived(IN PageMessage* /*pPageMessage*/)
    {
        // The subclass MUST implement this method to handle an incoming MESSAGE request
    }
    virtual void HandleReferenceReceived(IN Reference* /*pReference*/)
    {
        // The subclass MUST implement this method to handle an incoming REFER request
    }
    virtual void HandleCapabilityQueryReceived(IN Capabilities* /*pCapabilities*/)
    {
        // The subclass MUST implement this method to handle an incoming OPTIONS request
    }
    virtual ISipClientConnection* CreateConnection(IN const SipAddress* pFrom,
            IN const SipAddress* pTo, IN const SipMethod& objMethod,
            IN IMS_BOOL bPrivacy = IMS_FALSE);
    virtual ISipClientConnection* CreateConnection(IN ISipDialog* piDialog,
            IN const SipMethod& objMethod, IN IMS_BOOL bPrivacy = IMS_FALSE);
    virtual ISipClientConnection* CreateCancelConnection(IN ISipClientConnection* piScc);
    virtual IMS_BOOL CreateResponse(IN_OUT ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
            IN const AString& strPhrase = AString::ConstNull(), IN IMS_BOOL bPrivacy = IMS_FALSE);
    virtual IMS_BOOL InitAck(IN ISipClientConnection* piScc);

    IMS_BOOL Equals(IN const Service* pService) const;
    IMS_UINT32 EvaluateFilterCriteria(IN const ISipMessage* piSipMsg) const;
    inline const AString& GetServiceId() const { return m_strServiceId; }
    inline AppConfig* GetAppConfig() const { return m_pAppConfig; }
    inline const CoreServiceConfig* GetServiceConfig() const
    {
        return m_pAppConfig->GetCoreServiceConfigEx(m_strServiceId);
    }
    const IpAddress& GetIpAddress() const;
    inline const ImsList<PreferenceHeader*>& GetAcceptContactHeaders() const
    {
        return m_objAcceptContacts;
    }
    const AString& GetAssociatedUri(IN IMS_SINT32 nScheme) const;
    const AStringArray& GetAssociatedUris() const;
    const SipAddress& GetAuthorizedUserId() const;
    inline const CallerCapability* GetCallerCapability() const { return m_pCallerCapability; }
    const SipAddress& GetContactAddress() const;
    const SipAddress* GetContactAddressForOutgoingMessage() const;
    ISipHeader* GetContactHeader(IN IMS_BOOL bPrivacy = IMS_FALSE, IN IMS_BOOL bRequest = IMS_TRUE,
            IN IMS_SINT32 nSipMethod = SipMethod::INVALID) const;
    inline const SipAddress& GetDefaultUserId() const { return m_objImpu; }
    // CONTACT_FEATURE_CAPS
    inline IFeatureCaps* GetFeatureCaps() const { return m_pFeatureCaps; }
    inline ServiceFilterCriteria* GetFilterCriteria() const { return m_pFilterCriteria; }
    const SipParameter* GetInstanceParameter() const;
    const AStringArray& GetPathHeaders() const;
    const IRegInfo* GetRegInfo() const;
    const AStringArray& GetServiceRoutes() const;
    const ISipConfigV* GetISipConfigV() const;
    inline const SipConfigV* GetSipConfigV() const
    {
        return DYNAMIC_CAST(const SipConfigV*, GetISipConfigV());
    }
    SipProfile* GetSipProfile() const;
    const AString& GetSubscriberId() const;

    const SipAddress* GetPublicGruu() const;
    const SipAddress* GetTemporaryGruu() const;
    const ImsList<SipAddress*>& GetTemporaryGruus() const;

    IMS_BOOL IsBehindNat() const;
    IMS_BOOL IsEventPackageSupported(IN const AString& strEvent) const;
    IMS_BOOL IsForEmergency() const;
    inline IMS_BOOL IsImsConnected() const { return m_bImsConnected; }
    IMS_BOOL IsWithinTrustDomain() const;

    inline IMS_BOOL AddFeatureTags(
            IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE)
    {
        return UpdateFeatureTags(objFeatureTags, bRegRequired, FEATURE_TAG_OP_ADD);
    }
    inline IMS_BOOL RemoveFeatureTags(
            IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE)
    {
        return UpdateFeatureTags(objFeatureTags, bRegRequired, FEATURE_TAG_OP_REMOVE);
    }
    IMS_BOOL UpdateFeatureTags(IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired,
            IN IMS_SINT32 nOperation);

    void NotifyError(IN IMS_SINT32 nErrorCode);
    inline IMS_BOOL NotifyRequest(IN ISipServerConnection* piSsc)
    {
        return ServerConnection_NotifyRequest(piSsc);
    }
    IMS_BOOL SendResponse(IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
            IN const AString& strPhrase = AString::ConstNull());
    inline void SetServiceCloseListener(IN IServiceCloseListener* piListener)
    {
        m_piServiceCloseListener = piListener;
    }
    inline void SetSipProfile(IN SipProfile* pProfile) { m_pSipProfile = pProfile; }
    void RegisterMethod(IN Method* pMethod);
    void DeregisterMethod(IN Method* pMethod);

    IMS_BOOL ValidateMethod(IN const SipMethod& objMethod) const;
    IMS_BOOL ValidateRequestUri(IN const SipAddress& objRequestUri,
            IN ISipDialog* piDialog = IMS_NULL, IN IMS_BOOL bIsMidDialogRequest = IMS_FALSE);
    IMS_BOOL ValidateRequestUriForIpAndPort(IN const SipAddress& objRequestUri,
            IN ISipDialog* piDialog = IMS_NULL, IN IMS_BOOL bIsMidDialogRequest = IMS_FALSE);

    static IMS_BOOL ValidateFromAndTo(
            IN const AString& strFrom, IN const AString& strTo, IN IMS_BOOL bToLenient);
    static IMS_BOOL ValidateReferTo(IN const AString& strUri, IN const AString& strMethod);

protected:
    // Connection class - overrides
    void Close() override;
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // IConfigUpdateListener class
    void ConfigUpdate_NotifyUpdate(IN IMS_SINT32 nCpi,
            IN const AString& strConfName = AString::ConstNull(),
            IN const AString& strExtraParam = AString::ConstNull()) override;

    // IRegBindingListener class
    void RegBinding_OnActive() override;
    void RegBinding_OnDestroy() override;
    void RegBinding_OnInit(IN const SipAddress* pAor) override;
    void RegBinding_OnQueryCapability(OUT CallerCapability*& pCapability) override;
    void RegBinding_OnQueryRegistrationHeaders(OUT AStringArray& objHeaders) override;
    void RegBinding_OnTerminated() override;

    inline virtual void Abort() {}
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode) = 0;
    inline virtual IMS_BOOL ServerConnection_NotifyRequest(IN ISipServerConnection* /*piSsc*/)
    {
        // The subclass MUST implement this method to handle new incoming SIP requests
        return IMS_FALSE;
    }
    inline void SetImsConnected(IN IMS_BOOL bImsConnected) { m_bImsConnected = bImsConnected; }

    void FormContactHeader(IN const SipMethod& objMethod, IN IMS_BOOL bPrivacy,
            IN IMS_BOOL bRequest, OUT AString& strContact, OUT IMS_BOOL& bIsContactGruu) const;
    inline IRegBinding* GetRegBinding() const { return m_piRegBinding; }
    inline IMS_BOOL IsUserIdProvisioned() const { return m_bProvisionedUserId; }
    void SetGruuOptionTagInMidDialog(IN ISipDialog* piDialog, IN_OUT ISipMessage*& piSipMsg);

private:
    void CreateDefaultPublicUserId();
    inline IMS_UINT32 GetServiceCode() const
    {
        AString strServiceCode = m_strAppId + m_strServiceId;
        return strServiceCode.GetHashCode();
    }
    inline IMS_BOOL IsRegBindingOnActive() const
    {
        return (IsImsConnected() && (m_piRegBinding != IMS_NULL));
    }
    // RFC 3325: INVITE/BYE/OPTIONS/SUBSCRIBE/NOTIFY/REFER
    // RFC 5876: all requests except for ACK/CANCEL
    // 3GPP Profile: INVITE/BYE/MESSAGE/PUBLISH/REFER/SUBSCRIBE/OPTIONS
    inline IMS_BOOL IsPpiHeaderRequired(IN const SipMethod& objMethod) const
    {
        return objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::BYE) ||
                objMethod.Equals(SipMethod::MESSAGE) || objMethod.Equals(SipMethod::PUBLISH) ||
                objMethod.Equals(SipMethod::REFER) || objMethod.Equals(SipMethod::SUBSCRIBE) ||
                objMethod.Equals(SipMethod::OPTIONS);
    }
    IMS_BOOL SetPPreferredIdentityHeader(
            IN IMS_SINT32 nPreferredId, IN_OUT ISipMessage*& piSipMsg) const;
    void SetRegBinding(IN IRegBinding* piRegBinding);
    void UpdateAuthorizedUserIds();
    void UpdateCallerCapabilityNPreference();
    void UpdateRegBindings();

protected:
    enum
    {
        AMSG_SERVICE_MAX = (AMSG_USER + 1)
    };

    enum
    {
        FEATURE_TAG_OP_ADD = 1,
        FEATURE_TAG_OP_REMOVE = 2
    };

private:
    friend class ServiceResolver;

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    class CachedRegBinding
    {
    public:
        inline CachedRegBinding() :
                m_nPortUc(Sip::PORT_UNSPECIFIED),
                m_nPortUs(Sip::PORT_UNSPECIFIED),
                m_nPortFlowControl(Sip::PORT_UNSPECIFIED),
                m_nTransportExt(Sip::TRANSPORT_EXT_ANY),
                m_objIp(IpAddress::NONE),
                m_objContactAddress(SipAddress::ConstNull()),
                m_pContactAddressForOutgoingMessage(IMS_NULL),
                m_objSecurityClients(AStringArray::ConstNull()),
                m_objSecurityVerifys(AStringArray::ConstNull()),
                m_pPubGruu(IMS_NULL),
                m_pTempGruu(IMS_NULL),
                m_objAssociatedUris(AStringArray::ConstNull()),
                m_bEmergencyRegistration(IMS_FALSE)
        {
        }
        inline ~CachedRegBinding()
        {
            if (m_pContactAddressForOutgoingMessage != IMS_NULL)
            {
                delete m_pContactAddressForOutgoingMessage;
            }

            if (m_pPubGruu != IMS_NULL)
            {
                delete m_pPubGruu;
            }

            if (m_pTempGruu != IMS_NULL)
            {
                delete m_pTempGruu;
            }
        }

        CachedRegBinding(IN const CachedRegBinding&) = delete;
        const CachedRegBinding& operator=(IN const CachedRegBinding&) = delete;

    public:
        inline IMS_SINT32 GetPortUc() const { return m_nPortUc; }
        inline IMS_SINT32 GetPortUs() const { return m_nPortUs; }
        // RFC5626_FLOW_CONTROL
        inline IMS_SINT32 GetPortFlowControl() const { return m_nPortFlowControl; }
        // MULTI_REG_TRANSPORT
        inline IMS_SINT32 GetTransportExt() const { return m_nTransportExt; }
        inline const IpAddress& GetIpAddress() const { return m_objIp; }
        inline const SipAddress& GetContactAddress() const { return m_objContactAddress; }
        inline const SipAddress* GetContactAddressForOutgoingMessage() const
        {
            return m_pContactAddressForOutgoingMessage;
        }

        inline const AStringArray& GetSecurityClients() const { return m_objSecurityClients; }
        inline const AStringArray& GetSecurityVerifys() const { return m_objSecurityVerifys; }

        inline const SipAddress* GetPublicGruu() const { return m_pPubGruu; }
        inline const SipAddress* GetTemporaryGruu() const { return m_pTempGruu; }

        inline const AStringArray& GetAssociatedUris() const { return m_objAssociatedUris; }
        inline SipProfile* GetSipProfile() const { return m_pSipProfile.Get(); }
        inline IMS_BOOL IsEmergencyRegistration() const { return m_bEmergencyRegistration; }

        inline void SetPortUc(IN IMS_SINT32 nPortUc) { m_nPortUc = nPortUc; }
        inline void SetPortUs(IN IMS_SINT32 nPortUs) { m_nPortUs = nPortUs; }
        // RFC5626_FLOW_CONTROL
        inline void SetPortFlowControl(IN IMS_SINT32 nPortFlowControl)
        {
            m_nPortFlowControl = nPortFlowControl;
        }
        // MULTI_REG_TRANSPORT
        inline void SetTransportExt(IN IMS_SINT32 nTransportExt)
        {
            m_nTransportExt = nTransportExt;
        }
        inline void SetIpAddress(IN const IpAddress& objIp) { m_objIp = objIp; }
        inline void SetContactAddress(IN const SipAddress& objContactAddress)
        {
            m_objContactAddress = objContactAddress;
        }
        inline void SetContactAddressForOutgoingMessage(IN const SipAddress* pContactAddress)
        {
            if (m_pContactAddressForOutgoingMessage != IMS_NULL)
            {
                delete m_pContactAddressForOutgoingMessage;
                m_pContactAddressForOutgoingMessage = IMS_NULL;
            }

            if (pContactAddress != IMS_NULL)
            {
                m_pContactAddressForOutgoingMessage = new SipAddress(*pContactAddress);
            }
        }
        inline void SetSecurityClients(IN const AStringArray& objSecurityClients)
        {
            m_objSecurityClients = objSecurityClients;
        }
        inline void SetSecurityVerifys(IN const AStringArray& objSecurityVerifys)
        {
            m_objSecurityVerifys = objSecurityVerifys;
        }

        inline void SetPublicGruu(IN const SipAddress* pPubGruu)
        {
            if (m_pPubGruu != IMS_NULL)
            {
                delete m_pPubGruu;
                m_pPubGruu = IMS_NULL;
            }

            if (pPubGruu != IMS_NULL)
            {
                m_pPubGruu = new SipAddress(*pPubGruu);
            }
        }

        inline void SetTemporaryGruu(IN const SipAddress* pTempGruu)
        {
            if (m_pTempGruu != IMS_NULL)
            {
                delete m_pTempGruu;
                m_pTempGruu = IMS_NULL;
            }

            if (pTempGruu != IMS_NULL)
            {
                m_pTempGruu = new SipAddress(*pTempGruu);
            }
        }

        inline void SetAssociatedUris(IN const AStringArray& objAssociatedUris)
        {
            m_objAssociatedUris = objAssociatedUris;
        }
        inline void SetSipProfile(IN SipProfile* pSipProfile) { m_pSipProfile = pSipProfile; }
        inline void SetEmergencyRegistration(IN IMS_BOOL bEmergencyRegistration)
        {
            m_bEmergencyRegistration = bEmergencyRegistration;
        }

    public:
        // port-uc / port-us
        IMS_SINT32 m_nPortUc;
        IMS_SINT32 m_nPortUs;
        // RFC5626_FLOW_CONTROL
        IMS_SINT32 m_nPortFlowControl;
        // MULTI_REG_TRANSPORT
        IMS_SINT32 m_nTransportExt;
        // IP address
        IpAddress m_objIp;
        // Contact address
        SipAddress m_objContactAddress;
        SipAddress* m_pContactAddressForOutgoingMessage;
        // Security-Client/Security-Verify headers
        AStringArray m_objSecurityClients;
        AStringArray m_objSecurityVerifys;

        // Public / Temporary GRUU
        SipAddress* m_pPubGruu;
        SipAddress* m_pTempGruu;

        // Authorized URIs
        AStringArray m_objAssociatedUris;
        RcPtr<SipProfile> m_pSipProfile;
        // Flag specifying whether the registration is for emergency or not.
        IMS_BOOL m_bEmergencyRegistration;
    };

    // In case of JSR 281, it MUST be a "imscore"
    AString m_strScheme;
    // Unique id for identifying the application which this service belongs to
    AString m_strAppId;
    // Unique id for identifying this service
    AString m_strServiceId;
    // IMS registry; Storage for application & service specific configurations
    AppConfig* m_pAppConfig;
    // Listener for monitoring the service close.
    IServiceCloseListener* m_piServiceCloseListener;

    // Registration state
    IMS_BOOL m_bImsConnected;
    IRegBinding* m_piRegBinding;
    // For cached reg-bindings to handle the existing dialog after IMS de-registration
    CachedRegBinding m_objCachedRegBinding;
    // MULTI_REG_SIP_PROFILE
    RcPtr<SipProfile> m_pSipProfile;

    // AOR; Public User Identity
    // P-Preferred-Identity, From, AOR for registration
    IMS_BOOL m_bProvisionedUserId;
    SipAddress m_objImpu;
    ImsList<ISipHeader*> m_objAuthorizedUserIds;

    // Default feature tags
    IMS_UINT32 m_nFeatureTags;
    // Caller preference; This field is included in Accept-Contact/Reject-Contact header.
    ImsList<PreferenceHeader*> m_objAcceptContacts;
    // Service capability; It is used to indicate the caller capability in Contact header
    CallerCapability* m_pCallerCapability;
    // Flag to track the caller capability changes
    IMS_BOOL m_bCallerCapabilityChanged;

    // Storage for Method class which maintains for some duration
    MethodManager* m_pMethodMngr;

    // Service specific initial filter criteria
    ServiceFilterCriteria* m_pFilterCriteria;

    // CONTACT_FEATURE_CAPS
    FeatureCaps* m_pFeatureCaps;
};

#endif
