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

#ifndef CANCEL_HANDLER_H_
#define CANCEL_HANDLER_H_

#include "CallReasonInfo.h"
#include "IMSTypeDef.h"

class IMessage;

class CancelHandler final
{
public:
    explicit CancelHandler();
    ~CancelHandler();
    CancelHandler(const CancelHandler&) = delete;
    CancelHandler& operator=(const CancelHandler&) = delete;

    CallReasonInfo Handle(IN const IMessage& objMessage) const;

private:
    static const AString REASON_TEXT_CALL_BUSY;
    static const AString REASON_TEXT_CALL_COMPLETED;
    static const AString REASON_TEXT_CALL_DECLINED;

    CallReasonInfo GetCallReasonInfoFromReasonHeader(
            IN IMS_SINT32 nCause, IN const AString& strText) const;
};

#endif
