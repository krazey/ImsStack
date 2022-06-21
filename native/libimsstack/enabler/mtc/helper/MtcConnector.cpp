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

#include "ServiceTrace.h"

#include "helper/CallStateProxy.h"
#include "IMtcCallStateListener.h"
#include "IMtcContext.h"
#include "MtcConnector.h"
#include "MtcContextRepository.h"

PUBLIC GLOBAL void MtcConnector::AddCallStateListener(
        IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener)
{
    //----------------------------------------------------------------------------------------------

    IMtcContext* piContext = MtcContextRepository::GetContext(nSlotId);
    if (piContext)
    {
        piContext->GetCallStateProxy().AddListener(pListener);
    }
}

PUBLIC GLOBAL void MtcConnector::RemoveCallStateListener(
        IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener)
{
    //----------------------------------------------------------------------------------------------

    IMtcContext* piContext = MtcContextRepository::GetContext(nSlotId);
    if (piContext)
    {
        piContext->GetCallStateProxy().RemoveListener(pListener);
    }
}
