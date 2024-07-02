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
#ifndef TEST_SIP_PROTOCOL_H_
#define TEST_SIP_PROTOCOL_H_

#include "Protocol.h"

#include "MockISipClientConnection.h"

class TestSipProtocol : public Protocol
{
public:
    inline TestSipProtocol() :
            Protocol(),
            m_piScc(&m_objScc)
    {
    }
    ~TestSipProtocol() override = default;

    TestSipProtocol(IN const TestSipProtocol&) = delete;
    TestSipProtocol& operator=(IN const TestSipProtocol&) = delete;

public:
    // Protocol class
    IConnection* OpenPrim(IN const AString& strName) override;
    IConnection* OpenPrim(IN const AString& strScheme, IN const AString& strTarget,
            IN const AString& strParams) override;

    inline MockISipClientConnection& GetMockSipClientConnection() { return m_objScc; }
    inline void SetSipClientConnection(ISipClientConnection* piScc) { m_piScc = piScc; }

private:
    MockISipClientConnection m_objScc;

    ISipClientConnection* m_piScc;
};

#endif
