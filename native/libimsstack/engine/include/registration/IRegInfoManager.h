/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef INTERFACE_REG_INFO_MANAGER_H_
#define INTERFACE_REG_INFO_MANAGER_H_

#include "AString.h"

#include "RegKey.h"

class RegInfo;

/**
 * @brief An interface for managing the information of "reg" event package.
 */
class IRegInfoManager
{
protected:
    virtual ~IRegInfoManager() = default;

public:
    /**
     * @brief Creates a RegInfo instance with the specified registration key.
     *
     * @param objRegKey Registration key
     * @return IMS_TRUE if RegInfo is successfully created, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL CreateRegInfo(IN const RegKey& objRegKey) = 0;

    /**
     * @brief Destroys the RegInfo instance with the specified registration key.
     *
     * @param objRegKey Registration key
     */
    virtual void DestroyRegInfo(IN const RegKey& objRegKey) = 0;

    /**
     * @brief Gets the RegInfo instance matching the specified registration key.
     *
     * @param objRegKey Registration key
     * @return RegInfo instance or IMS_NULL.
     */
    virtual RegInfo* GetRegInfo(IN const RegKey& objRegKey) = 0;

    /**
     * @brief Gets the RegInfo instance matching the specified registration key.
     *
     * @param objRegKey Registration key
     * @return RegInfo instance or IMS_NULL.
     */
    virtual const RegInfo* GetRegInfo(IN const RegKey& objRegKey) const = 0;

    /**
     * @brief Updates the IRegInfoManager with the specified registration key and
     *        the registration information.
     *
     * @param objRegKey Registration key
     * @param strRegInfo Registration information received from the IMS server
     * @return IMS_TRUE if it's successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Update(IN const RegKey& objRegKey, IN const AString& strRegInfo) = 0;

    /**
     * @brief Displays all the current registration information.
     *
     * NOTE: This is used for debugging purpose.
     */
    virtual void DisplayRegInfo() const = 0;
};

#endif
