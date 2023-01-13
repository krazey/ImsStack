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
#include "RcsRegistrationService.h"

#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC
RcsRegistrationService::RcsRegistrationService(
        IN const AString& strName, IN const IMS_SINT32 nSlotId) :
        ImsService(strName),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_I("+RcsRegistrationService : Service Name = %s\n Slot = %d", GetName().GetStr(),
            m_nSlotId, 0);
    IMS_TRACE_MEM(
            "SNC_MSG", "IM_M : RCSRegService = %" PFLS_u, sizeof(RcsRegistrationService), 0, 0);
}

PUBLIC
VIRTUAL RcsRegistrationService::~RcsRegistrationService()
{
    IMS_TRACE_D("RcsRegistrationService_F : RcsRegistrationService = %" PFLS_u,
            sizeof(RcsRegistrationService), 0, 0);
    IMS_TRACE_I("~RcsRegistrationService [%d]", m_nSlotId, 0, 0);
}
