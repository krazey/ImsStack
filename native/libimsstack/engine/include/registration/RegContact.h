/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090908  toastops@                 Created
    </table>

    Description

*/

#ifndef _REG_CONTACT_H_
#define _REG_CONTACT_H_

#include "AStringBuffer.h"
#include "ImsSlot.h"
#include "IRegContact.h"
#include "util/CallerCapability.h"

class SIPProfile;
class IRegCapabilityChangeListener;



class RegContact
    : public ImsSlot
    , public IRegContact
{
public:
    RegContact(IN IMS_SINT32 nSlotId,
            IN CONST IPAddress &objIPA_, IN IMS_SINT32 nPort_,
            IN IRegCapabilityChangeListener *piListener_, IN IMS_SINT32 nRegId_ = (-1),
            IN CONST SIPProfile *pSIPProfile = IMS_NULL);
    virtual ~RegContact();

private:
    RegContact(IN CONST RegContact &objRHS);
    RegContact& operator=(IN CONST RegContact &objRHS);

public:
    // IRegContact interface
    virtual const SIPAddress& GetContactAddress() const;
    virtual IMS_UINT32 GetExpires() const;
    virtual const IPAddress& GetIPAddress() const;
    virtual IMS_SINT32 GetPort() const;
    virtual const IMSList<SIPParameter*>& GetHeaderParameters() const;
    virtual const SIPParameter* GetInstanceParameter() const;
    virtual const SIPParameter* GetRegIdParameter() const;

    virtual const SIPAddress* GetPublicGRUU() const;
    virtual const SIPAddress* GetTemporaryGRUU() const;
    virtual const IMSList<SIPAddress*>& GetTemporaryGRUUs() const;

    virtual IMS_BOOL IsActiveBinding() const;
    virtual IMS_BOOL IsEmpty() const;

    void DestroyGRUU();

    IMS_UINT32 GetInitialExpires() const;
    const AString& GetPreference() const;
    IMS_SINT32 GetState() const;
    IMS_BOOL IsBindingsUpdated() const;
    IMS_BOOL IsExpirationValueSpecified() const;
    void Restore();
    void SetAOR(IN CONST SIPAddress &objAOR);
    void SetExpires(IN IMS_UINT32 nExpiresValue);
    // IMS_IPSEC_UDP_ENC
    void SetHostInfo(IN CONST IPAddress &objIP);
    void SetTerminated();
    AString ToString() const;
    AString ToStringWithExpires() const;
    // For fake registration
    IMS_SINT32 UpdateParameter(IN IMS_SINT32 nExpiresValue);
    IMS_SINT32 UpdateParameter(IN CONST IMSList<ISIPHeader*> &objContactHeaders,
            IN IMS_SINT32 nExpiresValue);

private:
    // IRegContact interface
    virtual IMS_BOOL AddHeaderParameter(IN CONST AString &strName,
            IN CONST AString &strValue = AString::ConstNull());
    virtual IMS_BOOL AddUriParameter(IN CONST AString &strName,
            IN CONST AString &strValue = AString::ConstNull());
    virtual void RemoveAllHeaderParameters();
    virtual void RemoveHeaderParameter(IN CONST AString &strName,
            IN CONST AString &strValue = AString::ConstNull());
    virtual void RemoveUriParameter(IN CONST AString &strName,
            IN CONST AString &strValue = AString::ConstNull());
    virtual void SetDisplayName(IN CONST AString &strDisplayName);
    virtual void SetListener(IN IRegContactListener *piListener);
    virtual void SetPolicyForCallerCapability(IN IMS_BOOL bCapsByApp);
    virtual void SetPort(IN IMS_SINT32 nPort);
    virtual void SetUserInfo(IN IMS_SINT32 nPolicy = POLICY_USER_INFO_IMPU,
            IN CONST AString &strUserInfo = AString::ConstNull());
    virtual IMS_BOOL AddExtraCapability(IN CONST AString &strName,
            IN CONST AString &strValue);
    virtual void RemoveExtraCapability(IN CONST AString &strName,
            IN CONST AString &strValue);
    virtual IMS_BOOL AddService(IN CONST AString &strAppId, IN CONST AString &strServiceId);
    virtual void RemoveService(IN CONST AString &strAppId, IN CONST AString &strServiceId);
    virtual IMS_BOOL IsServiceRegistered(IN const AString &strAppId,
            IN const AString &strServiceId) const;
    virtual IMS_BOOL IsFeatureRegistered(IN const AString &strFTName,
            IN const AString &strFTValue = AString::ConstNull()) const;
    virtual void RecalculateCallerCapabilities();

    void FormContact(IN IMS_BOOL bExpiresRequired, OUT AStringBuffer &objSB) const;
    IMS_BOOL AddCallerCapability(IN CONST CallerCapability *pCC);
    IMS_BOOL RemoveCallerCapability(IN CONST CallerCapability *pCC);
    IMS_BOOL RegisterServiceCapability(IN CONST CallerCapability *pCC);
    void UnregisterServiceCapability(IN CONST CallerCapability *pCC);
    void SetState(IN IMS_SINT32 nState);
    void UpdateGRUU(IN CONST ISIPHeader *piHeader);
    void UpdateRegisteredCapabilities(IN const ISIPHeader* piHeader);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // State of the contact
    enum
    {
        STATE_CREATED,
        STATE_ACTIVE,
        STATE_TERMINATED
    };

    // Result of parameter updates
    enum
    {
        UPDATE_OK = 0x00,
        UPDATE_NO_EXPIRES = 0x01
    };

    enum { EXPIRES_NOT_SPECIFIED = (0xFFFFFFFF) };

private:
    // State of the contact
    IMS_SINT32 nState;
    SIPAddress *pAOR;
    // URI for Contact header
    IPAddress objIPA;
    IMS_SINT32 nPolicyUserInfo;
    SIPAddress objContactAddress;

    // Header parameter: +sip.instance
    SIPParameter *pInstanceParameter;
    // Header parameter: reg-id
    SIPParameter *pRegIdParameter;

    SIPAddress *pPubGRUU;
    SIPAddress *pTempGRUU;
    IMSList<SIPAddress*> objTempGRUUs;

    // All the caller capabilities for this contact
    IMS_BOOL bFlag_BindingsUpdateTracker;
    // Capability for Contact header
    IMS_BOOL bFlag_AllCapabilitiesByConfig;
    CallerCapability *pAllCapabilities;
    CallerCapability *pExtraCapabilities;
    // Caller capability for each service
    IMSList<CallerCapability*> objCallerCapabilities;
    IRegCapabilityChangeListener *piCapabilityChangeListener;
    // Registered capabilities: composed from 200OK-REGISTER
    CallerCapability* pRegisteredCapabilities;

    // Expiration time for this contact
    //4 origin expires & network provisioned expires
    IMS_UINT32 nInitialExpires;
    IMS_UINT32 nNetworkProvisionedExpires;

    // Header parameters
    IMSList<SIPParameter*> objHeaderParams;
};

#endif // _REG_CONTACT_H_
