/**
 * Copyright (C) 2024 The Android Open Source Project
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

#include "ServiceTrace.h"
#include "BaseNego.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC BaseNego::BaseNego(IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_pEnvironment(IMS_NULL)
{
    IMS_TRACE_I("+BaseNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC VIRTUAL BaseNego::~BaseNego()
{
    IMS_TRACE_I("~BaseNego()", 0, 0, 0);
}
