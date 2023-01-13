/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "RcsConfigurationService.h"

#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC
RcsConfigurationService::RcsConfigurationService(
        IN const AString& strName, IN const IMS_SINT32 nSlotId) :
        ImsService(strName),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_I("+RcsConfigurationService : Service Name = %s\n Slot = %d", GetName().GetStr(),
            m_nSlotId, 0);
    IMS_TRACE_MEM(
            "SNC_MSG", "IM_M : RCSConfService = %" PFLS_u, sizeof(RcsConfigurationService), 0, 0);
}

PUBLIC
VIRTUAL RcsConfigurationService::~RcsConfigurationService()
{
    IMS_TRACE_D("RcsConfigurationService_F : RcsConfigurationService = %" PFLS_u,
            sizeof(RcsConfigurationService), 0, 0);
    IMS_TRACE_I("~RcsConfigurationService [%d]", m_nSlotId, 0, 0);
}
