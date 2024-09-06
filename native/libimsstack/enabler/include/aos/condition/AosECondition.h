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
#ifndef AOS_E_CONDITION_H_
#define AOS_E_CONDITION_H_

#include "condition/AosCondition.h"

class AosECondition : public AosCondition
{
public:
    explicit AosECondition(IN IAosAppContext* piAppContext);
    virtual ~AosECondition();

    IMS_BOOL IsReady() final;

protected:
    inline void AddServiceAvailable() final{};
    inline void RemoveServiceAvailable() final{};

    inline void AddEventListener() final{};
    inline void RemoveEventListener() final{};

    void AddAosServiceListener() override;
    void RemoveAosServiceListener() override;

    // IAosBlockListener
    void Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam) override;

    // AosServicePhoneListener
    void ServicePhone_AosStart() override;
    inline void ServicePhone_LocationInfoChanged(IN LocationInfo /*eState*/) final{};
    inline void ServicePhone_PhoneNumberStateChanged(
            IN IMS_BOOL /*bIsRefresh*/, IN PhoneNumberState /*eState*/) final{};
    inline void ServicePhone_PlmnChanged() final{};
    inline void ServicePhone_PowerOff() final{};
};

#endif  // AOS_E_CONDITION_H_