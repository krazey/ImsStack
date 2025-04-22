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

#ifndef _MOCK_INTERFACE_MEDIA_SESSION_CLIENT_LISTENER_H_
#define _MOCK_INTERFACE_MEDIA_SESSION_CLIENT_LISTENER_H_

#include <gmock/gmock.h>

#include "IMediaSessionClientListener.h"

class MockIMediaSessionClientListener : public IMediaSessionClientListener
{
public:
    MOCK_METHOD(void, MediaSession_Notify,
            (IMS_UINT32 eReportType, MEDIA_CONTENT_TYPE eMediaType,
                    MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType),
            (override));
    MOCK_METHOD(void, MediaSession_NotifyFailures,
            (IMS_UINT32 eReportType, IMS_SINT32 eError, MEDIA_CONTENT_TYPE eMediaType), (override));
    MOCK_METHOD(void, MediaSession_NotifyQos,
            (IMS_UINTP nNegoId, IMS_BOOL bSuccess, MEDIA_CONTENT_TYPE eMediaType), (override));
};

#endif
