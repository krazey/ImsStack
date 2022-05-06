/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090527  toastops@                 Created
    </table>

    Description
*/

#ifndef _CORE_SERVICE_IMPL_H_
#define _CORE_SERVICE_IMPL_H_

#include "ICoreService.h"
#include "IOnCoreServiceListener.h"
#include "IOnDirectCoreServiceListener.h"
#include "CoreService.h"

class ISipConnectionNotifier;
class ISipServerConnection;

class CoreServiceImpl :
        public ICoreService,
        public IOnCoreServiceListener,
        public IOnDirectCoreServiceListener
{
public:
    CoreServiceImpl(IN CoreService* pCoreService_);
    virtual ~CoreServiceImpl();

private:
    CoreServiceImpl(IN CONST CoreServiceImpl& objRHS);
    CoreServiceImpl& operator=(IN CONST CoreServiceImpl& objRHS);

private:
    // IConnection interface
    virtual void Close();

    // IService interface
    virtual const AString& GetAppId() const;
    virtual const AString& GetScheme() const;
    virtual const SipAddress& GetAuthorizedUserId() const;
    virtual const SipAddress& GetContactAddress() const;
    virtual const SipAddress* GetContactAddressForOutgoingMessage() const;
    virtual ISipHeader* GetContactHeader(IN IMS_BOOL bPrivacy = IMS_FALSE,
            IN IMS_BOOL bRequest = IMS_TRUE,
            IN IMS_SINT32 nSIPMethod = (-1) /* SipMethod::INVALID */) const;
    virtual IFeatureCaps* GetFeatureCaps() const;
    virtual IServiceFilterCriteria* GetFilterCriteria() const;
    virtual const AStringArray& GetPathHeaders() const;
    virtual const IRegInfo* GetRegInfo() const;
    virtual const IPAddress& GetIPAddress() const;
    // MULTI_REG_SIP_PROFILE
    virtual SipProfile* GetSIPProfile() const;
    virtual const AStringArray& GetUserIdentities() const;
    virtual const AString& GetUserIdentity(IN IMS_SINT32 nScheme) const;
    virtual const SipParameter* GetInstanceParameter() const;

    virtual const SipAddress* GetPublicGRUU() const;
    virtual const SipAddress* GetTemporaryGRUU() const;
    virtual const IMSList<SipAddress*>& GetTemporaryGRUUs() const;

    virtual IMS_BOOL IsBehindNAT() const;
    virtual IMS_BOOL IsImsConnected() const;
    virtual IMS_BOOL IsWithinTrustDomain() const;

    virtual IMS_BOOL AddFeatureTags(
            IN CONST IMSList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE);
    virtual IMS_BOOL RemoveFeatureTags(
            IN CONST IMSList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE);
    // MULTI_REG_SIP_PROFILE
    virtual void SetSIPProfile(IN SipProfile* pProfile);

    // ICoreService interface
    virtual ICapabilities* CreateCapabilities(IN CONST AString& strFrom, IN CONST AString& strTo);
    virtual IPageMessage* CreatePageMessage(IN CONST AString& strFrom, IN CONST AString& strTo);
    virtual IPublication* CreatePublication(
            IN CONST AString& strFrom, IN CONST AString& strTo, IN CONST AString& strEvent);
    virtual IReference* CreateReference(IN CONST AString& strFrom, IN CONST AString& strTo,
            IN CONST AString& strReferTo, IN CONST AString& strReferMethod);
    virtual ISession* CreateSession(IN CONST AString& strFrom, IN CONST AString& strTo);
    virtual ISubscription* CreateSubscription(
            IN CONST AString& strFrom, IN CONST AString& strTo, IN CONST AString& strEvent);
    virtual AString GetLocalUserId() const;
    virtual void SetListener(IN ICoreServiceListener* piListener);
    virtual ISIPConnectionFactory* CreateSIPConnectionFactory();
    virtual void SetDirectListener(IN IDirectCoreServiceListener* piListener);

    // IOnCoreServiceListener interface
    virtual void OnCoreService_PageMessageReceived(
            IN CoreService* pService, IN PageMessage* pMessage);
    virtual void OnCoreService_ReferenceReceived(
            IN CoreService* pService, IN Reference* pReference);
    virtual void OnCoreService_ServiceClosed(IN CoreService* pService, IN ReasonInfo* pReasonInfo);
    virtual void OnCoreService_SessionInvitationReceived(
            IN CoreService* pService, IN SessionEx* pSession);
    virtual void OnCoreService_UnsolicitedNotifyReceived(
            IN CoreService* pService, IN Message* pNotify);
    virtual void OnCoreService_CapabilityQueryReceived(
            IN CoreService* pService, IN Capabilities* pCapabilities);

    // IOnDirectCoreServiceListener interface
    virtual IMS_SINT32 OnDirectCoreService_TransactionReceived(
            IN CoreService* pService, IN ISIPConnectionFactory* piSCF);

private:
    ICoreServiceListener* piCoreServiceListener;
    IDirectCoreServiceListener* piDirectCoreServiceListener;

    CoreService* pService;
};

#endif  // _CORE_SERVICE_IMPL_H_
