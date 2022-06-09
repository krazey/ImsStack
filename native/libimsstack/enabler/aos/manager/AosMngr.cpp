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
#include "ImsServiceConfig.h"

#include "interface/IAosBuilder.h"
#include "interface/IAosHandle.h"
#include "app/AosAppContext.h"
#include "manager/AosBuilder.h"
#include "manager/AosBuildDirector.h"
#include "provider/AosStaticConfig.h"
#include "provider/AosStaticProfile.h"

#include "manager/AosMngr.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

PUBLIC
AosMngr::AosMngr(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_objAppId(IMSList<AString>()),
        m_objAppContext(IMSMap<AString, IAosAppContext*>()),
        m_pBuildDirector(IMS_NULL)
{
    CreateStaticConfig();

    CreateAoS();

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosMngr = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosMngr), this);
}

PUBLIC VIRTUAL AosMngr::~AosMngr()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosMngr = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosMngr), this);

    DestroyAoS();

    DestroyStaticConfig();
}

PUBLIC
IAosHandle* AosMngr::GetAosHandle(IN const AString& strAppId, IN const AString& strSrvId)
{
    AosStaticProfile* pProfile = AosStaticConfig::GetInstance()->GetProfile(strAppId, strSrvId);
    if (pProfile == IMS_NULL)
        return IMS_NULL;

    AString& strId = pProfile->GetId();

    IAosAppContext* piContext = m_objAppContext.GetValue(strId);
    if (piContext == IMS_NULL)
        return IMS_NULL;

    IMS_TRACE_D("GetAosHandle ::  [%s/%s] >> PID[%s]", strAppId.GetStr(), strSrvId.GetStr(),
            strId.GetStr());

    return piContext->GetHandle(strSrvId);
}

PUBLIC
IMSList<IAosHandle*> AosMngr::GetAllAosHandles(
        IN const AString& strAppId, IN const AString& strSrvId)
{
    const IMSList<AosStaticProfile*> objProfiles = AosStaticConfig::GetInstance()->GetProfiles();
    IMSList<IAosHandle*> objHandles;

    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); i++)
    {
        const AString& strId = objProfiles.GetAt(i)->GetId();
        IAosAppContext* piContext = m_objAppContext.GetValue(strId);

        if (piContext == IMS_NULL)
            continue;

        IAosHandle* piHandle = piContext->GetHandle(strSrvId);

        if (piHandle == IMS_NULL)
            continue;

        IMS_TRACE_D("GetAllAosHandles :: [%s/%s] >> PID[%s]", strAppId.GetStr(), strSrvId.GetStr(),
                strId.GetStr());

        objHandles.Append(piHandle);
    }

    return objHandles;
}

PUBLIC
IMSList<IAosHandle*> AosMngr::GetAllAosHandles(IN const AString& strAppId)
{
    const IMSList<AosStaticProfile*> objProfiles = AosStaticConfig::GetInstance()->GetProfiles();
    IMSList<IAosHandle*> objHandles;

    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); i++)
    {
        AosStaticProfile* pProfile = objProfiles.GetAt(i);
        if (pProfile == IMS_NULL)
            continue;

        const IMSList<AosServiceProfile*> objServices = pProfile->GetServiceProfiles();

        for (IMS_UINT32 j = 0; j < objServices.GetSize(); j++)
        {
            AosServiceProfile* pService = objServices.GetAt(j);
            if (pService == IMS_NULL)
                continue;

            if (!pService->GetAppId().Equals(strAppId))
                continue;

            IAosAppContext* piAppContext = m_objAppContext.GetValue(pProfile->GetId());
            if (piAppContext == IMS_NULL)
                continue;

            IAosHandle* piHandle = piAppContext->GetHandle(pService->GetServiceId());
            if (piHandle == IMS_NULL)
                continue;

            objHandles.Append(piHandle);
        }
    }
    return objHandles;
}

PUBLIC
IAosAppContext* AosMngr::GetAosAppContext(IN const AString& strProfileId)
{
    IAosAppContext* piAppContext = m_objAppContext.GetValue(strProfileId);

    if (piAppContext == IMS_NULL)
    {
        IMS_TRACE_D("[SLOT%d] GetAosAppContext :: PID[%s] no app context", m_nSlotId,
                strProfileId.GetStr(), 0);
        return IMS_NULL;
    }

    return piAppContext;
}

PRIVATE
void AosMngr::CreateStaticConfig()
{
    AosStaticConfig* pConfig = AosStaticConfig::GetInstance();
    pConfig->Create();
    const IMSList<AosStaticProfile*>& objProfiles = pConfig->GetProfiles();

    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); i++)
    {
        AosStaticProfile* pProfile = objProfiles.GetAt(i);

        if (pProfile == IMS_NULL)
            continue;

        const AString& strId = pProfile->GetId();
        m_objAppId.Append(AString(strId));
    }

    AString strLog;
    for (IMS_UINT32 i = 0; i < m_objAppId.GetSize(); i++)
    {
        AString& strAoSAppId = m_objAppId.GetAt(i);
        strLog.Append("[");
        strLog.Append(strAoSAppId.GetStr());
        strLog.Append("]");
    }

    IMS_TRACE_I("[SLOT%d] CreateStaticConfig :: appId list (%s)", m_nSlotId, strLog.GetStr(), 0);
}

PRIVATE
void AosMngr::CreateAoS()
{
    IAosBuilder* piAosBuilder = AosBuilderFactory();
    m_pBuildDirector = new AosBuildDirector(piAosBuilder, m_nSlotId);

    m_pBuildDirector->ConstructProvider();

    const IMSList<AosStaticProfile*>& objProfiles = AosStaticConfig::GetInstance()->GetProfiles();
    AString strLog;
    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); i++)
    {
        AosStaticProfile* pProfile = objProfiles.GetAt(i);

        if (pProfile == IMS_NULL)
            continue;

        AString& strAosAppId = pProfile->GetId();
        m_objAppContext.Add(AString(strAosAppId), m_pBuildDirector->ConstructAos(pProfile));

        strLog.Append("[");
        strLog.Append(strAosAppId.GetStr());
        strLog.Append("]");
    }

    IMS_TRACE_I("[SLOT%d] CreateAoS :: (%s) creation is completed", m_nSlotId, strLog.GetStr(), 0);
    delete piAosBuilder;
}

PRIVATE
void AosMngr::DestroyStaticConfig()
{
    AosStaticConfig* pConfig = AosStaticConfig::GetInstance();
    pConfig->Destroy();
}

PRIVATE
void AosMngr::DestroyAoS()
{
    if (m_pBuildDirector != IMS_NULL)
    {
        m_pBuildDirector->DestructAos();
        m_pBuildDirector->DestructProvider();

        delete m_pBuildDirector;
        m_pBuildDirector = IMS_NULL;
    }
}

PRIVATE
IAosBuilder* AosMngr::AosBuilderFactory()
{
    return new AosBuilder();
}
