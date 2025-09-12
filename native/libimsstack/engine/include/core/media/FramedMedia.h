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
#ifndef FRAMED_MEDIA_H_
#define FRAMED_MEDIA_H_

#include "AStringArray.h"
#include "ByteArray.h"
#include "ImsMap.h"

#include "media/Media.h"

class FramedMedia : public Media
{
public:
    FramedMedia(IN Service* pService, IN ISdpOaState* piOaState);
    ~FramedMedia() override;

    FramedMedia(IN const FramedMedia&) = delete;
    FramedMedia& operator=(IN const FramedMedia&) = delete;

public:
    // Media class
    inline IMS_SINT32 GetType() const override { return ImsCore::MEDIA_TYPE_FRAMED; }

protected:
    // Media class
    MediaProposal* CreateMediaProposal(IN ISdpOaState* piOaState) override;
};

#endif
