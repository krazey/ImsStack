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
#include "AString.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "CoreInterfaceFactory.h"
#include "IUIMS.h"
#include "JniAosService.h"
#include "JniMtcCall.h"
#include "JniMtcService.h"
#include "JniMtsService.h"
#include "JniUceService.h"
#include "JniSipControllerService.h"

__IMS_TRACE_TAG_USER_DECL__("CoreInterfaceFactory");

PUBLIC GLOBAL BaseService* CoreInterfaceFactory::GetInterface(IN IMS_SINT32 nInterfaceType,
        IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId)
{
    BaseService* pService = IMS_NULL;

    IMS_TRACE_D("GetInterface - interface=%d, slotId=%d", nInterfaceType, nSlotId, 0);

    switch (nInterfaceType)
    {
        case IUIMS::APP_MTC:
            pService = new JniMtcService(pfnSendDataToJava, nSlotId);
            break;

        case IUIMS::MTC_CALL:
            pService = new JniMtcCall(pfnSendDataToJava, -1, nSlotId);
            break;

        case IUIMS::APP_UCE:
            pService = new JniUceService(pfnSendDataToJava, nSlotId);
            break;

        case IUIMS::APP_MTS:
            pService = new JniMtsService(pfnSendDataToJava, nSlotId);
            break;

        case IUIMS::APP_SIP_DELEGATE:
            pService = new JniSipControllerService(pfnSendDataToJava, nSlotId);
            break;

        case IUIMS::MTS_EMERGENCY_SERVICE:
            // TODO: If need, a JniService for E911 SMS will be added
            break;

        // IMS_SERVICE_AOS
        case IUIMS::AOS_SERVICE:
            pService = new JniAosService(pfnSendDataToJava, nSlotId);
            break;

        default:
            IMS_TRACE_D("Invalid interface type (%d)", nInterfaceType, 0, 0);
            return IMS_NULL;
    }

    return pService;
}
