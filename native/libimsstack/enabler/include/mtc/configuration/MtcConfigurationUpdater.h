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

#ifndef MTC_CONFIGURATION_UPDATER_H_
#define MTC_CONFIGURATION_UPDATER_H_

#include "IMSTypeDef.h"
#include "IMSMap.h"

class ICarrierConfig;
struct CarrierConfigItems;
struct AssetItems;

class MtcConfigurationUpdater final
{
public:
    static void Update(IN ICarrierConfig* piCc, IN CarrierConfigItems& objCarrierConfigItems,
            IN AssetItems& objAssetItems);

private:
    static void UpdateByCarrierConfig(IN ICarrierConfig* piCc, IN CarrierConfigItems& objItems);
    static void UpdateByAsset(IN ICarrierConfig* piCc, IN AssetItems& objItems);
};

#endif
