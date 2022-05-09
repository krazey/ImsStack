/**
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

/*!
 *    @brief        implementation of the MMPFSession
 */

#include <mmpf/MMPFSession.h>

class MMPFSessionListener : public IMMPFListener
{
public:
    void OnResponse(const HMMPFSession /*hMMPFSession*/, const eMMPFRequest /*nRequest*/,
            const eMMPFResponse /*eResponse*/)
    {
        // no-op
    }

    void OnNotify(const HMMPFSession /*hMMPFSession*/, const eMMPFNotify /*nNotify*/,
            const tMMPFNotificationData* /*pNotifyData*/)
    {
        // no-op
    }
};

void MMPFSession::Clear()
{
    // no-op
}

MMPFSession* MMPFSession::create(
        MMPF_IN eMMPFInterfaceID /*eID*/, MMPF_IN eMMPFSessionType /*eSessionType*/)
{
    return nullptr;
}

void MMPFSession::destroy(MMPF_IN MMPFSession* /*pMMPFSession*/)
{
    // no-op
}

MMPFSession::MMPFSession() {}

MMPFSession::~MMPFSession() {}

mmpf_bool MMPFSession::Initialize(eMMPFSessionType /*eSessionType*/)
{
    return MMPF_TRUE;
}

eMMPFResult MMPFSession::setListener(MMPF_IN IMMPFSessionListener* /*pListener*/)
{
    return MMPF_RESULT_OK;
}

eMMPFResult MMPFSession::setProperty(MMPF_IN const tMMPFProperty* /*pstProperty*/)
{
    return MMPF_RESULT_OK;
}

eMMPFResult MMPFSession::request(MMPF_IN const tMMPFRequest* /*pstRequest*/)
{
    return MMPF_RESULT_ERR_INVALID_INTERFACE;
}
