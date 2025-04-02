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
#ifndef INTERFACE_CARRIER_CONFIG_H_
#define INTERFACE_CARRIER_CONFIG_H_

#include "AString.h"
#include "ImsVector.h"

#include "ICarrierConfigListener.h"

class ICarrierConfig
{
protected:
    virtual ~ICarrierConfig() = default;

public:
    /**
     * @brief Returns the boolean value for a specified key.
     *
     * @param pszKey The config key
     * @param bDefaultValue The default value if not present
     * @return A boolean value if present. Otherwise, returns a default value.
     */
    virtual IMS_BOOL GetBoolean(
            IN const IMS_CHAR* pszKey, IN IMS_BOOL bDefaultValue = IMS_FALSE) const = 0;

    /**
     * @brief Returns the integer value for a specified key.
     *
     * @param pszKey The config key
     * @param nDefaultValue The default value if not present
     * @return An integer value if present. Otherwise, returns a default value.
     */
    virtual IMS_SINT32 GetInt(
            IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue = -1) const = 0;

    /**
     * @brief Returns the long value for a specified key.
     *
     * @param pszKey The config key
     * @param nDefaultValue The default value if not present
     * @return A long value if present. Otherwise, returns a default value.
     */
    virtual IMS_SLONG GetLong(
            IN const IMS_CHAR* pszKey, IN IMS_SLONG nDefaultValue = -1L) const = 0;

    /**
     * @brief Returns the string value for a specified key.
     *
     * @param pszKey The config key
     * @param strDefaultValue The default value if not present
     * @return A string value if present. Otherwise, returns a default value.
     */
    virtual AString GetString(IN const IMS_CHAR* pszKey,
            IN const AString& strDefaultValue = AString::ConstNull()) const = 0;

    /**
     * @brief Returns the boolean-array value for a specified key.
     *
     * @param pszKey The config key
     * @param bKeyExists A flag used to check the presence or absence of a key in an empty vector
     * @return A boolean-array value if present. Otherwise, returns an empty vector.
     */
    virtual ImsVector<IMS_BOOL> GetBooleanArray(IN const IMS_CHAR* pszKey,
            OUT IMS_BOOL& bKeyExists = ByRef<IMS_BOOL>(IMS_TRUE)) const = 0;

    /**
     * @brief Returns the integer-array value for a specified key.
     *
     * @param pszKey The config key
     * @param bKeyExists A flag used to check the presence or absence of a key in an empty vector
     * @return An integer-array value if present. Otherwise, returns an empty vector.
     */
    virtual ImsVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey,
            OUT IMS_BOOL& bKeyExists = ByRef<IMS_BOOL>(IMS_TRUE)) const = 0;

    /**
     * @brief Returns the long-array value for a specified key.
     *
     * @param pszKey The config key
     * @param bKeyExists A flag used to check the presence or absence of a key in an empty vector
     * @return A long-array value if present. Otherwise, returns an empty vector.
     */
    virtual ImsVector<IMS_SLONG> GetLongArray(IN const IMS_CHAR* pszKey,
            OUT IMS_BOOL& bKeyExists = ByRef<IMS_BOOL>(IMS_TRUE)) const = 0;

    /**
     * @brief Returns the string-array value for a specified key.
     *
     * @param pszKey The config key
     * @param bKeyExists A flag used to check the presence or absence of a key in an empty vector
     * @return A string-array value if present. Otherwise, returns an empty vector.
     */
    virtual ImsVector<AString> GetStringArray(IN const IMS_CHAR* pszKey,
            OUT IMS_BOOL& bKeyExists = ByRef<IMS_BOOL>(IMS_TRUE)) const = 0;

    /**
     * @brief Returns the bundle value for a specified key.
     *        If it returns non-null value, then the caller MUST release the returned
     *        ICarrierConfig by calling the ReleaseBundle() method when it's not used anymore.
     *
     * @param pszKey The config key
     * @return An instance of ICarrierConfig if present. Otherwise, returns null.
     */
    virtual ICarrierConfig* GetBundle(IN const IMS_CHAR* pszKey) const = 0;

    /**
     * @brief Releases the bundle instance.
     *        If this ICarrierConfig is not a bundle, this method call is ignored.
     */
    virtual void ReleaseBundle() = 0;

    /**
     * @brief Adds a listener for the carrier configuration change.
     *
     * @param piListener The listener to be set
     */
    virtual void AddListener(IN ICarrierConfigListener* piListener) = 0;

    /**
     * @brief Removes a listener for the carrier configuration change.
     *
     * @param piListener The listener to be removed
     */
    virtual void RemoveListener(IN ICarrierConfigListener* piListener) = 0;
};

#endif
