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
#ifndef INTERFACE_IMS_ISIM_H_
#define INTERFACE_IMS_ISIM_H_

#include "AStringArray.h"

class IDigestAka;
class IIsimListener;

class IIsim
{
protected:
    virtual ~IIsim() = default;

public:
    /**
     * @brief Creates the Digest AKA.
     *
     * @return The pointer of IDigestAka.
     */
    virtual IDigestAka* CreateDigestAka() = 0;

    /**
     * @brief Gets the home domain name.
     *
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @return A home domain name of ISIM record.
     */
    virtual AString GetHomeDomainName() const = 0;

    /**
     * @brief Gets the private user identity.
     *
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @return A private user identity of ISIM record.
     */
    virtual AString GetImpi() const = 0;

    /**
     * @brief Gets the public user identities.
     *
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @return A list of public user identity of ISIM record.
     */
    virtual AStringArray GetImpu() const = 0;

    /**
     * @brief Gets the list of P-CSCF address string (IP address or FQDN)
     *
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @return A list of IMS Proxy Call Session Control Function(P-CSCF) of ISIM record.
     */
    virtual AStringArray GetPcscf() const = 0;

    /**
     * @brief Returns the state of ISIM.
     *
     * @return The ISIM state.\n
     *         #STATE_IDLE\n
     *         #STATE_INIT\n
     *         #STATE_READY\n
     *         #STATE_REFRESHING\n
     *         #STATE_REFRESHED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Checks if the load of ISIM application completes or not.
     *
     * @return IMS_TRUE if ISIM application is completely loaded, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsLoadCompleted() const = 0;

    /**
     * @brief Adds the listener to receive the result of operation.
     *
     * @param piListener The listener to be added
     */
    virtual void AddListener(IN IIsimListener* piListener) = 0;

    /**
     * @brief Removes the listener to receive the result of operation.
     *
     * @param piListener The listener to be removed
     */
    virtual void RemoveListener(IN IIsimListener* piListener) = 0;

    /**
     * @brief Initializes the ISIM module to register a listener and its event callback to
     *        communicate with the ISIM application of the device.
     */
    virtual void Init() = 0;

    /**
     * @brief Releases all the resource related to the ISIM.
     */
    virtual void Release() = 0;

public:
    /// Identifiers of ISIM record fields
    enum
    {
        EF_NONE = 0x0000,

        EF_IMPI = 0x0001,
        EF_DOMAIN = 0x0002,
        EF_IMPU = 0x0004,

        EF_ALL = 0xFFFF
    };

    enum
    {
        /// The default state.
        STATE_IDLE = 0,
        /// When ISIM application is not present in the UICC application.
        STATE_NOT_PRESENT,
        /// When ISIM application is detected and all the records are loaded.
        STATE_LOADED,
        /// When ISIM application starts the ISIM refresh.
        /// If the ISIM refresh completes, the ISIM state is transiting to LOADED.
        STATE_REFRESHING
    };
};

#endif
