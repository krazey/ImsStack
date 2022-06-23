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

#ifndef MTS_CARRIER_CONFIG_ITEMS_H_
#define MTS_CARRIER_CONFIG_ITEMS_H_

#include "IMSTypeDef.h"
#include "ImsVector.h"

struct MtsCarrierConfigItems
{
public:
    MtsCarrierConfigItems() :
            nRequestUriType(0),
            bSmsCsfbRetryOnFailure(IMS_FALSE),
            nSmsOverImsFormat(0)
    {
    }

    MtsCarrierConfigItems(IN const MtsCarrierConfigItems&) = delete;
    MtsCarrierConfigItems& operator=(IN const MtsCarrierConfigItems&) = delete;

public:
    // ims configurations
    IMS_SINT32 nRequestUriType;
    // sms configurations
    IMS_BOOL bSmsCsfbRetryOnFailure;
    IMS_SINT32 nSmsOverImsFormat;
};

#endif
