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
#include "ServiceSystemTime.h"

#include "SipHeaderUtils.h"

PUBLIC GLOBAL IMS_SINT32 SipHeaderUtils::GenerateRetryAfterSeconds(IN IMS_SINT32 nExtent /* = 0*/)
{
    IMS_SINT32 nMilliSeconds = IMS_SYS_GetTimeInMicroSeconds();

    if (nExtent <= 0)
    {
        return (nMilliSeconds % 10 + 1);
    }
    else
    {
        return (nMilliSeconds % nExtent + 1);
    }
}
