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

#ifndef TRANSACTION_TIMER_UPDATE_HELPER_H
#define TRANSACTION_TIMER_UPDATE_HELPER_H

#include "IMSTypeDef.h"

class MtcConfigurationProxy;
class IMtcSessionContext;

class TransactionTimerUpdateHelper
{
public:
    explicit TransactionTimerUpdateHelper(IN IMtcSessionContext& objContext);
    virtual ~TransactionTimerUpdateHelper();
    TransactionTimerUpdateHelper(IN const TransactionTimerUpdateHelper&) = delete;
    TransactionTimerUpdateHelper& operator=(IN const TransactionTimerUpdateHelper&) = delete;

    virtual void SetInviteTransactionTimer();
    virtual void ResetInviteTransactionTimer();
    virtual void SetNonInviteTransactionTimer();
    virtual void ResetNonInviteTransactionTimer();

private:
    void UpdateTimer(IN IMS_BOOL bInviteTransaction, IN IMS_SINT32 nValue);
    IMS_BOOL IsNeedToUpdate() const;

    IMS_SINT32 m_nSlotId;
    MtcConfigurationProxy& m_objConfiguration;
    IMS_BOOL m_bWifi;
};

#endif
