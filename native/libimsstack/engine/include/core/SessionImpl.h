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
#ifndef SESSION_IMPL_H_
#define SESSION_IMPL_H_

#include "IOnSessionExListener.h"
#include "IOnSessionListener.h"
#include "ISession.h"
#include "SdpReader.h"
#include "SessionDescriptor.h"
#include "SessionEx.h"
#include "VirtualSessionImpl.h"

class ICapabilities;
class IMedia;
class IReference;
class ISubscription;
class MediaImpl;

class SessionImpl : public ISession, public IOnSessionListener, public IOnSessionExListener
{
public:
    explicit SessionImpl(IN SessionEx* pSession);
    ~SessionImpl() override;

    SessionImpl(IN const SessionImpl&) = delete;
    SessionImpl& operator=(IN const SessionImpl&) = delete;

public:
    inline SessionEx* GetSession() const { return m_pSession; }

private:
    // IMethod interface
    void Destroy() override;
    inline void SetMessageMediator(IN IMessageMediator* piMediator) override
    {
        m_pSession->SetMessageMediator(piMediator);
    }

    // IServiceMethod interface
    inline IMessage* GetNextRequest() override { return m_pSession->GetNextRequest(); }
    inline IMessage* GetNextResponse() override { return m_pSession->GetNextResponse(); }
    inline IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pSession->GetPreviousRequest(nServiceMethod);
    }
    inline IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pSession->GetPreviousResponse(nServiceMethod);
    }
    ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const override;
    inline ImsList<AString> GetRemoteUserId() const override
    {
        return m_pSession->GetRemoteUserId();
    }

    // ISession interface
    inline IMS_RESULT Accept() override { return m_pSession->Accept(); }
    ICapabilities* CreateCapabilities() override;
    IMedia* CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0) override;
    IReference* CreateReference(
            IN const AString& strReferTo, IN const AString& strReferMethod) override;
    ImsList<IMedia*> GetMedia() override;
    inline ISessionDescriptor* GetSessionDescriptor() override
    {
        return m_pSession->GetSessionDescriptor();
    }
    inline IMS_SINT32 GetState() const override { return m_pSession->GetState(); }
    inline IMS_BOOL HasPendingUpdate() const override { return m_pSession->HasPendingUpdate(); }
    inline IMS_RESULT Reject() override { return m_pSession->Reject(); }
    inline IMS_RESULT Reject(IN IMS_SINT32 nStatusCode) override
    {
        return m_pSession->Reject(nStatusCode);
    }
    inline IMS_RESULT RejectWithDiversion(IN const AString& strAlternativeUserAddress) override
    {
        return m_pSession->RejectWithDiversion(strAlternativeUserAddress);
    }
    IMS_RESULT RemoveMedia(IN IMedia* piMedia) override;
    inline IMS_RESULT Restore() override { return m_pSession->Restore(); }
    inline void SetListener(IN ISessionListener* piListener) override { m_piListener = piListener; }
    inline IMS_RESULT Start() override { return m_pSession->Start(); }
    inline IMS_RESULT Terminate() override { return m_pSession->Terminate(); }
    inline IMS_RESULT Update() override { return m_pSession->Update(); }

    ISubscription* CreateSubscription(IN const AString& strEvent) override;
    IPublication* CreatePublication(IN const AString& strEvent) override;
    inline ISipClientConnection* CreateTransaction(IN const SipMethod& objMethod) override
    {
        return m_pSession->CreateTransaction(objMethod);
    }
    inline IMS_SINT32 GetConfiguration() const override { return m_pSession->GetConfiguration(); }
    inline const ISipHeader* GetContactHeader() const override
    {
        return m_pSession->GetContactHeader();
    }
    inline const Replaces* GetReplaces() const override { return m_pSession->GetReplaces(); }
    inline const AString& GetSessionId() const override { return m_pSession->GetSessionId(); }
    inline IMS_SINT32 GetTerminationReason() const override
    {
        return m_pSession->GetTerminationReason();
    }
    inline IMS_BOOL IsFinalResponseReceivedForInitialInviteRequest() const override
    {
        return m_pSession->IsFinalResponseReceivedForInitialInviteRequest();
    }
    inline IMS_BOOL IsReliableProvResponseSupported() const override
    {
        return m_pSession->IsReliableProvResponseSupported();
    }
    inline IMS_BOOL IsSdpNegotiationAllowedForNonRpr() const override
    {
        return m_pSession->IsSdpNegotiationAllowedForNonRpr();
    }
    inline IMS_BOOL IsSdpOaInPreviewMode() const override
    {
        return m_pSession->IsSdpOaInPreviewMode();
    }
    inline IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode,
            IN const AString& strReasonPhrase = AString::ConstNull()) override
    {
        return m_pSession->RejectEx(nStatusCode, strReasonPhrase);
    }
    inline IMS_RESULT RespondToEarlyUpdate(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull()) override
    {
        return m_pSession->RespondToEarlyUpdate(nStatusCode, strReason);
    }
    inline IMS_RESULT RespondToPrack(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull()) override
    {
        return m_pSession->RespondToPrack(nStatusCode, strReason);
    }
    inline IMS_RESULT SendAck() override { return m_pSession->SendAck(); }
    inline IMS_RESULT SendPrack() override { return m_pSession->SendPrack(); }
    inline IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_SINT32 nFlags = 0) override
    {
        return m_pSession->SendProvisionalResponse(nStatusCode, strReason, nFlags);
    }
    inline IMS_RESULT SendRpr(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_BOOL bSdp = IMS_TRUE,
            IN IMS_SINT32 nFlags = 0) override
    {
        return m_pSession->SendRpr(nStatusCode, strReason, bSdp, nFlags);
    }
    inline IMS_RESULT SetCallerPreference(IN const ImsList<AString>& objCallerPreference) override
    {
        return m_pSession->SetCallerPreference(objCallerPreference);
    }
    inline void SetConfiguration(IN IMS_SINT32 nConfigValue) override
    {
        m_pSession->SetConfiguration(nConfigValue);
    }
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    inline IMS_RESULT SetContactParameter(IN const AString& strParameter,
            IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */) override
    {
        return m_pSession->SetContactParameter(strParameter, nOperation);
    }
    inline void SetImplicitRoutingRequired(IN IMS_BOOL bFlag) override
    {
        return m_pSession->SetImplicitRoutingRequired(bFlag);
    }
    inline void SetReasonForCallTermination(IN IMS_SINT32 nReason) override
    {
        return m_pSession->SetReasonForCallTermination(nReason);
    }
    inline void SetRefreshListener(IN IRefreshListener* piListener) override
    {
        m_pSession->SetRefreshListener(piListener);
    }
    inline void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) override
    {
        m_pSession->SetRefreshPolicy(nPolicy, nCriteriaInterval, nValueEorLt, nValueGt);
    }
    inline IMS_RESULT TerminateEx(IN IMS_BOOL bTerminateMethodBye = IMS_FALSE) override
    {
        return m_pSession->TerminateEx(bTerminateMethodBye);
    }
    inline IMS_RESULT UpdateEarlyMedia() override { return m_pSession->UpdateEarlyMedia(); }
    inline IMS_RESULT UpdateEx(IN IMS_SINT32 nMethod = SipMethod::INVALID,
            IN IMS_BOOL bSessionRefresh = IMS_FALSE) override
    {
        return m_pSession->UpdateEx(nMethod, bSessionRefresh);
    }

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    inline IMS_RESULT CreateFailureSdp() override { return m_pSession->CreateFailureSdp(); }
    inline void DestroyFailureSdp() override { m_pSession->DestroyFailureSdp(); }
    inline ISessionParameter* GetFailureSdp() const override { return m_pSession->GetFailureSdp(); }
    // }
    // EARLY_SESSION_MODEL {
    inline ISession* GetOwnerSession() const override { return IMS_NULL; }
    inline ISession* GetVirtualSession() const override { return m_pVirtualSessionImpl; }
    // }
    IMS_BOOL IsSessionRefreshInProgress() const override
    {
        return m_pSession->IsSessionRefreshInProgress();
    }
    inline void SetReasonHeaderSetter(IN IReasonHeaderSetter* piSetter) override
    {
        return m_pSession->SetReasonHeaderSetter(piSetter);
    }
    inline ISdpReader* GetRemoteMediaCapabilities() const override
    {
        return m_pSession->GetRemoteMediaCapabilities();
    }
    inline IMS_BOOL IsSessionCanceledOnAccepted() const override
    {
        return m_pSession->IsSessionCanceledOnAccepted();
    }
    inline void AbortEarlyUpdateTransaction() override
    {
        return m_pSession->AbortEarlyUpdateTransaction();
    }

    // IOnSessionListener interface
    void OnSession_Alerting(IN Session* pSession) override;
    void OnSession_ReferenceReceived(IN Session* pSession, IN Reference* pReference) override;
    void OnSession_Started(IN Session* pSession) override;
    void OnSession_StartFailed(IN Session* pSession) override;
    void OnSession_Terminated(IN Session* pSession) override;
    void OnSession_Updated(IN Session* pSession) override;
    void OnSession_UpdateFailed(IN Session* pSession) override;
    void OnSession_UpdateReceived(IN Session* pSession) override;
    void OnSession_CanceledOnAccepted(IN Session* pSession) override;
    void OnSession_CancelDelivered(IN Session* pSession) override;
    void OnSession_CancelDeliveryFailed(IN Session* pSession) override;
    IMS_BOOL OnSession_ForkedResponseReceived(
            IN Session* pSession, IN Session* pForkedSession) override;
    void OnSession_ProvisionalResponseReceived(IN Session* pSession,
            IN IMS_UINT32 nIndex = Session::INDEX_MOST_RECENT_MESSAGE) override;
    IMS_BOOL OnSession_TransactionReceived(
            IN Session* pSession, IN ISipServerConnection* piSsc) override;

    // IOnSessionExListener interface
    void OnSessionEx_EarlyMediaUpdated(IN SessionEx* pSessionEx) override;
    void OnSessionEx_EarlyMediaUpdateFailed(IN SessionEx* pSessionEx) override;
    void OnSessionEx_EarlyMediaUpdateReceived(IN SessionEx* pSessionEx) override;
    void OnSessionEx_PrackDelivered(IN SessionEx* pSessionEx) override;
    void OnSessionEx_PrackDeliveryFailed(IN SessionEx* pSessionEx) override;
    void OnSessionEx_PrackReceived(IN SessionEx* pSessionEx) override;
    void OnSessionEx_RprDeliveryFailed(IN SessionEx* pSessionEx) override;
    void OnSessionEx_RprReceived(IN SessionEx* pSessionEx, IN VirtualSession* pVirtualSession,
            IN IMS_UINT32 nIndex = Session::INDEX_MOST_RECENT_MESSAGE) override;

    void UpdateVirtualSession(IN VirtualSession* pVirtualSession);

private:
    SessionEx* m_pSession;
    ISessionListener* m_piListener;
    ImsList<MediaImpl*> m_objMediaImpls;
    VirtualSessionImpl* m_pVirtualSessionImpl;
};

#endif
