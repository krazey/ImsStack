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

#ifndef CONFERENCE_REFERENCE_H_
#define CONFERENCE_REFERENCE_H_

#include "ImsList.h"
#include "AString.h"
#include "IReferenceListener.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "conferencecall/IConferenceReference.h"

class CallConnectionIdManager;
class IReference;
class IMtcContext;
class IConferenceReferenceListener;

class ConferenceReference final : public IReferenceListener, public IConferenceReference
{
public:
    explicit ConferenceReference(IN IMtcContext& objContext, IN CallKey nConfCallKey,
            IN ConfUser* pConfUser, IN IConferenceReferenceListener& objListener);
    explicit ConferenceReference(IN IMtcContext& objContext, IN CallKey nConfCallKey,
            IN ImsList<ConfUser*>& objConfUsers, IN IConferenceReferenceListener& objListener);
    virtual ~ConferenceReference();
    ConferenceReference(IN const ConferenceReference&) = delete;
    ConferenceReference& operator=(IN const ConferenceReference&) = delete;

public:
    // IReferenceListener interface implementation
    void ReferenceDelivered(IN IReference* piReference) override;
    void ReferenceDeliveryFailed(IN IReference* piReference) override;
    void ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify) override;
    void ReferenceTerminated(IN IReference* piReference) override;

    // IConferenceReference interface implementation
    IMS_RESULT SendInvite(OUT AString& strReferToUri,
            IN CallConnectionIdManager& objConnectionIdManager) override;
    IMS_RESULT SendBye(IN AString strInvitedUri = AString::ConstEmpty()) override;
    IMS_UINT32 GetType() const override;
    IMS_UINT32 GetResponseCode() const override;
    inline void SetForceToTerminateInterface(IN IMS_BOOL bTerminate) override
    {
        m_bForceToTerminateInterface = bTerminate;
    }

private:
    IMS_RESULT SendInviteForSingleUser(OUT AString& strReferToUri, IN CallKey n1To1Key);
    IMS_RESULT SendInviteForMultipleUser(OUT AString& strReferToUri);
    void GetReferToUri(OUT AString& strUri, IN IMtcCall* pi1To1Call) const;
    void SetReplaces(IN IMtcCall* pi1To1Call);
    void SetReferredByHeader();
    void SetHeadersForReferTo(OUT AString& strHeadersForReferTo);
    IMtcCall* GetConferenceCall();
    IReference* GetIReference(IN const AString& strInvitedUri, IN const AString& strMethod);

private:
    static const IMS_CHAR METHOD_INVITE[];
    static const IMS_CHAR METHOD_BYE[];

    IMtcContext& m_objContext;
    CallKey m_nConfCallKey;
    IConferenceReferenceListener& m_objListener;
    IMS_UINT32 m_nType;
    ConfUser* m_pConfUser;
    ImsList<ConfUser*> m_objConfUsers;
    IReference* m_piReference;
    IMS_BOOL m_bForceToTerminateInterface;
};

#endif
