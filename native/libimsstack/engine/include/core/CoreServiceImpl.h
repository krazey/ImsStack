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
#ifndef CORE_SERVICE_IMPL_H_
#define CORE_SERVICE_IMPL_H_

#include "CoreService.h"
#include "ICoreService.h"
#include "IOnCoreServiceListener.h"
#include "IOnDirectCoreServiceListener.h"

class CoreServiceImpl :
        public ICoreService,
        public IOnCoreServiceListener,
        public IOnDirectCoreServiceListener
{
public:
    explicit CoreServiceImpl(IN CoreService* pCoreService);
    ~CoreServiceImpl() override;

    CoreServiceImpl(IN const CoreServiceImpl&) = delete;
    CoreServiceImpl& operator=(IN const CoreServiceImpl&) = delete;

private:
    // IConnection interface
    void Close() override;

    // IService interface
    inline const AString& GetAppId() const override { return m_pService->GetAppId(); }
    inline const AString& GetScheme() const override { return m_pService->GetScheme(); }
    inline const SipAddress& GetAuthorizedUserId() const override
    {
        return m_pService->GetAuthorizedUserId();
    }
    inline const SipAddress& GetContactAddress() const override
    {
        return m_pService->GetContactAddress();
    }
    inline const SipAddress* GetContactAddressForOutgoingMessage() const override
    {
        return m_pService->GetContactAddressForOutgoingMessage();
    }
    inline ISipHeader* GetContactHeader(IN IMS_BOOL bPrivacy = IMS_FALSE,
            IN IMS_BOOL bRequest = IMS_TRUE,
            IN IMS_SINT32 nSipMethod = SipMethod::INVALID) const override
    {
        return m_pService->GetContactHeader(bPrivacy, bRequest, nSipMethod);
    }
    inline IFeatureCaps* GetFeatureCaps() const override { return m_pService->GetFeatureCaps(); }
    inline IServiceFilterCriteria* GetFilterCriteria() const override
    {
        return m_pService->GetFilterCriteria();
    }
    inline const AStringArray& GetPathHeaders() const override
    {
        return m_pService->GetPathHeaders();
    }
    inline const IRegInfo* GetRegInfo() const override { return m_pService->GetRegInfo(); }
    inline const IpAddress& GetIpAddress() const override { return m_pService->GetIpAddress(); }
    inline SipProfile* GetSipProfile() const override { return m_pService->GetSipProfile(); }
    inline const AStringArray& GetUserIdentities() const override
    {
        return m_pService->GetAssociatedUris();
    }
    inline const AString& GetUserIdentity(IN IMS_SINT32 nScheme) const override
    {
        return m_pService->GetAssociatedUri(nScheme);
    }
    inline const SipParameter* GetInstanceParameter() const override
    {
        return m_pService->GetInstanceParameter();
    }

    inline const SipAddress* GetPublicGruu() const override { return m_pService->GetPublicGruu(); }
    inline const SipAddress* GetTemporaryGruu() const override
    {
        return m_pService->GetTemporaryGruu();
    }
    inline const ImsList<SipAddress*>& GetTemporaryGruus() const override
    {
        return m_pService->GetTemporaryGruus();
    }

    inline IMS_BOOL IsBehindNat() const override { return m_pService->IsBehindNat(); }
    inline IMS_BOOL IsImsConnected() const override { return m_pService->IsImsConnected(); }
    inline IMS_BOOL IsWithinTrustDomain() const override
    {
        return m_pService->IsWithinTrustDomain();
    }

    inline IMS_BOOL AddFeatureTags(
            IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE) override
    {
        return m_pService->AddFeatureTags(objFeatureTags, bRegRequired);
    }
    inline IMS_BOOL RemoveFeatureTags(
            IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE) override
    {
        return m_pService->RemoveFeatureTags(objFeatureTags, bRegRequired);
    }
    inline void SetSipProfile(IN SipProfile* pProfile) override
    {
        m_pService->SetSipProfile(pProfile);
    }

    // ICoreService interface
    ICapabilities* CreateCapabilities(IN const AString& strFrom, IN const AString& strTo) override;
    IPageMessage* CreatePageMessage(IN const AString& strFrom, IN const AString& strTo) override;
    IPublication* CreatePublication(IN const AString& strFrom, IN const AString& strTo,
            IN const AString& strEvent) override;
    IReference* CreateReference(IN const AString& strFrom, IN const AString& strTo,
            IN const AString& strReferTo, IN const AString& strReferMethod) override;
    ISession* CreateSession(IN const AString& strFrom, IN const AString& strTo) override;
    ISubscription* CreateSubscription(IN const AString& strFrom, IN const AString& strTo,
            IN const AString& strEvent) override;
    inline AString GetLocalUserId() const override { return m_pService->GetLocalUserId(); }
    inline void SetListener(IN ICoreServiceListener* piListener) override
    {
        m_piCoreServiceListener = piListener;
    }
    inline ISipConnectionFactory* CreateSipConnectionFactory() override
    {
        return m_pService->CreateSipConnectionFactory();
    }
    void SetDirectListener(IN IDirectCoreServiceListener* piListener) override;

    // IOnCoreServiceListener interface
    void OnCoreService_PageMessageReceived(
            IN CoreService* pService, IN PageMessage* pMessage) override;
    void OnCoreService_ReferenceReceived(
            IN CoreService* pService, IN Reference* pReference) override;
    void OnCoreService_ServiceClosed(IN CoreService* pService, IN ReasonInfo* pReasonInfo) override;
    void OnCoreService_SessionInvitationReceived(
            IN CoreService* pService, IN SessionEx* pSession) override;
    void OnCoreService_UnsolicitedNotifyReceived(
            IN CoreService* pService, IN Message* pNotify) override;
    void OnCoreService_CapabilityQueryReceived(
            IN CoreService* pService, IN Capabilities* pCapabilities) override;

    // IOnDirectCoreServiceListener interface
    IMS_SINT32 OnDirectCoreService_TransactionReceived(
            IN CoreService* pService, IN ISipConnectionFactory* piScf) override;

private:
    CoreService* m_pService;
    ICoreServiceListener* m_piCoreServiceListener;
    IDirectCoreServiceListener* m_piDirectCoreServiceListener;
};

#endif
