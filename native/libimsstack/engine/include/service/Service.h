/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090527  toastops@                 Created
    </table>

    Description
     Service is the base class for IMS services, and follows
    the Generic Connection Framework (GCF).
*/

#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "IPAddress.h"
#include "Connection.h"
#include "Sip.h"
#include "SipAddress.h"
#include "SipMethod.h"
#include "SipProfile.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"
#include "IConfigUpdateListener.h"
#include "IRegBindingListener.h"
// CONTACT_FEATURE_CAPS
#include "FeatureCaps.h"
#include "ServiceFilterCriteria.h"

class SipConfigV;
class ISIPDialog;
class ISIPConnectionNotifier;
class ISIPMessage;
class ISIPClientConnection;
class ISIPServerConnection;
class IServiceManagerListener;
class IRegBinding;
class IRegInfo;
class CallerCapability;
class PreferenceHeader;
class Method;
class MethodManager;
class Capabilities;
class PageMessage;
class Reference;
class Session;
class SessionEx;



class Service
    : public Connection
    , public IConfigUpdateListener
    , public IRegBindingListener
{
public:
    Service(IN CONST AString &strScheme_, IN CONST AString& strAppId_,
            IN CONST AString &strServiceId_, IN CONST SIPAddress *pIMPU_ = IMS_NULL);
    virtual ~Service();

private:
    Service(IN CONST Service &objRHS);
    Service& operator=(IN CONST Service &objRHS);

public:
    // IService interface implementations
    virtual const AString& GetAppId() const;
    virtual const AString& GetScheme() const;

    virtual IMS_BOOL CreateConfig(IN CONST AppConfig &objAppConfig);
    virtual void HandleSessionInvitationReceived(IN Session *pSession);
    virtual void HandlePageMessageReceived(IN PageMessage *pPageMessage);
    virtual void HandleReferenceReceived(IN Reference *pReference);
    virtual void HandleCapabilityQueryReceived(IN Capabilities *pCapabilities);

    ISIPClientConnection* CreateConnection(IN CONST SIPAddress *pFrom, IN CONST SIPAddress *pTo,
            IN CONST SIPMethod &objMethod, IN IMS_BOOL bPrivacy = IMS_FALSE);
    ISIPClientConnection* CreateConnection(IN ISIPDialog *piDialog, IN CONST SIPMethod &objMethod,
            IN IMS_BOOL bPrivacy = IMS_FALSE);
    ISIPClientConnection* CreateCancelConnection(IN ISIPClientConnection* piSCC);
    IMS_BOOL CreateResponse(IN_OUT ISIPServerConnection *piSSC, IN IMS_SINT32 nStatusCode,
            IN CONST AString &strPhrase = AString::ConstNull(), IN IMS_BOOL bPrivacy = IMS_FALSE);
    IMS_BOOL InitAck(IN ISIPClientConnection* piSCC);
    IMS_BOOL Equals(IN CONST Service *pService) const;
    IMS_UINT32 EvaluateFilterCriteria(IN CONST ISIPMessage *piSIPMsg) const;
    const AString& GetServiceId() const;
    AppConfig* GetAppConfig() const;
    const CoreServiceConfig* GetServiceConfig() const;
    const IPAddress& GetIPAddress() const;
    const IMSList<PreferenceHeader*>& GetAcceptContactHeaders() const;
    const AString& GetAssociatedURI(IN IMS_SINT32 nScheme) const;
    const AStringArray& GetAssociatedURIs() const;
    const SIPAddress& GetAuthorizedUserId() const;
    const CallerCapability* GetCallerCapability() const;
    const SIPAddress& GetContactAddress() const;
    const SIPAddress* GetContactAddressForOutgoingMessage() const;
    ISIPHeader* GetContactHeader(
            IN IMS_BOOL bPrivacy = IMS_FALSE, IN IMS_BOOL bRequest = IMS_TRUE,
            IN IMS_SINT32 nSIPMethod = (-1) /* SIPMethod::INVALID */) const;
    const SIPAddress& GetDefaultUserId() const;
    // CONTACT_FEATURE_CAPS
    IFeatureCaps* GetFeatureCaps() const;
    ServiceFilterCriteria* GetFilterCriteria() const;
    const SIPParameter* GetInstanceParameter() const;
    const AStringArray& GetPathHeaders() const;
    const IRegInfo* GetRegInfo() const;
    const AStringArray& GetServiceRoutes() const;
    const ISipConfigV* GetISipConfigV() const;
    inline const SipConfigV* GetSipConfigV() const
    { return DYNAMIC_CAST(const SipConfigV*, GetISipConfigV()); }
    // MULTI_REG_SIP_PROFILE
    SIPProfile* GetSIPProfile() const;
    // MULTI_SUBS
    const AString& GetSubscriberId() const;

    const SIPAddress* GetPublicGRUU() const;
    const SIPAddress* GetTemporaryGRUU() const;
    const IMSList<SIPAddress*>& GetTemporaryGRUUs() const;

    IMS_BOOL IsBehindNAT() const;
    IMS_BOOL IsEventPackageSupported(IN CONST AString &strEvent) const;
    IMS_BOOL IsImsConnected() const;
    IMS_BOOL IsWithinTrustDomain() const;

    IMS_BOOL AddFeatureTags(IN CONST IMSList<AString> &objFeatureTags,
            IN IMS_BOOL bRegRequired = IMS_TRUE);
    IMS_BOOL RemoveFeatureTags(IN CONST IMSList<AString> &objFeatureTags,
            IN IMS_BOOL bRegRequired = IMS_TRUE);
    IMS_BOOL UpdateFeatureTags(IN CONST IMSList<AString> &objFeatureTags,
            IN IMS_BOOL bRegRequired = IMS_TRUE, IN IMS_SINT32 nOP = 1 /* 1: add, 2: remove */);

    void NotifyError(IN IMS_SINT32 nErrorCode);
    IMS_BOOL NotifyRequest(IN ISIPServerConnection *piSSC);
    // Sends SIP response
    IMS_BOOL SendResponse(IN ISIPServerConnection *piSSC, IN IMS_SINT32 nStatusCode,
            IN CONST AString &strPhrase = AString::ConstNull());
    void SetServiceManagerListener(IN IServiceManagerListener *piListener);
    // MULTI_REG_SIP_PROFILE
    void SetSIPProfile(IN SIPProfile *pProfile);
    void RegisterMethod(IN Method *pMethod);
    void DeregisterMethod(IN Method *pMethod);

    IMS_BOOL ValidateMethod(IN CONST SIPMethod &objMethod);
    IMS_BOOL ValidateRequestURI(IN CONST SIPAddress &objRequestURI,
            IN ISIPDialog *piDialog = IMS_NULL, IN IMS_BOOL bIsMidDialogRequest = IMS_FALSE);
    IMS_BOOL ValidateRequestURIForIPAndPort(IN CONST SIPAddress &objRequestURI,
            IN ISIPDialog *piDialog = IMS_NULL, IN IMS_BOOL bIsMidDialogRequest = IMS_FALSE);

    static IMS_BOOL ValidateFromAndTo(IN CONST AString &strFrom, IN CONST AString &strTo,
            IN IMS_BOOL bToLenient);
    static IMS_BOOL ValidateReferTo(IN CONST AString &strURI, IN CONST AString &strMethod);

protected:
    // Connection class - overrides
    virtual void Close();
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG &objMSG);

    // IConfigUpdateListener class
    virtual void ConfigUpdate_NotifyUpdate(IN IMS_SINT32 nCPI,
            IN const AString &strConfName = AString::ConstNull(),
            IN const AString &strExtraParam = AString::ConstNull());

    // IRegBindingListener class
    virtual void RegBinding_OnActive();
    virtual void RegBinding_OnDestroy();
    virtual void RegBinding_OnInit(IN CONST SIPAddress *pAOR);
    virtual void RegBinding_OnQueryCapability(OUT CallerCapability *&pCapability);
    virtual void RegBinding_OnQueryRegistrationHeaders(OUT AStringArray &objHeaders);
    virtual void RegBinding_OnTerminated();

    virtual void Abort();
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode) = 0;
    virtual IMS_BOOL ServerConnection_NotifyRequest(IN ISIPServerConnection *piSSC);

    void FormContactHeader(IN CONST SIPMethod &objMethod, IN IMS_BOOL bPrivacy,
            IN IMS_BOOL bRequest, OUT AString &strContact, OUT IMS_BOOL &bIsContactGRUU) const;
    IRegBinding* GetRegBinding() const;
    IMS_BOOL IsUserIdProvisioned() const;
    void SetGRUUOptionTagInMidDialog(IN ISIPDialog *piDialog, IN_OUT ISIPMessage *&piSIPMsg);

