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
#include "call/IMtcCall.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcSipInterfaceFactory::MtcSipInterfaceFactory() :
        m_objSessionHolder(),
        m_piReferenceHolder(IMS_NULL),
        m_piSubscriptionHolder(IMS_NULL)
{
    IMS_TRACE_D("+MtcSipInterfaceFactory", 0, 0, 0);
}

PUBLIC
MtcSipInterfaceFactory::~MtcSipInterfaceFactory()
{
    IMS_TRACE_D("~MtcSipInterfaceFactory", 0, 0, 0);

    delete m_piReferenceHolder;
    delete m_piSubscriptionHolder;
}

PUBLIC VIRTUAL void MtcSipInterfaceFactory::OnReferenceInterfaceCleared()
{
    delete m_piReferenceHolder;
    m_piReferenceHolder = IMS_NULL;
}

PUBLIC VIRTUAL void MtcSipInterfaceFactory::OnSubscriptionInterfaceCleared()
{
    delete m_piSubscriptionHolder;
    m_piSubscriptionHolder = IMS_NULL;
}

PUBLIC
SessionInterfaceHolder& MtcSipInterfaceFactory::GetISessionHolder()
{
    return m_objSessionHolder;
}

PUBLIC
ReferenceInterfaceHolder* MtcSipInterfaceFactory::GetIReferenceHolder()
{
    if (m_piReferenceHolder == IMS_NULL)
    {
        m_piReferenceHolder = new ReferenceInterfaceHolder(*this);
    }
    return m_piReferenceHolder;
}

PUBLIC
SubscriptionInterfaceHolder* MtcSipInterfaceFactory::GetISubscriptionHolder()
{
    if (m_piSubscriptionHolder == IMS_NULL)
    {
        m_piSubscriptionHolder = new SubscriptionInterfaceHolder(*this);
    }
    return m_piSubscriptionHolder;
}
