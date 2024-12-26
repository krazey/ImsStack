/*
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

#include "CallReasonInfo.h"
#include "ISession.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "helper/MultipleDialogHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MultipleDialogHandler::MultipleDialogHandler() {}

PUBLIC
void MultipleDialogHandler::OnCallConnected(
        IN IMtcCallContext& objContext, IN IMtcSession& objMtcSession)
{
    ImsList<IMtcSession*> objSessions = objContext.GetSessions();
    IMS_TRACE_D("OnCallConnected size[%d]", objSessions.GetSize(), 0, 0);

    for (IMS_SINT32 i = static_cast<IMS_SINT32>(objSessions.GetSize()) - 1; i >= 0; i--)
    {
        IMtcSession* pSession = objSessions.GetAt(i);
        if (&objMtcSession == pSession)
        {
            continue;
        }
        IMS_TRACE_D("OnCallConnected remove[%d]", i, 0, 0);
        objContext.RemoveSession(*pSession);
    }
}
