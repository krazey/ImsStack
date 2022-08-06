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
#ifndef TEST_UTIL_SERVICE_H_
#define TEST_UTIL_SERVICE_H_

#include "MockIImsPrivateProperty.h"
#include "MockISystemProperty.h"
#include "MockISystemUtil.h"
#include "MockIZLib.h"
#include "ServiceUtil.h"

class TestUtilService : public UtilService
{
public:
    inline TestUtilService() :
            UtilService(),
            m_piPrivateProperty(&m_objPrivateProperty),
            m_piSystemUtil(&m_objSystemUtil),
            m_piSystemProperty(&m_objSystemProperty),
            m_piZLib(&m_objZLib)
    {
    }

    inline IImsPrivateProperty* GetPrivateProperty() override { return m_piPrivateProperty; }
    inline ISystemUtil* GetSystemUtil() override { return m_piSystemUtil; }
    inline ISystemProperty* GetSystemProperty() override { return m_piSystemProperty; }
    inline IZLib* GetZLib() override { return m_piZLib; }

    inline MockIImsPrivateProperty& GetMockPrivateProperty() { return m_objPrivateProperty; }
    inline MockISystemUtil& GetMockSystemUtil() { return m_objSystemUtil; }
    inline MockISystemProperty& GetMockSystemProperty() { return m_objSystemProperty; }
    inline MockIZLib& GetMockZLib() { return m_objZLib; }
    inline void SetPrivateProperty(IN IImsPrivateProperty* piPrivateProperty)
    {
        m_piPrivateProperty = piPrivateProperty;
    }
    inline void SetSystemUtil(IN ISystemUtil* piSystemUtil) { m_piSystemUtil = piSystemUtil; }
    inline void SetSystemProperty(IN ISystemProperty* piSystemProperty)
    {
        m_piSystemProperty = piSystemProperty;
    }
    inline void SetZLib(IN IZLib* piZLib) { m_piZLib = piZLib; }

private:
    MockIImsPrivateProperty m_objPrivateProperty;
    MockISystemUtil m_objSystemUtil;
    MockISystemProperty m_objSystemProperty;
    MockIZLib m_objZLib;

    IImsPrivateProperty* m_piPrivateProperty;
    ISystemUtil* m_piSystemUtil;
    ISystemProperty* m_piSystemProperty;
    IZLib* m_piZLib;
};

#endif
