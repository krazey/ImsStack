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
#include "ImsCore.h"
#include "ProtocolPermission.h"
#include "Sip.h"

#include "TestConnector.h"
#include "TestImsCoreProtocol.h"
#include "TestSipProtocol.h"

class TestConnectorPrivate
{
public:
    inline TestConnectorPrivate()
    {
        ProtocolPermission::RegisterProtocol(ImsCore::CONNECTION_SCHEME, &m_objImsCoreProtocol);
        ProtocolPermission::RegisterProtocol(Sip::CONNECTION_SCHEME_SIP, &m_objSipProtocol);
    }
    inline ~TestConnectorPrivate() { ProtocolPermission::UnregisterAllProtocols(); }

public:
    inline MockICoreService& GetMockCoreService()
    {
        return m_objImsCoreProtocol.GetMockCoreService();
    }
    inline MockISipClientConnection& GetMockSipClientConnection()
    {
        return m_objSipProtocol.GetMockSipClientConnection();
    }
    inline void SetCoreService(IN ICoreService* piCoreService)
    {
        m_objImsCoreProtocol.SetCoreService(piCoreService);
    }
    inline void SetCoreService(IN const AString& strServiceId, IN ICoreService* piCoreService)
    {
        m_objImsCoreProtocol.SetCoreService(strServiceId, piCoreService);
    }
    inline void ClearCoreService(IN const AString& strServiceId)
    {
        m_objImsCoreProtocol.ClearCoreService(strServiceId);
    }
    inline void SetSipClientConnection(IN ISipClientConnection* piScc)
    {
        m_objSipProtocol.SetSipClientConnection(piScc);
    }

private:
    TestImsCoreProtocol m_objImsCoreProtocol;
    TestSipProtocol m_objSipProtocol;
};

TestConnector::TestConnector() :
        m_pPrivate(new TestConnectorPrivate())
{
}

TestConnector::~TestConnector()
{
    delete m_pPrivate;
}

MockICoreService& TestConnector::GetMockCoreService()
{
    return m_pPrivate->GetMockCoreService();
}

MockISipClientConnection& TestConnector::GetMockSipClientConnection()
{
    return m_pPrivate->GetMockSipClientConnection();
}

void TestConnector::SetCoreService(IN ICoreService* piCoreService)
{
    m_pPrivate->SetCoreService(piCoreService);
}

void TestConnector::SetCoreService(IN const AString& strServiceId, IN ICoreService* piCoreService)
{
    m_pPrivate->SetCoreService(strServiceId, piCoreService);
}

void TestConnector::ClearCoreService(IN const AString& strServiceId)
{
    m_pPrivate->ClearCoreService(strServiceId);
}

void TestConnector::SetSipClientConnection(IN ISipClientConnection* piScc)
{
    m_pPrivate->SetSipClientConnection(piScc);
}
