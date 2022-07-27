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
    inline IImsPrivateProperty* GetPrivateProperty() override { return &m_objPrivateProperty; }
    inline ISystemUtil* GetSystemUtil() override { return &m_objSystemUtil; }
    inline ISystemProperty* GetSystemProperty() override { return &m_objSystemProperty; }
    inline IZLib* GetZLib() override { return &m_objZLib; }

    inline MockIImsPrivateProperty& GetMockPrivateProperty() { return m_objPrivateProperty; }
    inline MockISystemUtil& GetMockSystemUtil() { return m_objSystemUtil; }
    inline MockISystemProperty& GetMockSystemProperty() { return m_objSystemProperty; }
    inline MockIZLib& GetMockZLib() { return m_objZLib; }

private:
    MockIImsPrivateProperty m_objPrivateProperty;
    MockISystemUtil m_objSystemUtil;
    MockISystemProperty m_objSystemProperty;
    MockIZLib m_objZLib;
};

#endif
