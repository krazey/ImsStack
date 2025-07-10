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

#include "AString.h"
#include "ImsIdentity.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "dialingplan/NormalDialingPlan.h"

class IMtcCallContext;
class ISubscriberInfo;
class ImsIdentityProxy;
struct CallInfo;

using NumberFormat = NormalDialingPlan::NumberFormat;
using LocalNumberPolicy = NormalDialingPlan::LocalNumberPolicy;
using Scheme = NormalDialingPlan::Scheme;

class MtcDialingPlan : public IMtcDialingPlan
{
public:
    MtcDialingPlan();
    virtual ~MtcDialingPlan() override;
    MtcDialingPlan(IN const MtcDialingPlan&) = delete;
    MtcDialingPlan& operator=(IN const MtcDialingPlan&) = delete;

public:
    AString GetToUri(IN const AString& strNumber, IN IMtcCallContext& objContext,
            IN Scheme eScheme = Scheme::UNKNOWN) override;

private:
    static IMS_BOOL IsUriForm(IN const AString& strNumber);
    AString GetConferenceFactoryUri(IN IMtcCallContext& objContext) const;

protected:
    ImsIdentityProxy* m_pIdentityProxy;
};

#endif
