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
#ifndef IMS_USIM_H_
#define IMS_USIM_H_

#include "ImsSlot.h"
#include "IUsim.h"

class ImsUsim
    : public ImsSlot
    , public IUsim
{
public:
    inline ImsUsim(IN IMS_SINT32 nSlotId)
        : ImsSlot(nSlotId)
    {}
    inline virtual ~ImsUsim()
    {}

public:
    virtual void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;
};

#endif
