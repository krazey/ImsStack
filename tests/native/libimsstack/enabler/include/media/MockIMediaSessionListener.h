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

#ifndef MOCK_I_MEDIA_SESSION_LISTENER_H_
#define MOCK_I_MEDIA_SESSION_LISTENER_H_

#include <gmock/gmock.h>

#include "IJniMedia.h"
#include "IMediaSessionListener.h"

class FakeIMediaSessionListener : public IMediaSessionListener
{
public:
    FakeIMediaSessionListener() {}
    virtual ~FakeIMediaSessionListener() override {}

    IMS_BOOL MediaSession_SendMsgToMediaManager(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam) override
    {
        if (pParam == NULL)
        {
            return IMS_FALSE;
        }

        switch (eEvent)
        {
            case IJniMedia::REQUEST_OPEN_SESSION:
            case IJniMedia::REQUEST_MODIFY_SESSION:
            case IJniMedia::REQUEST_CLOSE_SESSION:
            case IJniMedia::REQUEST_ADD_CONFIG:
            case IJniMedia::REQUEST_DELETE_CONFIG:
            case IJniMedia::REQUEST_CONFIRM_CONFIG:
            case IJniMedia::REQUEST_SEND_DTMF:
            case IJniMedia::REQUEST_SET_MEDIA_QUALITY:
                delete pParam;
                break;
            default:
                break;
        }

        return IMS_TRUE;
    }

    IMS_BOOL MediaSession_NotifyToClient(IN IMS_UINT32 /* eReportType */,
            IN MEDIA_CONTENT_TYPE /* eMediaType */,
            IN MEDIA_TRANSPORT_PROTOCOL /* eMediaProtocolType */) override
    {
        return IMS_TRUE;
    }
};

class MockIMediaSessionListener : public IMediaSessionListener
{
public:
    MockIMediaSessionListener() { m_pFake = IMS_NULL; }
    virtual ~MockIMediaSessionListener() override {}

    MOCK_METHOD(IMS_BOOL, MediaSession_SendMsgToMediaManager,
            (IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam), (override));
    MOCK_METHOD(IMS_BOOL, MediaSession_NotifyToClient,
            (IN IMS_UINT32 eReportType, IN MEDIA_CONTENT_TYPE eMediaType,
                    IN MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType),
            (override));

    void DelegateToFake()
    {
        ON_CALL(*this, MediaSession_SendMsgToMediaManager)
                .WillByDefault(
                        [this](IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam)
                        {
                            if (m_pFake != nullptr)
                            {
                                return m_pFake->MediaSession_SendMsgToMediaManager(eEvent, pParam);
                            }

                            return IMS_FALSE;
                        });
    }

    void SetDelegate(IMediaSessionListener* pFake) { m_pFake = pFake; }

private:
    IMediaSessionListener* m_pFake;
};

#endif
