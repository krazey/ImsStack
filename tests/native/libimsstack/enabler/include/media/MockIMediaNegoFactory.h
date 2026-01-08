/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef I_MOCK_MEDIA_NEGO_FACTORY_H_
#define I_MOCK_MEDIA_NEGO_FACTORY_H_

#include <gmock/gmock.h>

#include "IMediaNegoFactory.h"
#include "MediaEnvironment.h"
#include "MediaNego.h"

/**
 * @brief Mocking the IMediaNegoFactory
 */
class MockMediaNegoFactory : public IMediaNegoFactory
{
public:
    MOCK_METHOD(std::shared_ptr<MediaNego>, CreateMediaNego, (IMS_UINT32 nSlotId), (override));
    MOCK_METHOD(std::shared_ptr<MediaNego>, CreateForkedMediaNego,
            (IMS_UINT32 nSlotId, std::shared_ptr<MediaNego> pExistingNego,
                    std::shared_ptr<MediaEnvironment> pEnvironment),
            (override));
};

#endif
