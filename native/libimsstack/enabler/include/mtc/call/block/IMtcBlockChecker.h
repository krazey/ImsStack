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

#ifndef INTERFACE_MTC_BLOCK_CHECKER_H_
#define INTERFACE_MTC_BLOCK_CHECKER_H_

#include "call/block/IMtcBlockRule.h"

/*
 * This is the interface to check if some operation is blocked for some reason or not.
 * The result could be unblocked, blocked, or in a pending status.
 */
class IMtcBlockChecker
{
public:
    using Result = IMtcBlockRule::Result;

    virtual ~IMtcBlockChecker() {}

    /**
     * Checks if an operation is blocked for some reason.
     * The checker could notify `IMtcBlockCheckListener` the final result later if pending.
     * Notifying listener happens only once per each checking.
     *
     * @return Result. `CallReasonInfo` is valid only when the status is `BLOCKED`.
     */
    virtual Result Check() = 0;
};

/**
 * The result of `IMtcBlockChecker::Check()` could be notified by this listener.
 */
class IMtcBlockCheckListener
{
public:
    virtual ~IMtcBlockCheckListener() {}

    /**
     * Notifies the block check result.
     *
     * @param objResult Result. `eStatus` is not `PENDING`.
     *                  `CallReasonInfo` is valid only when `eStatus` is `BLOCKED`.
     */
    virtual void OnBlockChecked(IN IMtcBlockChecker::Result objResult) = 0;
};

#endif
