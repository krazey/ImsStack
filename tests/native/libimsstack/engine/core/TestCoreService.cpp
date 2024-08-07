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

#include "ISipConfig.h"

#include "ImsCoreContext.h"
#include "ServiceManager.h"
#include "ServiceResolver.h"
#include "TestCoreService.h"

TestCoreService::TestCoreService() :
        CoreService(TestAppConfig::TEST_APP_NAME, TestAppConfig::TEST_SERVICE_NAME_1, IMS_NULL),
        m_piScc(&m_objScc),
        m_piSccForMidDialog(&m_objSccForMidDialog),
        m_piSccForCancel(&m_objSccForCancel)
{
    ImsCoreContext::GetInstance()->GetServiceManager()->AttachService(this);
    ServiceResolver::SetRegBinding(IMS_SLOT_0, TestAppConfig::TEST_APP_NAME,
            TestAppConfig::TEST_SERVICE_NAME_1, &m_objRegBinding);

    RcPtr<SipProfile> pSipProfile = new SipProfile();
    pSipProfile->SetSipFeatureCaps(ISipConfig::SIP_FEATURE_CAPS_IPSEC);
    SetSipProfile(pSipProfile.Get());

    AppConfig objAppConfig =
            TestAppConfig::Create(TestAppConfig::TEST_APP_NAME, TestAppConfig::TEST_SERVICE_NAME_1);

    CreateConfig(objAppConfig);
}

TestCoreService::~TestCoreService()
{
    ImsCoreContext::GetInstance()->GetServiceManager()->DetachService(this);
}
