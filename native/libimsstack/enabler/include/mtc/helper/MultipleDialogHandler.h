/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef MULTIPLE_DIALOG_HANDLER_H_
#define MULTIPLE_DIALOG_HANDLER_H_

#include "ImsTypeDef.h"

class IMtcCallContext;
class IMtcSession;

class MultipleDialogHandler final
{
public:
    enum class Result
    {
        HANDLED = 0,
        NOT_HANDLED,
    };

    MultipleDialogHandler();
    virtual ~MultipleDialogHandler() = default;
    MultipleDialogHandler(IN const MultipleDialogHandler&) = delete;
    MultipleDialogHandler& operator=(IN const MultipleDialogHandler&) = delete;

    void OnStarted(IN IMtcCallContext& objContext, IN IMtcSession& objSession);
    void OnSessionForked(IN IMtcCallContext& objContext, IN IMtcSession* piOriginalMtcSession);
    Result OnDialogRequestFailed(IN IMtcCallContext& objContext, IN IMtcSession& objMtcSession);
    Result OnUnavailableDialogCreated(
            IN IMtcCallContext& objContext, IN IMtcSession& objMtcSession);
};

#endif
