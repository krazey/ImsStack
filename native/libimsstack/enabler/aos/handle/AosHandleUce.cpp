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

#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "handle/AosHandleUce.h"

//__IMS_TRACE_TAG_USER_DECL__("AOS");

/*

Remarks

*/
PUBLIC
AosHandleUce::AosHandleUce
    (
        IN IAosAppContext* piAppContext,
        IN const AString& strAppId,
        IN const AString& strServiceId,
        IN const IMS_SINT32 nServiceType
    )
    : AosHandle(piAppContext, strAppId, strServiceId, nServiceType)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleUce = %" PFLS_u "/%" PFLS_x,
            strAppId.GetStr(), sizeof(AosHandleUce), this);

    /* @jryou: temp blocked. Defined in VZWAoSHandle.
    AddConfig(CONFIG_LIMITED_MODE | CONFIG_SRLTE_MODE);
    */
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosHandleUce::~AosHandleUce()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleUce = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleUce), this);
}