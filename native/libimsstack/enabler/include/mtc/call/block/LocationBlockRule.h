/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef LOCATION_BLOCK_RULE_H_
#define LOCATION_BLOCK_RULE_H_

#include "IPhoneInfoLocation.h"
#include "ImsTypeDef.h"
#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;

class LocationBlockRule final : public IMtcBlockRule, public ILocationUpdateListener
{
public:
    explicit LocationBlockRule(IN IMtcCallContext& objContext);
    virtual ~LocationBlockRule();
    LocationBlockRule(IN const LocationBlockRule&) = delete;
    LocationBlockRule& operator=(IN const LocationBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

    void LocationUpdate_OnCompleted() override;

private:
    IMtcCallContext& m_objContext;
    MtcLocationRefresher& m_objLocationRefresher;
    IMtcBlockRuleCheckListener* m_pListener;

    IMS_BOOL IsLocationUpdateRequired() const;
    IMS_BOOL IsLocationUpdateCompleted() const;
};

#endif
