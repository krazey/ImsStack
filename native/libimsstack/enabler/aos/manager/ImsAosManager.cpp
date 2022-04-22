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
#include "ServiceMSG.h"
#include "IImsAos.h"
#include "manager/ImsAosManager.h"

PUBLIC
ImsAosManager::ImsAosManager(IN const AString& strAppName)
    : IMSApp(strAppName)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] ImsAosManager = %" PFLS_u "/%" PFLS_x,
            GetName().GetStr(), sizeof(ImsAosManager), this);
}

PUBLIC VIRTUAL
ImsAosManager::~ImsAosManager()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] ImsAosManager = %" PFLS_u "/%" PFLS_x,
        GetName().GetStr(), sizeof(ImsAosManager), this);
}

PUBLIC
IImsAos* ImsAosManager::GetImsAos(IN const AString& /*strAppId*/,
        IN const AString& /*strServiceId*/)
{
    return IMS_NULL;
}

PUBLIC
IMSList<IImsAos*> ImsAosManager::GetImsAosList(IN const AString& /*strAppId*/,
        IN const AString& /*strServiceId*/)
{
   return IMSList<IImsAos*>();
}

PUBLIC
IMSList<IImsAos*> ImsAosManager::GetImsAosList (IN const AString& /*strAppId*/)
{
   return IMSList<IImsAos*>();
}

PUBLIC VIRTUAL
IMS_BOOL ImsAosManager::OnPreprocess(IN IMSMSG& /*objMSG*/)
{
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL ImsAosManager::OnMessage(IN IMSMSG& /*objMSG*/)
{
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL ImsAosManager::OnPostprocess(IN IMSMSG& /*objMSG*/)
{
    return IMS_TRUE;
}
