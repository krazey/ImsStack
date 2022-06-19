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
#ifndef PROTOCOL_PERMISSION_H_
#define PROTOCOL_PERMISSION_H_

#include "AString.h"

class Protocol;

class ProtocolPermission
{
private:
    inline ProtocolPermission() {}
    inline ~ProtocolPermission() {}

public:
    ProtocolPermission(IN const ProtocolPermission&) = delete;
    ProtocolPermission& operator=(IN const ProtocolPermission&) = delete;

public:
    Protocol* Lookup(IN const AString& strName);

    static ProtocolPermission* GetInstance();
};

#endif
