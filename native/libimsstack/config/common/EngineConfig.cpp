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
#include "IImsPrivateProperty.h"
#include "ISystemProperty.h"
#include "ITraceOption.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "private/EngineConfig.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC
EngineConfig::EngineConfig(IN IMS_SINT32 nSlotId) :
        ConfigBase(nSlotId),
        m_nTraceOption(ITraceOption::OPT_DEFAULT),
        m_nTraceModule(IMS_TRACE_MODULE_ALL)
{
    if (IMS_UTIL_SYS_PROP_IS_USER_MODE())
    {
        m_nTraceOption = ITraceOption::RELEASE_OPT_DEFAULT;
    }
}

PUBLIC VIRTUAL EngineConfig::~EngineConfig() {}

PUBLIC VIRTUAL void EngineConfig::Refresh()
{
    ReadFrom();
    TraceService::GetTraceService()->SetOption(GetTraceOption(), GetTraceModule());
}

PROTECTED VIRTUAL IMS_BOOL EngineConfig::ReadFrom()
{
    IImsPrivateProperty* piProperty = GetPrivateProperty();
    AString strLogOptions = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_TEST_LOG_OPTIONS, IMS_SLOT_0);

    if (strLogOptions.GetLength() > 0)
    {
        IMS_BOOL bOk = IMS_FALSE;
        IMS_UINT32 nLogOptions = strLogOptions.ToInt32(&bOk, 16);

        if (bOk)
        {
            IMS_TRACE_I("TraceOption: %08X >> %08X", m_nTraceOption, nLogOptions, 0);
            m_nTraceOption = nLogOptions;
        }
    }

    return IMS_TRUE;
}
