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
#ifndef IMS_ISIM_H_
#define IMS_ISIM_H_

#include "IIsim.h"
#include "ImsSlot.h"

class ImsIsim : public ImsSlot, public IIsim
{
public:
    inline ImsIsim(IN IMS_SINT32 nSlotId) :
            ImsSlot(nSlotId)
    {
    }

protected:
    inline virtual ~ImsIsim() {}

public:
    inline virtual void Destroy() { delete this; }

    virtual void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;
};

#endif
