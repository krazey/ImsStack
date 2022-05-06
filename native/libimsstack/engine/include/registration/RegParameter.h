/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090908  toastops@                 Created
    </table>

    Description

*/

#ifndef _REG_PARAMETER_H_
#define _REG_PARAMETER_H_

#include "AStringArray.h"
#include "SipAddress.h"
#include "Credential.h"
#include "IRegParameter.h"
#include "ImsSlot.h"
#include "RegStateTracker.h"

class ISipMessage;
class ISipClientConnection;
class ExtraHeaders;
class ImsSubscriberInfo;
class SipSecurityHeader;

class RegParameter : public ImsSlot, public IRegParameter
{
public:
    explicit RegParameter(IN IMS_SINT32 nSlotId);
    virtual ~RegParameter();

private:
    RegParameter(IN CONST RegParameter& objRHS);
    RegParameter& operator=(IN CONST RegParameter& objRHS);

public:
    // IRegParameter interface
    virtual IMS_SINT32 GetPort() const;
    virtual const SipAddress& GetTopmostRouteAddress() const;
    virtual void SetSecurityVerifys(IN CONST IMSList<SipSecurityHeader>& objSecurityVerifys);

    IMS_RESULT FormHeaders(
            IN_OUT ISipClientConnection*& piSCC, IN CONST RCPtr<RegStateTracker> pStateTracker);
    IMS_RESULT FormRouteHeaders(
            IN_OUT ISipClientConnection*& piSCC, IN CONST RCPtr<RegStateTracker> pStateTracker);
    IMS_RESULT FormSecurityHeaders(IN_OUT ISipClientConnection*& piSCC);
    Credential& GetCredential();
    IMS_SINT32 GetFlowControlOption() const;
    IMS_SINT32 GetPortFlowControl() const;
    const AStringArray& GetPreloadedRoutes() const;
    IMS_SINT32 GetProtectedPortUC() const;
    IMS_SINT32 GetProtectedPortUS() const;
    const IMSList<SipSecurityHeader>& GetSecurityClients() const;
    const IMSList<SipSecurityHeader>& GetSecurityVerifys() const;
    const SipTimerValues* GetSIPTimerValues() const;
    IMS_SINT32 GetTransportExt() const;
    IMS_SINT32 GetTransportExtForRegOnly() const;
    IMS_BOOL IsAuthRealmLenient() const;
    IMS_BOOL IsSecurityAssociationPresent() const;
    IMS_BOOL IsSecurityAssociationRequired() const;
    // IMS_IPSEC_UDP_ENC
    IMS_BOOL IsSecurityAssociationRequiredViaUDPEnc() const;
    void RemovePreferredSecurityHeaders();
    void RemoveSecurityServers();
    void Restore();
    void RestoreSecurityHeaders();
    void SetTransportExtForIPSec();
    // MULTI_SUBS
    IMS_BOOL UpdateProfile(
            IN CONST SipAddress& objAOR, IN CONST AString& strSubsId = AString::ConstNull());
    IMS_BOOL UpdateSecurityHeaders(IN CONST ISipMessage* piSIPMsg);
    // MULTI_REG_SIP_PROFILE
    void UpdateSIPProfile(IN SipProfile* pSIPProfile);

private:
    // IRegParameter interface
    virtual IMS_BOOL AddExtraHeaders(IN CONST AStringArray& objHeaders);
    virtual IMS_BOOL AddMessageBodyPart(IN ISipMessageBodyPart* piBodyPart);
    virtual IMS_BOOL AddPreloadedRoute(IN CONST AString& strRoute);
    virtual IMS_BOOL AddPreloadedRoute(IN CONST AString& strHost, IN IMS_SINT32 nPort,
            IN CONST AString& strScheme = AString::ConstNull());
    virtual IMS_BOOL AddSecurityClient(IN CONST SipSecurityHeader& objSecurityHeader);
    virtual const SipSecurityHeader* GetPreferredSecurityClient() const;
    virtual const SipSecurityHeader* GetPreferredSecurityServer() const;
    virtual const IMSList<SipSecurityHeader>& GetSecurityServers() const;
    virtual void RemoveAllMessageBodyParts();
    virtual void RemoveAllPreloadedRoutes();
    virtual void RemoveExtraHeaders(IN CONST AStringArray& objHeaders);
    virtual void RemoveSecurityClients();
    virtual void SetAuthenticationCredentials(IN IMS_BOOL bPolicy);
    virtual void SetFlowControlOption(IN IMS_SINT32 nOption);
    virtual void SetPort(IN IMS_SINT32 nPort);
    virtual void SetPortFlowControl(IN IMS_SINT32 nPort);
    virtual void SetSIPTimerValues(IN CONST SipTimerValues& objTVs);
    virtual void SetTransportExt(IN IMS_SINT32 nTransportExt);
    virtual void SetTransportExtForRegOnly(IN IMS_SINT32 nTransportExt);

    void ChoosePreferredSecurityClient();
    void ChoosePreferredSecurityServer();

    // MULTI_SUBS
    static const ImsSubscriberInfo* GetImsSubscriberInfo(IN IMS_SINT32 nSlotId,
            IN CONST SipAddress& objAOR, IN CONST AString& strSubsId = AString::ConstNull());

private:
    // Policy for Authorization header
    IMS_BOOL bPolicyForAuthenticationCredentials;
    // MULTI_REG_TRANSPORT
    IMS_SINT32 nTransportExt;
    IMS_SINT32 nTransportExtForRegOnly;
    // Default SIP port number
    IMS_SINT32 nPort;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 nFlowControlOption;
    IMS_SINT32 nPortFlowControl;

    // Address of S-CSCF (Registrar)
    AString strServingCSCF;

    // Preloaded route set (in this moment, only one pre-configured route exists)
    AStringArray objPreloadedRoutes;
    // Topmost Route address
    SipAddress objTopmostRouteAddress;

    // Security-Client headers
    IMSList<SipSecurityHeader> objSecurityClients;
    IMSList<SipSecurityHeader> objNewSecurityClients;
    IMSList<SipSecurityHeader> objOldSecurityClients;
    // Security-Server header
    IMSList<SipSecurityHeader> objSecurityServers;
    IMSList<SipSecurityHeader> objOldSecurityServers;
    // Security-Verify header
    IMSList<SipSecurityHeader> objSecurityVerifys;
    IMSList<SipSecurityHeader> objOldSecurityVerifys;

    // Preferred Security Client/Server
    SipSecurityHeader* pPreferredSecurityClient;
    SipSecurityHeader* pPreferredSecurityServer;

    // Extra headers from IMS registry
    ExtraHeaders* pExtraHeaders;
    // Message body parts which are always added in initial-REG/re-REG/de-REG
    IMSList<ISipMessageBodyPart*> objBodyParts;

    // Credentials if present
    Credential objCredential;
    IMS_BOOL bIsAuthRealmLenient;

    // Timer values of SIP transaction layer for this registration
    SipTimerValues* pSIPTVs;
};

#endif  // _REG_PARAMETER_H_
