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

#ifndef MTC_DIALING_PLAN_H_
#define MTC_DIALING_PLAN_H_

#include "dialingplan/EmergencyNumberList.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "dialingplan/NormalDialingPlan.h"
#include "ImsIdentity.h"
#include "AString.h"
#include <memory>

class IMtcContext;
class ISubscriberInfo;
struct CallInfo;

using NumberFormat = NormalDialingPlan::NumberFormat;
using LocalNumberPolicy = NormalDialingPlan::LocalNumberPolicy;
using Scheme = NormalDialingPlan::Scheme;

class MtcDialingPlan final : public IMtcDialingPlan
{
public:
    explicit MtcDialingPlan(IN IMtcContext& objContext, IN ISubscriberInfo& objSubscriberInfo);
    virtual ~MtcDialingPlan();
    MtcDialingPlan(IN const MtcDialingPlan&) = delete;
    MtcDialingPlan& operator=(IN const MtcDialingPlan&) = delete;

public:
    AString GetToUri(IN const AString& strNumber, IN const CallInfo& objCallInfo,
            IN Scheme eScheme = Scheme::UNKNOWN) override;

    void OnCountrySpecificServiceUrnReceived(
            IN const AString& strNumber, IN const AString& strServiceUrn) override;

private:
    IMS_BOOL IsUriForm(IN const AString& strNumber) const;
    AString GetConferenceFactoryUri() const;
    AString GetMcc() const;
    AString GetMnc(IN IMS_UINT32 nLength) const;

    struct TemporaryServiceUrn final
    {
    public:
        TemporaryServiceUrn(IN AString strNumber_, IN AString strUrn_)
        {
            strNumber = strNumber_;
            strUrn = strUrn_;
        }
        TemporaryServiceUrn(IN const TemporaryServiceUrn&) = delete;
        TemporaryServiceUrn& operator=(IN const TemporaryServiceUrn&) = delete;

        inline const AString& GetNumber() const { return strNumber; }
        inline const AString& GetUrn() const { return strUrn; }

        AString strNumber;
        AString strUrn;
    };

    IMtcContext& m_objContext;
    // TODO: no requirement found... try to find the standard again and update the logic.
    std::unique_ptr<TemporaryServiceUrn> m_pTemporaryServiceUrn;
    ISubscriberInfo& m_objSubscriberInfo;
};

#endif
