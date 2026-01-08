/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef TEST_IMS_CORE_PROTOCOL_H_
#define TEST_IMS_CORE_PROTOCOL_H_

#include "ImsMap.h"

#include "ImsCore.h"
#include "ServiceProtocol.h"

#include "MockICoreService.h"

class TestImsCoreProtocol : public ServiceProtocol
{
public:
    inline TestImsCoreProtocol() :
            ServiceProtocol(),
            m_piCoreService(&m_objCoreService)
    {
    }
    ~TestImsCoreProtocol() override = default;

    TestImsCoreProtocol(IN const TestImsCoreProtocol&) = delete;
    TestImsCoreProtocol& operator=(IN const TestImsCoreProtocol&) = delete;

public:
    inline MockICoreService& GetMockCoreService() { return m_objCoreService; }
    inline void SetCoreService(IN ICoreService* piCoreService) { m_piCoreService = piCoreService; }
    inline void SetCoreService(IN const AString& strServiceId, IN ICoreService* piCoreService)
    {
        m_objCoreServices.SetValue(strServiceId, piCoreService);
    }
    inline void ClearCoreService(IN const AString& strServiceId)
    {
        m_objCoreServices.Remove(strServiceId);
    }

protected:
    inline IService* CreateService(IN const AString& /*strAppId*/, IN const AString& strServiceId,
            IN const AString& /*strUserId*/) override
    {
        IMS_SLONG nIndex = m_objCoreServices.GetIndexOfKey(strServiceId);

        if (nIndex >= 0)
        {
            return m_objCoreServices.GetValueAt(nIndex);
        }

        return m_piCoreService;
    }
    inline const IMS_CHAR* GetConnectionScheme() const override
    {
        return ImsCore::CONNECTION_SCHEME;
    }

private:
    MockICoreService m_objCoreService;

    ICoreService* m_piCoreService;
    // <ServiceId, ICoreService>
    ImsMap<AString, ICoreService*> m_objCoreServices;
};

#endif
