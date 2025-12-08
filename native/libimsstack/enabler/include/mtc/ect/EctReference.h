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

class AString;
class IReference;
class IMtcCall;
class IMtcContext;
class IEctReferenceListener;

/**
 * @brief Manages the SIP REFER transaction for Explicit Call Transfer (ECT).
 *
 * This class is responsible for creating and sending the SIP REFER message to initiate
 * a call transfer. It listens for events related to the REFER transaction via the
 * IReferenceListener interface and forwards them to an IEctReferenceListener.
 */
class EctReference : public IReferenceListener
{
public:
    /**
     * @brief Constructs a new Ect Reference object.
     *
     * @param objContext The MTC context.
     * @param nTransfereeKey The key of the call being transferred.
     * @param objListener The listener for ECT reference events.
     */
    explicit EctReference(IN IMtcContext& objContext, IN CallKey nTransfereeKey,
            IN IEctReferenceListener& objListener);
    virtual ~EctReference() override;
    EctReference(IN const EctReference&) = delete;
    EctReference& operator=(IN const EctReference&) = delete;

public:
    /** See {@link IReferenceListener#ReferenceDelivered}. */
    void ReferenceDelivered(IN IReference* piReference) override;

    /** See {@link IReferenceListener#ReferenceDeliveryFailed}. */
    void ReferenceDeliveryFailed(IN IReference* piReference) override;

    /** See {@link IReferenceListener#ReferenceNotify}. */
    void ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify) override;

    /** See {@link IReferenceListener#ReferenceTerminated}. */
    void ReferenceTerminated(IN IReference* piReference) override;

    /**
     * @brief Sends a REFER request to transfer the call to an existing call (consultative
     * transfer).
     *
     * @param nTransferTargetKey The key of the call to transfer to.
     * @return IMS_RESULT Returns IMS_SUCCESS if the request is sent, otherwise IMS_FAILURE.
     */
    virtual IMS_RESULT SendInvite(IN CallKey nTransferTargetKey);

    /**
     * @brief Sends a REFER request to transfer the call to a new number (blind transfer).
     *
     * @param strTransferTarget The target number to transfer to.
     * @return IMS_RESULT Returns IMS_SUCCESS if the request is sent, otherwise IMS_FAILURE.
     */
    virtual IMS_RESULT SendInvite(IN const AString& strTransferTarget);

    /**
     * @brief Gets the response code from the REFER transaction.
     *
     * @return IMS_UINT32 The SIP response code.
     */
    IMS_UINT32 GetResponseCode() const;

private:
    IMS_RESULT SendRefer(IN const AString& strReferToUri, IN IMtcCall* piTransferTargetCall);
    static AString GetReferToUri(IN IMtcCall* piTransferTargetCall);
    AString GetReferToUri(IN const AString& strTransferTarget) const;
    void SetReplaces(IN IMtcCall* piTransferTargetCall);
    void SetReferredByHeader();

private:
    IMtcContext& m_objContext;
    CallKey m_nTransfereeKey;
    IEctReferenceListener& m_objListener;
    IReference* m_piReference;
};

#endif
