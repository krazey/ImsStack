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
#ifndef VIRTUAL_SESSION_H_
#define VIRTUAL_SESSION_H_

#include "RcObject.h"

#include "ISessionState.h"
#include "Service.h"

class ISipMessage;
class Media;
class SdpOaState;
class SessionDescriptor;
class SipAddress;

class VirtualSession : public RcObject, public ISessionState
{
public:
    explicit VirtualSession(IN Service* pService, IN const SipAddress* pUserAor);
    VirtualSession(IN const VirtualSession& other);
    ~VirtualSession() override;

public:
    VirtualSession& operator=(IN const VirtualSession&) = delete;

public:
    IMS_BOOL CheckNSetSdpBodyPart(IN_OUT ISipMessage*& piSipMsg);
    IMS_RESULT Notify18xResponse(IN const ISipMessage* piSipMsg);
    void NotifyPrackSent(IN const ISipMessage* piSipMsg);

    Media* CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0);
    const ImsList<Media*>& GetMedia() const;
    SessionDescriptor* GetSessionDescriptor();
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_RESULT RemoveMedia(IN Media* pMedia);
    IMS_RESULT RemoveMedia(IN IMS_UINT32 nIndex);

protected:
    // ISessionState interface
    const AString& GetConnectionAddress() const override;
    IMS_SINT32 GetSessionState() const override;
    SdpSessionParameter* GetSessionParameter() const override;
    const AString& GetPeerConnectionAddress() const override;
    SdpSessionParameter* GetPeerSessionParameter() const override;
    SdpSessionParameter* GetProposalSessionParameter() override;

    // Methods for handling SDP & Session Descriptor related operations
    IMS_BOOL CheckNCreateSessionDescriptor();
    IMS_SINT32 GetOfferAnswerState() const;
    IMS_SINT32 HandleSdpOfferAnswer(IN const ISipMessage* piSipMsg);
    void RestoreEx();
    IMS_BOOL UpdateMedia(IN IMS_SINT32 nTrigger);
    IMS_BOOL RestoreOfferAnswerState();
    IMS_BOOL UpdateOfferAnswerStateOnMessageReceived(IN const ISipMessage* piSipMsg);
    IMS_BOOL UpdateOfferAnswerStateOnMessageSent(IN const ISipMessage* piSipMsg);

private:
    void Init(IN const SipAddress* pUserAor);
    inline IMS_SINT32 GetSlotId() const
    {
        return (m_pService != IMS_NULL) ? m_pService->GetSlotId() : IMS_SLOT_ANY;
    }
    void SetState(IN IMS_SINT32 nState);

    // Methods for handling SDP & Media related operations
    IMS_BOOL AddMedia(IN Media* pMedia);
    void CleanupMedia();
    IMS_BOOL UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Refer to ISession class
    enum
    {
        STATE_CREATED = 0,
        STATE_INITIATED = 1,
        STATE_NEGOTIATING = 2,
        STATE_ESTABLISHING = 3,
        STATE_ESTABLISHED = 4,
        STATE_RENEGOTIATING = 5,
        STATE_REESTABLISHING = 6,
        STATE_TERMINATING = 7,
        STATE_TERMINATED = 8
    };

private:
    Service* m_pService;
    SipAddress m_objUserAor;
    IMS_SINT32 m_nState;
    SdpOaState* m_pOaState;
    SessionDescriptor* m_pSessionDescriptor;
    ImsList<Media*> m_objMedias;
};

#endif
