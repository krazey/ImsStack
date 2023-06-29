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

#ifndef TEST_IMS_RADIO_SERVICE_H_
#define TEST_IMS_RADIO_SERVICE_H_

#include "ServiceImsRadio.h"
#include "MockIImsRadio.h"
#include "MockIImsTraffic.h"

class TestImsRadioService : public ImsRadioService
{
public:
    inline TestImsRadioService() :
            ImsRadioService(),
            m_piImsRadio(&m_objImsRadio),
            m_piImsTraffic(&m_objImsTraffic)
    {
    }

    inline IImsRadio* GetImsRadio(IN IMS_SINT32 /* nSlotId */) override { return m_piImsRadio; }
    inline IImsTraffic* GetImsTraffic() override { return m_piImsTraffic; }

    inline MockIImsRadio& GetMockImsRadio() { return m_objImsRadio; }
    inline MockIImsTraffic& GetMockImsTraffic() { return m_objImsTraffic; }

private:
    MockIImsRadio m_objImsRadio;
    MockIImsTraffic m_objImsTraffic;

    IImsRadio* m_piImsRadio;
    IImsTraffic* m_piImsTraffic;
};

#endif
