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
#ifndef AOS_MNGR_H_
#define AOS_MNGR_H_

#include "ImsList.h"
#include "ImsMap.h"
#include "AString.h"

class AosBuildDirector;
class AosStaticConfig;
class IAosHandle;
class IAosBuilder;
class IAosAppContext;

class AosMngr
{
public:
    explicit AosMngr(IN IMS_SINT32 nSlotId);
    virtual ~AosMngr();

private:
    AosMngr(IN const AosMngr& objRHS);
    AosMngr& operator=(IN const AosMngr& objRHS);

public:
    IAosHandle* GetAosHandle(IN const AString& strAppId, IN const AString& strSrvId);
    IMSList<IAosHandle*> GetAllAosHandles(IN const AString& strAppId, IN const AString& strSrvId);
    IMSList<IAosHandle*> GetAllAosHandles(IN const AString& strAppId);

private:
    void CreateStaticConfig();
    void CreateAos();
    void DestroyStaticConfig();
    void DestroyAos();

    IAosBuilder* AosBuilderFactory();

private:
    IMS_SINT32 m_nSlotId;
    IMSList<AString> m_objAppId;
    IMSMap<AString, IAosAppContext*> m_objAppContext;
    AosBuildDirector* m_pBuildDirector;
    AosStaticConfig* m_pStaticConfig;

private:
    friend class AosMngrTest;
};
#endif  // AOS_MNGR_H_
