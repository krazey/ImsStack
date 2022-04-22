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
#ifndef SDP_PROFILE_H_
#define SDP_PROFILE_H_

#include "ImsTypeDef.h"

class SdpProfilePrivate;

class SdpProfile
{
private:
    SdpProfile();
    ~SdpProfile();

public:
    void InitFeatures(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nNewFeatures);
    IMS_SINT32 GetFeatures(IN IMS_SINT32 nSlotId) const;

    IMS_BOOL IsAttributeDirectionRequiredForRemovedMedia() const;
    IMS_BOOL IsAttributeDirectionRequiredForRemovedMedia(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsAttributePreconditionSupported(IN IMS_SINT32 nSlotId) const;

    static SdpProfile* GetInstance();

public:
    enum
    {
        FEATURE_NONE = 0,
        // Indicates that the attribute direction is required for removed media.
        FEATURE_A_DIRECTION_REQUIRED_FOR_REMOVED_MEDIA = 1,
        // Indicates that the "qos" precondition is supported or not.
        FEATURE_A_PRECONDITION_SUPPORTED = 1 << 1
    };

private:
    SdpProfilePrivate* m_pPrivate;
};

#endif
