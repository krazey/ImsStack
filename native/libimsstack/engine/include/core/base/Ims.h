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
#ifndef IMS_H_
#define IMS_H_

#include "base/ImsError.h"

class Ims
{
public:
    static void Init();
    static void Init(IN IMS_SINT32 nSlotId);

    static void SetLastError(IN IMS_SINT32 nErrorCode);
    static IMS_SINT32 GetLastError();

    static void SetLastError(IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSlotId);
    static IMS_SINT32 GetLastError(IN IMS_SINT32 nSlotId);

private:
    static IMS_SINT32* s_pnErrorCode;  // 0 means there is no error
};

#endif
