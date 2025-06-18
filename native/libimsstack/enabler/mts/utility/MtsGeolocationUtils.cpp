/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "CarrierConfig.h"
#include "IIpcan.h"
#include "ImsStrLib.h"
#include "ServiceTrace.h"
#include "utility/MtsGeolocationUtils.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsGeolocationUtils::MtsGeolocationUtils()
{
    IMS_TRACE_I("+MtsGeolocationUtils", 0, 0, 0);
}

PUBLIC
MtsGeolocationUtils::~MtsGeolocationUtils()
{
    IMS_TRACE_I("~MtsGeolocationUtils", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsGeolocationUtils::GetGeolocationPidfAllowedType(
        IN IMS_BOOL bWlan, IN IMS_BOOL bEmergencyNumber)
{
    if (bEmergencyNumber)
    {
        return bWlan ? CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI
                     : CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR;
    }
    else
    {
        return bWlan ? CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI
                     : CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR;
    }
}