private:
    void CreateDefaultPublicUserId();
    IMS_UINT32 GetServiceCode() const;
    IMS_BOOL IsRegBindingOnActive() const;
    IMS_BOOL SetPPreferredIdentityHeader(IN IMS_SINT32 nPreferredId,
            IN_OUT ISIPMessage *&piSIPMsg);
    void SetRegBinding(IN IRegBinding *piRegBinding);
    void UpdateAuthorizedUserIds();
    void UpdateCallerCapabilityNPreference();
    void UpdateRegBindings();

protected:
    enum
    {
        AMSG_SERVICE_MAX = (AMSG_USER + 1)
    };

private:
    friend class ServiceResolver;

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    class CachedRegBinding
    {
    public:
        inline CachedRegBinding()
            : nPortUC(SIP::PORT_UNSPECIFIED)
            , nPortUS(SIP::PORT_UNSPECIFIED)
            , nPortFlowControl(SIP::PORT_UNSPECIFIED)
            , nTransportExt(SIP::TRANSPORT_EXT_ANY)
            , objIPA(IPAddress::NONE)
            , objContactAddress(SIPAddress::ConstNull())
            , pContactAddressForOutgoingMessage(IMS_NULL)
            , objSecurityClients(AStringArray::ConstNull())
            , objSecurityVerifys(AStringArray::ConstNull())
            , pPubGRUU(IMS_NULL)
            , pTempGRUU(IMS_NULL)
            , objAssociatedURIs(AStringArray::ConstNull())
        {}
        inline ~CachedRegBinding()
        {
            if (pContactAddressForOutgoingMessage != IMS_NULL)
            {
                delete pContactAddressForOutgoingMessage;
            }

            if (pPubGRUU != IMS_NULL)
            {
                delete pPubGRUU;
            }

            if (pTempGRUU != IMS_NULL)
            {
                delete pTempGRUU;
            }
        }

    private:
        CachedRegBinding(IN CONST CachedRegBinding &objRHS);
        const CachedRegBinding& operator=(IN CONST CachedRegBinding &objRHS);

    public:
        inline IMS_SINT32 GetPortUC() const
        { return nPortUC; }
        inline IMS_SINT32 GetPortUS() const
        { return nPortUS; }
        // RFC5626_FLOW_CONTROL
        inline IMS_SINT32 GetPortFlowControl() const
        { return nPortFlowControl; }
        // MULTI_REG_TRANSPORT
        inline IMS_SINT32 GetTransportExt() const
        { return nTransportExt; }
        inline const IPAddress& GetIPAddress() const
        { return objIPA; }
        inline const SIPAddress& GetContactAddress() const
        { return objContactAddress; }
        inline const SIPAddress* GetContactAddressForOutgoingMessage() const
        { return pContactAddressForOutgoingMessage; }

        inline const AStringArray& GetSecurityClients() const
        { return objSecurityClients; }
        inline const AStringArray& GetSecurityVerifys() const
        { return objSecurityVerifys; }

        inline const SIPAddress* GetPublicGRUU() const
        { return pPubGRUU; }
        inline const SIPAddress* GetTemporaryGRUU() const
        { return pTempGRUU; }

        inline const AStringArray& GetAssociatedURIs() const
        { return objAssociatedURIs; }

        inline void SetPortUC(IN IMS_SINT32 nPortUC)
        { this->nPortUC = nPortUC; }
        inline void SetPortUS(IN IMS_SINT32 nPortUS)
        { this->nPortUS = nPortUS; }
        // RFC5626_FLOW_CONTROL
        inline void SetPortFlowControl(IN IMS_SINT32 nPortFlowControl)
        { this->nPortFlowControl = nPortFlowControl; }
        // MULTI_REG_TRANSPORT
        inline void SetTransportExt(IN IMS_SINT32 nTransportExt)
        { this->nTransportExt = nTransportExt; }
        inline void SetIPAddress(IN CONST IPAddress &objIPA)
        { this->objIPA = objIPA; }
        inline void SetContactAddress(IN CONST SIPAddress &objContactAddress)
        { this->objContactAddress = objContactAddress; }
        inline void SetContactAddressForOutgoingMessage(IN CONST SIPAddress *pContactAddress)
        {
            if (pContactAddressForOutgoingMessage != IMS_NULL)
            {
                delete pContactAddressForOutgoingMessage;
                pContactAddressForOutgoingMessage = IMS_NULL;
            }

            if (pContactAddress != IMS_NULL)
            {
                pContactAddressForOutgoingMessage = new SIPAddress(*pContactAddress);
            }
        }
        inline void SetSecurityClients(IN CONST AStringArray& objSecurityClients)
        { this->objSecurityClients = objSecurityClients; }
        inline void SetSecurityVerifys(IN CONST AStringArray& objSecurityVerifys)
        { this->objSecurityVerifys = objSecurityVerifys; }

        inline void SetPublicGRUU(IN CONST SIPAddress *pPubGRUU)
        {
            if (this->pPubGRUU != IMS_NULL)
            {
                delete this->pPubGRUU;
                this->pPubGRUU = IMS_NULL;
            }

            if (pPubGRUU != IMS_NULL)
            {
                this->pPubGRUU = new SIPAddress(*pPubGRUU);
            }
        }

        inline void SetTemporaryGRUU(IN CONST SIPAddress *pTempGRUU)
        {
            if (this->pTempGRUU != IMS_NULL)
            {
                delete this->pTempGRUU;
                this->pTempGRUU = IMS_NULL;
            }

            if (pTempGRUU != IMS_NULL)
            {
                this->pTempGRUU = new SIPAddress(*pTempGRUU);
            }
        }

        inline void SetAssociatedURIs(IN CONST AStringArray &objAssociatedURIs)
        { this->objAssociatedURIs = objAssociatedURIs; }

    public:
        // port-uc / port-us
        IMS_SINT32 nPortUC;
        IMS_SINT32 nPortUS;
        // RFC5626_FLOW_CONTROL
        IMS_SINT32 nPortFlowControl;
        // MULTI_REG_TRANSPORT
        IMS_SINT32 nTransportExt;
        // IP address
        IPAddress objIPA;
        // Contact address
        SIPAddress objContactAddress;
        SIPAddress *pContactAddressForOutgoingMessage;
        // Security-Client/Security-Verify headers
        AStringArray objSecurityClients;
        AStringArray objSecurityVerifys;

        // Public / Temporary GRUU
        SIPAddress *pPubGRUU;
        SIPAddress *pTempGRUU;

        // Authorized URIs
        AStringArray objAssociatedURIs;
    };

    // In case of JSR 281, it MUST be a "imscore"
    AString strScheme;
    // Unique id for identifying the application which this service belongs to
    AString strAppId;
    // Unique id for identifying this service
    AString strServiceId;
    // IMS registry; Storage for application & service specific configurations
    AppConfig *pAppConfig;
    // Reference of ServiceManager listener
    IServiceManagerListener *piServiceManagerListener;

    // Registration state
    IMS_BOOL bImsConnected;
    IRegBinding *piRegBinding;
    // For cached reg-bindings to handle the existing dialog after IMS de-registration
    CachedRegBinding objCachedRegBinding;
    // MULTI_REG_SIP_PROFILE
    RCPtr<SIPProfile> pSIPProfile;

    // AOR; Public User Identity
    // P-Preferred-Identity, From, AOR for registration
    IMS_BOOL bFlag_ProvisionedUserId;
    SIPAddress objIMPU;
    IMSList<ISIPHeader*> objAuthorizedUserIds;

    // Default feature tags
    IMS_UINT32 nFeatureTags;
    // Caller preference; This field is included in Accept-Contact/Reject-Contact header.
    IMSList<PreferenceHeader*> objAcceptContacts;
    // Service capability; It is used to indicate the caller capability in Contact header
    CallerCapability *pCallerCapability;
    // Flag to track the caller capability changes
    IMS_BOOL bFlag_CallerCapabilityChanged;

    // Storage for Method class which maintains for some duration
    MethodManager *pMethodMngr;

    // Service specific initial filter criteria
    ServiceFilterCriteria *pFilterCriteria;

    // CONTACT_FEATURE_CAPS
    FeatureCaps *pFeatureCaps;
};

#endif // _SERVICE_H_
