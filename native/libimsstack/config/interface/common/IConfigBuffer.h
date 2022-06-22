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
#ifndef INTERFACE_CONFIG_BUFFER_H_
#define INTERFACE_CONFIG_BUFFER_H_

#include "AStringArray.h"

class IConfigBuffer
{
public:
    /**
     * @brief Destroys the configuration buffer.
     *
     * This interface can be instantiated by Configuration interface for IMS enablers.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Captures the current work section to read/write the item value.
     *
     * @param pszSectName The section string to be worked
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName) = 0;

    /**
     * @brief Captures the current work section which has a property of list section
     *        to read/write the item value.
     *
     * @param pszSectName The section string to be worked
     * @param nIndex The index of the section list
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName, IN IMS_SINT32 nIndex) = 0;

    /**
     * @brief Releases the current work section.
     *
     * After invoking this method, READ/WRITE operation will be failed.
     */
    virtual void ReleaseSection() = 0;

    /**
     * @brief Reads the count of the specified key with 'count' property.
     *
     * For example,
     *   abc_count=3
     *   -> IMS_SINT32 nCount = piBuffer->ReadKeyCount("abc");
     *   -> nCount will be 3.
     *
     * @param pszKey The key string
     * @return The count of the specified key.
     */
    virtual IMS_SINT32 ReadKeyCount(IN const IMS_CHAR* pszKey) const = 0;

    /**
     * @brief Reads the string value of the specified key with 'string' property.
     *
     * @param pszKey The key string
     * @return The string value of the specified key.
     */
    virtual const AString& ReadValue(IN const IMS_CHAR* pszKey) const = 0;

    /**
     * @brief Reads the string value of the specified key with 'string' property & 'list' property.
     *
     * @param pszKey The key string
     * @param nIndex The index of key list
     * @return The string value of the specified key.
     */
    virtual const AString& ReadValue(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex) const = 0;

    /**
     * @brief Reads the boolean value of the specified key with 'boolean' property.
     *
     * @param pszKey The key string
     * @return The boolean value of the specified key.
     */
    virtual IMS_BOOL ReadValueBoolean(IN const IMS_CHAR* pszKey) const = 0;

    /**
     * @brief Reads the integer value of the specified key with 'integer' property.
     *
     * @param pszKey The key string
     * @return The integer value of the specified key.
     */
    virtual IMS_SINT32 ReadValueInt(IN const IMS_CHAR* pszKey) const = 0;

    /**
     * @brief Writes the count of the specified key with 'count' property.
     *
     * For example,
     *   piBuffer->WriteKeyCount("abc", 3);
     *   -> abc_count=3
     *
     * @param pszKey The key string
     * @param nCount The count value to be written
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL WriteKeyCount(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nCount) = 0;

    /**
     * @brief Writes the string value of the specified key with 'string' property.
     *
     * @param pszKey The key string
     * @param strValue The string value to be written
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL WriteValue(IN const IMS_CHAR* pszKey, IN const AString& strValue) = 0;

    /**
     * @brief Writes the string value of the specified key with 'string' property & 'list' property.
     *
     * @param pszKey The key string
     * @param nIndex The position that the value will be written
     * @param strValue The string value to be written
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL WriteValue(
            IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex, IN const AString& strValue) = 0;

    /**
     * @brief Reads the boolean value of the specified key with 'boolean' property.
     *
     * @param pszKey The key string
     * @param bValue The boolean value to be written
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL WriteValueBoolean(IN const IMS_CHAR* pszKey, IN IMS_BOOL bValue) = 0;

    /**
     * @brief Writes the integer value of the specified key with 'integer' property.
     *
     * @param pszKey The key string
     * @param nValue The boolean value to be written
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL WriteValueInt(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue) = 0;

    /**
     * @brief Writes the all configuration values to the specified medium.
     *
     * @return IMS_TRUE if the operation is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL WriteToMedium() const = 0;
};

#endif
