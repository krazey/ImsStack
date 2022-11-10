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

#ifndef SESSION_TIMER_EXTENSION_H_
#define SESSION_TIMER_EXTENSION_H_

#include "ImsTypeDef.h"
#include "call/extension/MtcExtension.h"

/**
 * This class represents the session timer extension.
 * But, timer header is controlled by SIP engine so this class only handles
 * supportability of the feature.
 */
class SessionTimerExtension final : public MtcExtension
{
public:
    explicit SessionTimerExtension();
    explicit SessionTimerExtension(IN const SessionTimerExtension& objRhs);
    virtual ~SessionTimerExtension();
    SessionTimerExtension& operator=(IN const SessionTimerExtension&) = delete;

    IMtcExtension* Clone() const override;
};

#endif
