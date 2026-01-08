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

#ifndef MTS_TRANSACTION_TIMER_UPDATE_HELPER_H_
#define MTS_TRANSACTION_TIMER_UPDATE_HELPER_H_

#include "ImsTypeDef.h"

class ICarrierConfig;
class ISipConfig;

class MtsTransactionTimerUpdateHelper final
{
public:
    explicit MtsTransactionTimerUpdateHelper(
            IN const ICarrierConfig* piCarrierConfig, IN const ISipConfig* piSipConfig);
    ~MtsTransactionTimerUpdateHelper();
    MtsTransactionTimerUpdateHelper(IN const MtsTransactionTimerUpdateHelper&) = delete;
    MtsTransactionTimerUpdateHelper& operator=(IN const MtsTransactionTimerUpdateHelper&) = delete;

    void SetMessageTransactionTimer(IN const IMS_SINT32 nMti) const;
    void ResetMessageTransactionTimer(IN const IMS_SINT32 nMti) const;

private:
    void UpdateTimer(IN const IMS_SINT32 nValue) const;
    IMS_BOOL IsNeedToUpdate(IN const IMS_SINT32 nMti) const;

    const ICarrierConfig* m_piCarrierConfig;
    const ISipConfig* m_piSipConfig;
};

#endif
