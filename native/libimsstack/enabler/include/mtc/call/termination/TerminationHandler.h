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

#ifndef TERMINATION_HANDLER_H_
#define TERMINATION_HANDLER_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"

class IMtcCallContext;
class ISession;

class TerminationHandler final
{
public:
    explicit TerminationHandler(IN IMtcCallContext& objContext);
    ~TerminationHandler();
    TerminationHandler(const TerminationHandler&) = delete;
    TerminationHandler& operator=(const TerminationHandler&) = delete;

    CallReasonInfo Handle(IN const ISession& objSession) const;

private:
    CallReasonInfo GetCallReasonInfoFromSessionTerminationReason(
            IN IMS_SINT32 nTerminationReason) const;

    IMtcCallContext& m_objContext;
};

#endif
