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
#ifndef VIRTUAL_SESSION_IMPL_H_
#define VIRTUAL_SESSION_IMPL_H_

#include "ISession.h"
#include "VirtualSession.h"

class IMedia;
class ISessionDescriptor;
class MediaImpl;

class VirtualSessionImpl : public ISession
{
public:
    explicit VirtualSessionImpl(IN ISession* piOwnerSession, IN VirtualSession* pSession);
    ~VirtualSessionImpl() override;

    VirtualSessionImpl(IN const VirtualSessionImpl&) = delete;
    VirtualSessionImpl& operator=(IN const VirtualSessionImpl&) = delete;

public:
    inline VirtualSession* GetSession() const { return m_pSession.Get(); }
    inline void UpdateSession(IN VirtualSession* pSession) { m_pSession = pSession; }

private:
    // IMethod interface
    void Destroy() override;
    void SetMessageMediator(IN IMessageMediator* piMediator) override;

    // IServiceMethod interface
    IMessage* GetNextRequest() override;
    IMessage* GetNextResponse() override;
    IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const override;
    IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const override;
    ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const override;
    ImsList<AString> GetRemoteUserId() const override;

    // ISession interface
    IMS_RESULT Accept() override;
    ICapabilities* CreateCapabilities() override;
    IMedia* CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0) override;
    IReference* CreateReference(
            IN const AString& strReferTo, IN const AString& strReferMethod) override;
    ImsList<IMedia*> GetMedia() override;
    ISessionDescriptor* GetSessionDescriptor() override;
    IMS_SINT32 GetState() const override;
    IMS_BOOL HasPendingUpdate() const override;
    IMS_RESULT Reject() override;
    IMS_RESULT Reject(IN IMS_SINT32 nStatusCode) override;
    IMS_RESULT RejectWithDiversion(IN const AString& strAlternativeUserAddress) override;
    IMS_RESULT RemoveMedia(IN IMedia* piMedia) override;
    IMS_RESULT Restore() override;
    void SetListener(IN ISessionListener* piListener) override;
    IMS_RESULT Start() override;
    IMS_RESULT Terminate() override;
    IMS_RESULT Update() override;

    ISubscription* CreateSubscription(IN const AString& strEvent) override;
    IPublication* CreatePublication(IN const AString& strEvent) override;
    ISipClientConnection* CreateTransaction(IN const SipMethod& objMethod) override;
    IMS_SINT32 GetConfiguration() const override;
    const ISipHeader* GetContactHeader() const override;
    const Replaces* GetReplaces() const override;
    const AString& GetSessionId() const override;
    IMS_SINT32 GetTerminationReason() const override;
    IMS_BOOL IsFinalResponseReceivedForInitialInviteRequest() const override;
    IMS_BOOL IsReliableProvResponseSupported() const override;
    IMS_BOOL IsSdpNegotiationAllowedForNonRpr() const override;
    IMS_BOOL IsSdpOaInPreviewMode() const override;
    IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode,
            IN const AString& strReasonPhrase = AString::ConstNull()) override;
    IMS_RESULT RespondToEarlyUpdate(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull()) override;
    IMS_RESULT RespondToPrack(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull()) override;
    IMS_RESULT SendAck() override;
    IMS_RESULT SendPrack() override;
    IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_SINT32 nFlags = 0) override;
    IMS_RESULT SendRpr(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_BOOL bSdp = IMS_TRUE,
            IN IMS_SINT32 nFlags = 0) override;
    IMS_RESULT SetCallerPreference(IN const ImsList<AString>& objCallerPreference) override;
    void SetConfiguration(IN IMS_SINT32 nConfigValue) override;
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    IMS_RESULT SetContactParameter(IN const AString& strParameter,
            IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */) override;
    void SetImplicitRoutingRequired(IN IMS_BOOL bFlag) override;
    void SetReasonForCallTermination(IN IMS_SINT32 nReason) override;
    void SetRefreshListener(IN IRefreshListener* piListener) override;
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) override;
    IMS_RESULT TerminateEx(IN IMS_BOOL bTerminateMethodBye = IMS_FALSE) override;
    IMS_RESULT UpdateEarlyMedia() override;
    IMS_RESULT UpdateEx(IN IMS_SINT32 nMethod = SipMethod::INVALID,
            IN IMS_BOOL bSessionRefresh = IMS_FALSE) override;

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    IMS_RESULT CreateFailureSdp() override;
    void DestroyFailureSdp() override;
    ISessionParameter* GetFailureSdp() const override;
    // }
    // EARLY_SESSION_MODEL {
    inline ISession* GetOwnerSession() const override { return m_piOwnerSession; }
    inline ISession* GetVirtualSession() const override { return IMS_NULL; }
    // }
    inline IMS_BOOL IsSessionRefreshInProgress() const override { return IMS_FALSE; }
    inline void SetReasonHeaderSetter(IN IReasonHeaderSetter* /*piSetter*/) override {}
    inline ISdpReader* GetRemoteMediaCapabilities() const override { return IMS_NULL; }
    inline IMS_BOOL IsSessionCanceledOnAccepted() const override { return IMS_FALSE; }
    inline void AbortEarlyUpdateTransaction() override {}

private:
    ISession* m_piOwnerSession;
    RcPtr<VirtualSession> m_pSession;
    ImsList<MediaImpl*> m_objMediaImpls;
};

#endif
