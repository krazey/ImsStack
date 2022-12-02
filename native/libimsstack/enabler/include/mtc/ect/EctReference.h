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

#ifndef ECT_REFERENCE_H_
#define ECT_REFERENCE_H_

#include "IReferenceListener.h"
#include "call/IMtcCall.h"

class IReference;
class IMtcCall;
class IMtcContext;
class IEctReferenceListener;

class EctReference : public IReferenceListener
{
public:
    explicit EctReference(IN IMtcContext& objContext, IN CallKey nTransfereeKey,
            IN IEctReferenceListener& objListener);
    virtual ~EctReference();
    EctReference(IN const EctReference&) = delete;
    EctReference& operator=(IN const EctReference&) = delete;

public:
    // IReferenceListener interface implementation
    void ReferenceDelivered(IN IReference* piReference) override;
    void ReferenceDeliveryFailed(IN IReference* piReference) override;
    void ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify) override;
    void ReferenceTerminated(IN IReference* piReference) override;

    virtual IMS_RESULT SendInvite(IN CallKey nTransferTargetKey);
    virtual IMS_RESULT SendInvite(IN const AString& strTransferTarget);
    IMS_UINT32 GetResponseCode() const;

private:
    IMS_RESULT SendRefer(IN const AString& strReferToUri, IN IMtcCall* piTransferTargetCall);
    AString GetReferToUri(IN IMtcCall* piTransferTargetCall) const;
    AString GetReferToUri(IN const AString& strTransferTarget) const;
    void SetReplaces(IN IMtcCall* piTransferTargetCall);
    void SetReferredByHeader();
    void SetHeadersForReferTo(OUT AString& strHeadersForReferTo);

private:
    IMtcContext& m_objContext;
    CallKey m_nTransfereeKey;
    IEctReferenceListener& m_objListener;
    IReference* m_piReference;
};

#endif
