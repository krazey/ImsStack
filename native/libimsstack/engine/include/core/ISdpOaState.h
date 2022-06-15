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
#ifndef INTERFACE_SDP_OFFER_ANSWER_STATE_H_
#define INTERFACE_SDP_OFFER_ANSWER_STATE_H_

#include "offeranswer/SdpMediaParameter.h"
#include "offeranswer/SdpSessionParameter.h"

class ISdpOaState
{
public:
    virtual void AbortProposal() = 0;
    virtual IMS_SINT32 CreateProposalView() = 0;
    virtual IMS_SINT32 GetSessionCurrentView(OUT SdpSessionParameter*& pSessionParam) const = 0;
    virtual IMS_SINT32 GetSessionPeerView(OUT SdpSessionParameter*& pSessionParam) const = 0;
    virtual IMS_SINT32 GetSessionProposalView(OUT SdpSessionParameter*& pSessionParam) const = 0;
    virtual IMS_SINT32 CreateMediaParameter(OUT SdpMediaParameter*& pMediaParam) = 0;
    virtual IMS_SINT32 GetMediaCurrentView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const = 0;
    virtual IMS_SINT32 GetMediaPeerView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const = 0;
    virtual IMS_SINT32 GetMediaProposalView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const = 0;
    virtual void MarkRejectedOrRemoved(IN IMS_SINT32 nMid) = 0;
    virtual void RemoveMediaParameter(IN IMS_SINT32 nMid) = 0;

public:
    /// Result of method call
    enum
    {
        RESULT_ERROR = (-1),
        RESULT_SUCCESS,
        RESULT_ALREADY_EXIST,
        RESULT_NOT_FOUND,
        RESULT_INVALID_STATE,
    };
};

#endif
