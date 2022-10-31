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
#include "call/IMtcCallContext.h"
#include "call/IMtcUiNotifier.h"
#include "ussi/UssiDef.h"
#include "ussi/UssiEventNotifier.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UssiEventNotifier::UssiEventNotifier(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
    IMS_TRACE_I("+UssiEventNotifier", 0, 0, 0);
}

PUBLIC VIRTUAL UssiEventNotifier::~UssiEventNotifier()
{
    IMS_TRACE_I("~UssiEventNotifier", 0, 0, 0);
}

PUBLIC
void UssiEventNotifier::NotifyUssiError(IN AString strUssdString)
{
    IMS_TRACE_D("NotifyUssiError", 0, 0, 0);
    m_objContext.GetUiNotifier().SendNotifyInfo(
            INFO_TYPE_USSI, strUssdString, (IMS_SINT32)UssiModeType::ERROR);
}

PUBLIC
void UssiEventNotifier::NotifyUssiResult(IN AString strUssdString, IN UssiModeType eType)
{
    IMS_TRACE_D("NotifyUssiResult", 0, 0, 0);
    m_objContext.GetUiNotifier().SendNotifyInfo(INFO_TYPE_USSI, strUssdString, (IMS_SINT32)eType);
}
