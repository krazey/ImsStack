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
#include "DeviceConfig.h"
#include "ServiceTrace.h"

#include "IEnablerLoader.h"

#include "NativeCommands.h"
#include "NativeCommandsHandler.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
void NativeCommandsHandler::OnCommand(
        IN IMS_SINT32 nCmd, IN IMS_SINT32 nSlotId, IN IMS_UINTP pnParam)
{
    IMS_TRACE_I("OnCommand: cmd=%d, slotId=%d, param=%" PFLS_x, nCmd, nSlotId, pnParam);

    switch (nCmd)
    {
        case NativeCommands::CMD_SET_DEVICE_CONFIG:
        {
            const __DeviceConfig* pConfig = reinterpret_cast<const __DeviceConfig*>(pnParam);

            if (pConfig != IMS_NULL)
            {
                DeviceConfig::SetConfig(*pConfig);
            }
            break;
        }
        case NativeCommands::CMD_START_ENABLER:
        {
            if (m_piEnablerLoader != IMS_NULL)
            {
                m_piEnablerLoader->StartEnabler(nSlotId);
            }
            break;
        }
        case NativeCommands::CMD_STOP_ENABLER:
        {
            if (m_piEnablerLoader != IMS_NULL)
            {
                m_piEnablerLoader->StopEnabler(nSlotId);
            }
            break;
        }
        default:
            // no-op
            break;
    }
}
