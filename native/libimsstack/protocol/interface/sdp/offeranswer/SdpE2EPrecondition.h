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
#ifndef SDP_E2E_PRECONDITION_H_
#define SDP_E2E_PRECONDITION_H_

#include "offeranswer/SdpPrecondition.h"

class SdpE2EPrecondition : public SdpPrecondition
{
public:
    explicit SdpE2EPrecondition(IN IMS_SINT32 nType = TYPE_QOS);
    SdpE2EPrecondition(IN const SdpE2EPrecondition& other);
    ~SdpE2EPrecondition() override;

public:
    SdpE2EPrecondition& operator=(IN const SdpE2EPrecondition& other);

public:
    // SdpPrecondition class
    IMS_BOOL AddStatus(IN IMS_SINT32 nStatus, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nStrength = STRENGTH_NOTUSED) override;
    inline IMS_BOOL IsPreconditionPresent() const override { return !m_objE2EDetails.IsEmpty(); }
    AString ToSdp(IN IMS_SINT32 nAttribute) const override;

    inline const ImsList<DetailInfo>& GetE2EDetails() const { return m_objE2EDetails; }

private:
    ImsList<DetailInfo> m_objE2EDetails;
};

#endif
