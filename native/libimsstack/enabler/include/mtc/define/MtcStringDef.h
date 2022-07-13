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

#ifndef MTC_STRING_DEF_H_
#define MTC_STRING_DEF_H_

#include "CallReasonInfo.h"
#include "MtcDef.h"
#include "ImsStrLib.h"

class MtcDefPs
{
public:
    inline static const IMS_CHAR* PS_BOOL(IN IMS_BOOL /*bValue*/) { return ""; }  //

    inline static const IMS_CHAR* PS_FR(IN CallReasonInfo /*bValue*/) { return ""; }  //
};

#ifndef PS_BOOL
#define PS_BOOL(A) MtcDefPs::PS_BOOL(A)
#endif

#ifndef PS_FR
#define PS_FR(A) MtcDefPs::PS_FR(A)
#endif

#endif
