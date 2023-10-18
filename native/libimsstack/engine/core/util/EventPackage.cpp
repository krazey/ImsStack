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
#include "ServiceMemory.h"

#include "ISipHeader.h"
#include "util/EventPackage.h"

PUBLIC
EventPackage::EventPackage() :
        m_strEvent(AString::ConstNull()),
        m_piEventHeader(IMS_NULL),
        m_nInitialDuration(-1)
{
}

PUBLIC VIRTUAL EventPackage::~EventPackage()
{
    if (m_piEventHeader != IMS_NULL)
    {
        m_piEventHeader->Destroy();
    }
}

PUBLIC
void EventPackage::SetEventHeader(IN ISipHeader* piHeader)
{
    if (m_piEventHeader != IMS_NULL)
    {
        m_piEventHeader->Destroy();
    }

    m_piEventHeader = piHeader;
}
