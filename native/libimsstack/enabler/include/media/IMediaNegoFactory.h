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

#ifndef I_MEDIA_NEGO_FACTORY_H_
#define I_MEDIA_NEGO_FACTORY_H_

#include <memory>

#include "ImsTypeDef.h"

class MediaNego;
class MediaEnvironment;

class IMediaNegoFactory
{
public:
    IMediaNegoFactory() = default;
    virtual ~IMediaNegoFactory() = default;
    virtual std::shared_ptr<MediaNego> CreateMediaNego(IMS_UINT32 nSlotId) = 0;
    virtual std::shared_ptr<MediaNego> CreateForkedMediaNego(IMS_UINT32 nSlotId,
            std::shared_ptr<MediaNego> pExistingNego,
            std::shared_ptr<MediaEnvironment> pEnvironment) = 0;
};

#endif
