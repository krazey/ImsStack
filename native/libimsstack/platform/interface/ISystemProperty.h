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
#ifndef INTERFACE_SYSTEM_PROPERTY_H_
#define INTERFACE_SYSTEM_PROPERTY_H_

#include "AString.h"

class ISystemProperty
{
protected:
    virtual ~ISystemProperty() = default;

public:
    /**
     * @brief Gets the system property.
     *
     * @param strName The name of the system property to be retrieved
     * @return A value of the specified key or NULL string if not present.
     */
    virtual AString Get(IN const AString& strName) = 0;

    /**
     * @brief Sets the system property.
     *
     * @param strName The name of the system property
     * @param strValue The value of the system property
     * @return IMS_TRUE if the operation succeeds, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Set(IN const AString& strName, IN const AString& strValue) = 0;

    //// FOR READ ONLY PROPERTIES

    /**
     * @brief Returns the string of the modem chipset vendor.
     *
     * @return A string of the modem chipset vendor - "qct" / "mtk" / ...
     */
    virtual const AString& GetChipsetVendor() const = 0;

    /**
     * @brief Checks if the debug mode is on or not.
     *
     * @return IMS_TRUE if the debug mode is enabled, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsDebugMode() const = 0;

    /**
     * @brief Checks if the server info. (routing info.) MUST be hidden or not.
     *
     * @return IMS_TRUE if the server info. (routing info.) MUST be hidden in the log,
     *         IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsServerInfoHiddenInLog() const = 0;

    /**
     * @brief Checks if the build type is user mode or not.
     *
     * @return IMS_TRUE if the build type is user mode, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsUserMode() const = 0;
};

#endif
