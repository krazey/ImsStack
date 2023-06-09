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
#ifndef INTERFACE_SDP_READER_H_
#define INTERFACE_SDP_READER_H_

#include "ImsList.h"

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"

/**
 * @brief A SDP reader provides access to the SDP body part
 *        when the enabler receives an error response (488 or 606) with the SDP body part.
 */
class ISdpReader
{
protected:
    virtual ~ISdpReader() = default;

public:
    /**
     * @brief Returns the session descriptor associated with this ISdpReader.
     *
     * The returned ISessionDescriptor does not allow to add / set / remove any SDP related
     * information.
     *
     * @return Pointer to ISessionDescriptor.
     */
    virtual ISessionDescriptor* GetSessionDescriptor() const = 0;

    /**
     * @brief Returns the media descriptor(s) associated with this ISdpReader.
     *
     * The returned IMediaDescriptor does not allow to add / set / remove any SDP related
     * information.
     *
     * @return List of pointer to IMediaDescriptor.
     */
    virtual const ImsList<IMediaDescriptor*>& GetMediaDescriptors() const = 0;
};

#endif
