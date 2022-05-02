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

class ISIPMessage;
class ISIPClientConnection;
class ExtraHeaders;
class ImsSubscriberInfo;
class SIPSecurityHeader;



class RegParameter
    : public ImsSlot
    , public IRegParameter
{
public:
    explicit RegParameter(IN IMS_SINT32 nSlotId);
    virtual ~RegParameter();

private:
    RegParameter(IN CONST RegParameter &objRHS);
    RegParameter& operator=(IN CONST RegParameter &objRHS);

public:
    // IRegParameter interface
    virtual IMS_SINT32 GetPort() const;
    virtual const SIPAddress& GetTopmostRouteAddress() const;
    virtual void SetSecurityVerifys(IN CONST IMSList<SIPSecurityHeader> &objSecurityVerifys);

    IMS_RESULT FormHeaders(IN_OUT ISIPClientConnection *&piSCC,
            IN CONST RCPtr<RegStateTracker> pStateTracker);
    IMS_RESULT FormRouteHeaders(IN_OUT ISIPClientConnection *&piSCC,
            IN CONST RCPtr<RegStateTracker> pStateTracker);
    IMS_RESULT FormSecurityHeaders(IN_OUT ISIPClientConnection *&piSCC);
    Credential& GetCredential();
    IMS_SINT32 GetFlowControlOption() const;
    IMS_SINT32 GetPortFlowControl() const;
    const AStringArray& GetPreloadedRoutes() const;
    IMS_SINT32 GetProtectedPortUC() const;
    IMS_SINT32 GetProtectedPortUS() const;
    const IMSList<SIPSecurityHeader>& GetSecurityClients() const;
    const IMSList<SIPSecurityHeader>& GetSecurityVerifys() const;
    const SIPTimerValues* GetSIPTimerValues() const;
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
    IMS_BOOL UpdateProfile(IN CONST SIPAddress &objAOR,
            IN CONST AString &strSubsId = AString::ConstNull());
    IMS_BOOL UpdateSecurityHeaders(IN CONST ISIPMessage *piSIPMsg);
    // MULTI_REG_SIP_PROFILE
    void UpdateSIPProfile(IN SIPProfile *pSIPProfile);

private:
    // IRegParameter interface
    virtual IMS_BOOL AddExtraHeaders(IN CONST AStringArray &objHeaders);
    virtual IMS_BOOL AddMessageBodyPart(IN ISIPMessageBodyPart *piBodyPart);
    virtual IMS_BOOL AddPreloadedRoute(IN CONST AString &strRoute);
    virtual IMS_BOOL AddPreloadedRoute(IN CONST AString &strHost, IN IMS_SINT32 nPort,
            IN CONST AString &strScheme = AString::ConstNull());
    virtual IMS_BOOL AddSecurityClient(IN CONST SIPSecurityHeader &objSecurityHeader);
    virtual const SIPSecurityHeader* GetPreferredSecurityClient() const;
    virtual const SIPSecurityHeader* GetPreferredSecurityServer() const;
    virtual const IMSList<SIPSecurityHeader>& GetSecurityServers() const;
    virtual void RemoveAllMessageBodyParts();
    virtual void RemoveAllPreloadedRoutes();
    virtual void RemoveExtraHeaders(IN CONST AStringArray &objHeaders);
    virtual void RemoveSecurityClients();
    virtual void SetAuthenticationCredentials(IN IMS_BOOL bPolicy);
    virtual void SetFlowControlOption(IN IMS_SINT32 nOption);
    virtual void SetPort(IN IMS_SINT32 nPort);
    virtual void SetPortFlowControl(IN IMS_SINT32 nPort);
    virtual void SetSIPTimerValues(IN CONST SIPTimerValues &objTVs);
    virtual void SetTransportExt(IN IMS_SINT32 nTransportExt);
    virtual void SetTransportExtForRegOnly(IN IMS_SINT32 nTransportExt);

    void ChoosePreferredSecurityClient();
    void ChoosePreferredSecurityServer();

    // MULTI_SUBS
    static const ImsSubscriberInfo* GetImsSubscriberInfo(IN IMS_SINT32 nSlotId,
            IN CONST SIPAddress &objAOR, IN CONST AString &strSubsId = AString::ConstNull());

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
    SIPAddress objTopmostRouteAddress;

    // Security-Client headers
    IMSList<SIPSecurityHeader> objSecurityClients;
    IMSList<SIPSecurityHeader> objNewSecurityClients;
    IMSList<SIPSecurityHeader> objOldSecurityClients;
    // Security-Server header
    IMSList<SIPSecurityHeader> objSecurityServers;
    IMSList<SIPSecurityHeader> objOldSecurityServers;
    // Security-Verify header
    IMSList<SIPSecurityHeader> objSecurityVerifys;
    IMSList<SIPSecurityHeader> objOldSecurityVerifys;

    // Preferred Security Client/Server
    SIPSecurityHeader *pPreferredSecurityClient;
    SIPSecurityHeader *pPreferredSecurityServer;

    // Extra headers from IMS registry
    ExtraHeaders *pExtraHeaders;
    // Message body parts which are always added in initial-REG/re-REG/de-REG
    IMSList<ISIPMessageBodyPart*> objBodyParts;

    // Credentials if present
    Credential objCredential;
    IMS_BOOL bIsAuthRealmLenient;

    // Timer values of SIP transaction layer for this registration
    SIPTimerValues *pSIPTVs;
};

#endif // _REG_PARAMETER_H_
