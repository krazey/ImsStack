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
#include "AString.h"
#include "IMtcContext.h"
#include "dialingplan/EmergencyDialingPlan.h"
#include "dialingplan/EmergencyNumberList.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL AString& EmergencyDialingPlan::GetTranslatedUri(
        IN IMtcContext& objContext, IN_OUT AString& strNumber)
{
    if (IsTestNumber(strNumber))
    {
        // TODO: vzw
        return strNumber;
    }

    EmergencyNumberList* pEnl = new EmergencyNumberList(objContext.GetSlotId());
    strNumber = pEnl->GetEmergencyServiceURN(strNumber);  // TODO:
    delete pEnl;

    return strNumber;
}

PRIVATE GLOBAL IMS_BOOL EmergencyDialingPlan::IsTestNumber(IN const AString& /*strNumber*/)
{
    // TODO: vzw.
    return IMS_FALSE;
}
