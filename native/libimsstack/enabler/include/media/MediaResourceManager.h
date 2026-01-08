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

#ifndef MEDIA_RESOURCE_MANAGER_H_
#define MEDIA_RESOURCE_MANAGER_H_

#include "ImsList.h"

class MediaConfiguration;

class MediaResourceManager
{
public:
    explicit MediaResourceManager();
    virtual ~MediaResourceManager();

    /**
     * @brief Get the random port number from the configuration which is not redundant with port
     * number which already using
     *
     * @param pConfig The configuration instance
     * @return IMS_UINT32 The port number acquired
     */
    virtual IMS_UINT32 AcquireRtpPort(IN MediaConfiguration* pConfig);

    /**
     * @brief Get the random port number from the configuration with the given range set
     *
     * @param nRangeStart The start number of the range
     * @param nRangeEnd The end number of the range
     * @return IMS_UINT32 The port number acquired
     */
    virtual IMS_UINT32 AcquireRtpPort(IN IMS_UINT32 nRangeStart, IN IMS_UINT32 nRangeEnd);

    /**
     * @brief Release port number
     *
     * @param nPort The port number to release
     */
    virtual IMS_BOOL ReleaseRtpPort(IN IMS_UINT32 nPort);

protected:
    ImsList<IMS_UINT32> m_lstUsedRtpPort;
};

#endif
