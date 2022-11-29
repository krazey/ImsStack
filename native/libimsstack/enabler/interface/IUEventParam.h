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
#ifndef IU_EVENT_PARAM_H_
#define IU_EVENT_PARAM_H_

#include "ImsTypeDef.h"

class IUEventParam
{
public:
    IMS_SINT32 nAppId;
    IMS_SINT32 nEventName;

public:
    inline explicit IUEventParam(IUEventParam* pParam = IMS_NULL) :
            nAppId(-1),
            nEventName(-1)
    {
        if (pParam == IMS_NULL)
        {
            return;
        }

        this->nAppId = pParam->nAppId;
        this->nEventName = pParam->nEventName;
    }

    virtual ~IUEventParam() {}
};

#endif
