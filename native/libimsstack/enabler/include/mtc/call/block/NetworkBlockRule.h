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

#ifndef NETWORK_BLOCK_RULE_H_
#define NETWORK_BLOCK_RULE_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;
class INetworkWatcher;

class NetworkBlockRule final : public IMtcBlockRule
{
public:
    explicit NetworkBlockRule(
            IN IMtcCallContext& objContext, IN INetworkWatcher& objNetworkWatcher);
    virtual ~NetworkBlockRule();
    NetworkBlockRule(IN const NetworkBlockRule&) = delete;
    NetworkBlockRule& operator=(IN const NetworkBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    const IMtcService& m_objService;
    INetworkWatcher& m_objNetworkWatcher;
    IMS_BOOL m_bWifiTestMode;
};

#endif
