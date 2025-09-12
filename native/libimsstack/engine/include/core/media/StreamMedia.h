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
#ifndef STREAM_MEDIA_H_
#define STREAM_MEDIA_H_

#include "media/Media.h"

class StreamMedia : public Media
{
public:
    StreamMedia(IN Service* pService, IN ISdpOaState* piOaState);
    ~StreamMedia() override;

    StreamMedia(IN const StreamMedia&) = delete;
    StreamMedia& operator=(IN const StreamMedia&) = delete;

public:
    // IMedia interface
    inline IMS_SINT32 GetType() const override { return ImsCore::MEDIA_TYPE_STREAM; }

protected:
    MediaProposal* CreateMediaProposal(IN ISdpOaState* piOaState) override;
};

#endif
