/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef SDP_READER_H_
#define SDP_READER_H_

#include "ISdpReader.h"
#include "SessionDescriptionReader.h"

/**
 * @brief A SDP reader provides access to the SDP body part
 *        when the enabler receives an error response (488 or 606) with the SDP body part.
 */
class SdpReader : public ISdpReader
{
public:
    explicit SdpReader(IN const ByteArray& objSdp);
    ~SdpReader() override;

    SdpReader(IN const SdpReader&) = delete;
    SdpReader& operator=(IN const SdpReader&) = delete;

public:
    inline ISessionDescriptor* GetSessionDescriptor() const override
    {
        return m_pSessionDescriptor;
    }
    inline const ImsList<IMediaDescriptor*>& GetMediaDescriptors() const override
    {
        return m_objMediaDescriptors;
    }

private:
    void Init(IN const ByteArray& objSdp);

private:
    SessionDescriptionReader* m_pSessionDescriptor;
    ImsList<IMediaDescriptor*> m_objMediaDescriptors;
};

#endif
