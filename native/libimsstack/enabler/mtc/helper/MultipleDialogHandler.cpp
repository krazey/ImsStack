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
#include "CarrierConfig.h"
#include "ISession.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MultipleDialogHandler.h"
#include "media/IMtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MultipleDialogHandler::MultipleDialogHandler() {}

PUBLIC
void MultipleDialogHandler::OnStarted(IN IMtcCallContext& objContext, IN IMtcSession& objMtcSession)
{
    const ImsList<IMtcSession*>& objSessions = objContext.GetSessions();
    IMS_TRACE_D("OnStarted size[%d]", objSessions.GetSize(), 0, 0);

    for (IMS_SINT32 i = static_cast<IMS_SINT32>(objSessions.GetSize()) - 1; i >= 0; i--)
    {
        IMtcSession* piMtcSession = objSessions.GetAt(i);
        if (&objMtcSession == piMtcSession)
        {
            continue;
        }
        IMS_TRACE_D("OnStarted remove[%d]", i, 0, 0);
        objContext.RemoveSession(*piMtcSession);
    }
}

PUBLIC
void MultipleDialogHandler::OnSessionForked(
        IN IMtcCallContext& objContext, IN IMtcSession* piOriginalMtcSession)
{
    if (objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL))
    {
        return;
    }

    if (piOriginalMtcSession == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("OnSessionForked : Terminate the original session.", 0, 0, 0);

    piOriginalMtcSession->Terminate(
            IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED));

    objContext.GetMediaManager().DestroyMediaProfile(&piOriginalMtcSession->GetISession());
    objContext.RemoveSession(*piOriginalMtcSession);
}

PUBLIC
MultipleDialogHandler::Result MultipleDialogHandler::OnDialogRequestFailed(
        IN IMtcCallContext& objContext, IN IMtcSession& objMtcSession)
{
    const ImsList<IMtcSession*>& objSessions = objContext.GetSessions();
    if (objSessions.GetSize() > 1)
    {
        IMS_TRACE_I("OnDialogRequestFailed : remove failed session.", 0, 0, 0);
        objMtcSession.Terminate(IMS_TRUE, CallReasonInfo(CODE_SIP_SERVER_ERROR));
        objContext.RemoveSession(objMtcSession);
        return Result::HANDLED;
    }

    return Result::NOT_HANDLED;
}

PUBLIC
MultipleDialogHandler::Result MultipleDialogHandler::OnUnavailableDialogCreated(
        IN IMtcCallContext& objContext, IN IMtcSession& objMtcSession)
{
    const ImsList<IMtcSession*>& objSessions = objContext.GetSessions();
    if (objSessions.GetSize() > 1)
    {
        IMS_TRACE_I("OnUnavailableDialogCreated : remove failed session.", 0, 0, 0);
        objMtcSession.Terminate(IMS_TRUE, CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE));
        objContext.RemoveSession(objMtcSession);
        return Result::HANDLED;
    }

    return Result::NOT_HANDLED;
}
