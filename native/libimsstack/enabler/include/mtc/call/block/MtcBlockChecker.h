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

#ifndef MTC_BLOCK_CHECKER_H_
#define MTC_BLOCK_CHECKER_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/IMtcBlockRule.h"

/**
 * This class checks if a call operation is blocked or not for the given ruleset.
 * Also it provides rulesets for each call operation.
 */
class MtcBlockChecker final : public IMtcBlockChecker, public IMtcBlockRuleCheckListener
{
public:
    /**
     * Constructs a new MtcBlockChecker object.
     *
     * @param lstRules List of the rules to check. The elements are managed by this object.
     * @param pListener Listener to be notified the result if pending.
     */
    MtcBlockChecker(
            IN const ImsList<IMtcBlockRule*>& lstRules, IN IMtcBlockCheckListener* pListener);

    ~MtcBlockChecker() override;

    MtcBlockChecker(IN const MtcBlockChecker&) = delete;
    MtcBlockChecker& operator=(IN const MtcBlockChecker&) = delete;

    Result Check() override;

    void OnBlockRuleChecked(IN IMtcBlockRule::Result objResult) override;

private:
    IMS_BOOL IsResultNotified() const;

    IMtcBlockCheckListener* m_pListener;
    ImsList<IMtcBlockRule*> m_lstRules;

    IMS_SINT32 m_nPendingCount;
};

#endif
