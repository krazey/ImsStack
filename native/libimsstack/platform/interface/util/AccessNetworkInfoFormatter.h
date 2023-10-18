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
#ifndef ACCESS_NETWORK_INFO_FORMATTER_H_
#define ACCESS_NETWORK_INFO_FORMATTER_H_

#include "AString.h"
#include "ImsAccessNetworkInfoType.h"

class AccessNetworkInfoFormatter
{
public:
    AccessNetworkInfoFormatter() = delete;

public:
    static IMS_BOOL Encode(IN const AccessNetworkInfo& objAni, OUT AString& strHeader,
            IN const AString& strCellInfo = AString::ConstNull());
};

#endif
