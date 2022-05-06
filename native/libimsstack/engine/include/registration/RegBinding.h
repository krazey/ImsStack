/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100912  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_BINDING_H_
#define _REG_BINDING_H_

#include "RegObserver.h"
#include "IRegBinding.h"

class IRegistrationEx;
class IRegContact;
class ISipConnectionNotifier;

class RegBinding : public RegObserver, public IRegBinding
{
public:
    RegBinding();

protected:
    virtual ~RegBinding();

public:
    IMS_BOOL Create(IN IRegistrationEx* piRegEx);
    void Destroy();
    IMS_BOOL IsSameContact(IN IRegContact* piContact) const;
    IMS_BOOL IsSameRegistration(IN IRegistrationEx* piRegEx) const;
    void QueryCapability(OUT CallerCapability*& pCapability) const;
    void QueryRegistrationHeaders(OUT AStringArray& objHeaders) const;
    void UpdateContact(IN IRegContact* piContact);

protected:
    // RegObserver class
    virtual void Update(IN IMS_SINT32 nWhat);

    // IRegBinding class
    virtual const AStringArray& GetAssociatedURIs() const;
    virtual const SipAddress& GetAuthorizedAOR() const;
    virtual const SipAddress& GetContactAddress() const;
    virtual const SipAddress* GetContactAddressForOutgoingMessage() const;
    virtual const IPAddress& GetIPAddress() const;
    virtual const AStringArray& GetPathHeaders() const;
    virtual IMS_SINT32 GetPortFlowControl() const;
    virtual IMS_SINT32 GetPortUC() const;
    virtual IMS_SINT32 GetPortUS() const;
    virtual const IRegInfo* GetRegInfo() const;
    virtual const AStringArray& GetSecurityClients() const;
    virtual const AStringArray& GetSecurityVerifys() const;
    virtual const AStringArray& GetServiceRoutes() const;
    // MULTI_REG_SIP_PROFILE
    virtual SipProfile* GetSIPProfile() const;
    virtual IMS_SINT32 GetState() const;
    // MULTI_SUBS
    virtual const AString& GetSubscriberId() const;
    // MULTI_REG_TRANSPORT
    virtual IMS_SINT32 GetTransportExt() const;
    virtual const SipParameter* GetInstanceParameter() const;

    virtual const SipAddress* GetPublicGRUU() const;
    virtual const SipAddress* GetTemporaryGRUU() const;
    virtual const IMSList<SipAddress*>& GetTemporaryGRUUs() const;

    virtual IMS_BOOL IsBehindNAT() const;
    virtual IMS_BOOL IsWithinTrustDomain() const;
    virtual void NotifyCallerCapabilityChanged();
    virtual void SetListener(IN IRegBindingListener* piListener);

private:
    void CreateSIPConnectionNotifier();
    void DestroySIPConnectionNotifier();
    IMS_BOOL IsBindingActive() const;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    void RestoreTransportResourceForClientInitiatedConnection();
    void RestoreTransportResourceForServerConnection();
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    IRegistrationEx* piRegEx;
    IRegContact* piContact;

    IMS_SINT32 nState;
    ISipConnectionNotifier* piSCN;

    IRegBindingListener* piListener;

    // Flag to indicate that the reg-state is transited from ACTIVE to TERMINATED
    // since the registration procedure is failed by the txn timeout or failure response.
    IMS_BOOL bDeregistrationNOK;
};

#endif  // _REG_BINDING_H_
