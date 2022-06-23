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

#ifndef MTS_ASSET_ITEMS_H_
#define MTS_ASSET_ITEMS_H_

#include "AString.h"
#include "IMSTypeDef.h"
#include "ImsVector.h"

struct MtsAssetItems
{
public:
    MtsAssetItems() :
            nSmsRequestUriType(-1),
            bUseDialedNumber(IMS_FALSE)
    {
    }

    MtsAssetItems(IN const MtsAssetItems&) = delete;
    MtsAssetItems& operator=(IN const MtsAssetItems&) = delete;

public:
    // sms asset
    IMS_SINT32 nSmsRequestUriType;
    IMS_BOOL bUseDialedNumber;
};

#endif
