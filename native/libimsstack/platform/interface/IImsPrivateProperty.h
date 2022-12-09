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
#ifndef INTERFACE_IMS_PRIVATE_PROPERTY_H_
#define INTERFACE_IMS_PRIVATE_PROPERTY_H_

#include "AString.h"
#include "ImsPrivateProperties.h"

/**
 * Ephemeral properties : cleared whenever IMS process starts
 * Persistent properties : preserved even though device or IMS re-starts
 */
class IImsPrivateProperty
{
protected:
    virtual ~IImsPrivateProperty() = default;

public:
    /**
     * @brief Gets an ephemeral IMS private property.
     *
     * @param strKey The key of the property to be retrieved
     * @param nSlotId The slot-id
     * @return A string value of the specified key.
     */
    virtual AString Get(IN const AString& strKey, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Gets an ephemeral IMS private property as boolean.
     *
     * @param strKey The key of the property to be retrieved
     * @param nSlotId The slot-id
     * @return A boolean value of the specified key.
     */
    virtual IMS_BOOL GetBoolean(IN const AString& strKey, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Gets an ephemeral IMS private property as integer.
     *
     * @param strKey The key of the property to be retrieved
     * @param nSlotId The slot-id
     * @return An integer value of the specified key.
     */
    virtual IMS_SINT32 GetInt(IN const AString& strKey, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Sets an ephemeral IMS private property.
     *
     * @param strKey The key of the property
     * @param strValue The value of the property
     * @param nSlotId The slot-id
     */
    virtual void Set(
            IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Sets an ephemeral IMS private property as boolean.
     *
     * @param strKey The key of the property
     * @param bValue The value of the property
     * @param nSlotId The slot-id
     */
    virtual void SetBoolean(
            IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Sets an ephemeral IMS private property as integer.
     *
     * @param strKey The key of the property
     * @param nValue The value of the property
     * @param nSlotId The slot-id
     */
    virtual void SetInt(IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Gets a persistent IMS private property.
     *
     * @param strKey The key of the property to be retrieved
     * @param nSlotId The slot-id
     * @return A string value of the specified key.
     */
    virtual AString GetPersistent(IN const AString& strKey, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Gets a persistent IMS private property as boolean.
     *
     * @param strKey The key of the property to be retrieved
     * @param nSlotId The slot-id
     * @return A boolean value of the specified key.
     */
    virtual IMS_BOOL GetPersistentBoolean(IN const AString& strKey, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Gets a persistent IMS private property as integer.
     *
     * @param strKey The key of the property to be retrieved
     * @param nSlotId The slot-id
     * @return An integer value of the specified key.
     */
    virtual IMS_SINT32 GetPersistentInt(IN const AString& strKey, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Sets a persistent IMS private property.
     *
     * @param strKey The key of the property
     * @param strValue The value of the property
     * @param nSlotId The slot-id
     */
    virtual void SetPersistent(
            IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Sets a persistent IMS private property as boolean.
     *
     * @param strKey The key of the property
     * @param bValue The value of the property
     * @param nSlotId The slot-id
     */
    virtual void SetPersistentBoolean(
            IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Sets a persistent IMS private property as integer.
     *
     * @param strKey The key of the property
     * @param nValue The value of the property
     * @param nSlotId The slot-id
     */
    virtual void SetPersistentInt(
            IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId) = 0;
};

#endif
