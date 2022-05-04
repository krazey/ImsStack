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
#ifndef INTERFACE_AOS_RETRYREPOSITORY_H_
#define INTERFACE_AOS_RETRYREPOSITORY_H_

class IAosRetryRepository
{
public:
    /**
     * @brief increase retry count
     *
     * Retry count can be increased by max count
     * (max count: CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_MAX_COUNT_INT)
     * If retry count reaches out max count, return IMS_FALSE and reset the retry count.
     *
     * @return IMS_BOOL Return whether increasing retry count or not
     */
    virtual IMS_BOOL IncreaseRetryCount(IN IMS_UINT32 nType) = 0;

    /**
     * @brief reset retry count
     *
     * Reset retry count below situation.
     * - the UE get new P-CSCFs
     * - If the subscription to reg event package is required, when it receives the NOTIFY message
     *   against SUBSCRIBE message
     * - If the subscription to reg event package is NOT required, when it receives the 200 OK
     *   message against REGISTER message
     *
     * @return
     */
    virtual void ResetRetryCount(IN IMS_UINT32 nType) = 0;
};
#endif