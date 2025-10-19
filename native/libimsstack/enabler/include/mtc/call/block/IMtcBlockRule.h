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

#ifndef INTERFACE_MTC_BLOCK_RULE_H_
#define INTERFACE_MTC_BLOCK_RULE_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"

class IMtcBlockRuleCheckListener;

/*
 * Represents a rule to check if some operation is blocked or not.
 */
class IMtcBlockRule
{
public:
    struct Result
    {
    public:
        enum class Status
        {
            UNBLOCKED,
            BLOCKED,
            PENDING,
        };

        Result(IN Status _eStatus, IN const CallReasonInfo& _objReason) :
                eStatus(_eStatus),
                objReason(_objReason)
        {
        }

        explicit Result(IN Status _eStatus) :
                Result(_eStatus, CallReasonInfo(CODE_NONE))
        {
        }

        IMS_BOOL operator==(const Result& objRhs) const
        {
            if (this == &objRhs)
            {
                return IMS_TRUE;
            }

            return eStatus == objRhs.eStatus && objReason == objRhs.objReason;
        }

        Status eStatus;
        CallReasonInfo objReason;
    };

    virtual ~IMtcBlockRule() {}

    /**
     * Checks if an operation is blocked for this rule.
     * The checker could return `PENDING` status if the checking takes time. But it must notify
     * `IMtcBlockRuleCheckListener` later with the final result in this case,
     * Notifying listener happens only once per each checking.
     *
     * @param objListener Listener to be notified the result if it was in pending.
     * @return Result. `CallReasonInfo` is valid only  when the status is `BLOCKED`.
     */
    virtual Result Check(IN IMtcBlockRuleCheckListener& objListener) = 0;
};

/**
 * The result of `IMtcBlockRule::Check()` could be notified by this listener.
 */
class IMtcBlockRuleCheckListener
{
public:
    virtual ~IMtcBlockRuleCheckListener() {}

    /**
     * Notifies the block check result.
     *
     * @param objResult Result. `eStatus` is not `PENDING`.
     *                  `CallReasonInfo` is valid only when `eStatus` is `BLOCKED`.
     */
    virtual void OnBlockRuleChecked(IN IMtcBlockRule::Result objResult) = 0;
};

#endif
