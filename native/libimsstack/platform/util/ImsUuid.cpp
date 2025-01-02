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
#include "ImsUuid.h"
#include "ServiceUtil.h"

// UUID = time-low "-" time-mid "-" time-hi-and-version "-"
// clock-seq-hi-and-reserved clock-seq-low "-" node
// 00000000-0000-0000-0000-000000000000

PUBLIC GLOBAL AString ImsUuid::GetUuid(
        IN IMS_SINT32 nVersion, IN const AString& strName /* = AString::ConstNull()*/)
{
    if (nVersion != VERSION_1 && nVersion != VERSION_3 && nVersion != VERSION_4)
    {
        return AString::ConstNull();
    }

    if (nVersion == VERSION_3 && strName.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AString strUuid;
    UtilService::GetUtilService()->GetSystemUtil()->GetUuid(nVersion, strUuid, strName);
    return strUuid;
}
