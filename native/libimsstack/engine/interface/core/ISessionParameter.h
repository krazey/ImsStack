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
#ifndef INTERFACE_SESSION_PARAMETER_H_
#define INTERFACE_SESSION_PARAMETER_H_

#include "offeranswer/SdpMediaParameter.h"
#include "offeranswer/SdpSessionParameter.h"

/**
 * @brief This class provides an interface to access SDP parameters.
 */
class ISessionParameter
{
protected:
    virtual ~ISessionParameter() = default;

public:
    /**
     * @brief This method returns a session-level session description object.
     *
     * @return A SDP session-level parameter.
     */
    virtual const SdpSessionParameter& GetSessionParameter() const = 0;

    /**
     * @brief This method returns the count of media-level session description.
     *
     * @return The count of SdpMediaParameter.
     */
    virtual IMS_SINT32 GetMediaCount() const = 0;

    /**
     * @brief This method returns a media-level session description object with the specified mid.
     *
     * @param nMid Media identifier (zero-based index)
     * @return A SDP media-level parameter.
     */
    virtual SdpMediaParameter* GetMediaParameter(IN IMS_UINT32 nMid) const = 0;
};

#endif
