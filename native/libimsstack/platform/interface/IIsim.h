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

#include "ImsTypeDef.h"

class IDigestAka;
class IIsimListener;

class IIsim
{
public:
    /**
     * @brief Clears all the records which are obtained during the initialization
     *        or refresh procedure.
     *
     * This method can be invoked to read ISIM records only excluding the EF attributes.
     */
    virtual void ClearRecords() = 0;

    /**
     * @brief Creates the Digest AKA.
     *
     * @return The pointer of IDigestAka.
     */
    virtual IDigestAka* CreateDigestAka() = 0;

    /**
     * @brief Gets the value of the specific field.
     *
     * The field type will be determined if needed.
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @param nField The field type to be read
     * @return IMS_SUCCESS if the operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT GetField(IN IMS_SINT32 nField) = 0;

    /**
     * @brief Gets the home domain name.
     *
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @return IMS_SUCCESS if the operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT GetHomeDomainName() = 0;

    /**
     * @brief Gets the private user identity.
     *
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @return IMS_SUCCESS if the operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT GetImpi() = 0;

    /**
     * @brief Gets the public user identities.
     *
     * NOTE: The result of this operation will be reported by the IIsimListener interface.
     *
     * @return IMS_SUCCESS if the operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT GetImpu() = 0;

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
     * @brief Checks if the ISIM is ready or not.
     *
     * If the return value is IMS_FALSE, the application MUST wait for the invocation of
     * Isim_OnStateChanged(...) method in the IIsimListener interface with STATE_READY.
     *
     * @return IMS_TRUE if the ISIM is ready, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsReady() = 0;

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
     * @brief Starts the ISIM module to establish a session to communicate with
     *        the ISIM application of the device.
     *
     * @param nEFs The selected EFs to be retrieved from ISIM
     * @return IMS_SUCCESS if the operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Start(IN IMS_SINT32 nEFs = EF_ALL) = 0;

    /**
     * @brief Starts the ISIM module to register a client and its event callback to
     *        communicate with the ISIM application of the device.
     *
     * @return IMS_SUCCESS if the operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Init() = 0;

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
        EF_IST = 0x0008,
        EF_PCSCF = 0x0010,

        EF_ALL = 0xFFFF
    };

    /// Additional field values : TBD
    enum
    {
        /// For other field values
        FIELD_NONE = 0,
        FIELD_IST,
        FIELD_PCSCF_ADDRESS,
        FIELD_MAX
    };

    enum
    {
        /// The default state.
        STATE_IDLE = 0,
        /// A client is registered to ISIM application.
        STATE_INIT,
        /// User can execute ISIM operation like reading IMPI or making the authentication.
        STATE_READY,
        /// ISIM application is refreshing.
        STATE_REFRESHING,
        /// ISIM application refresh is completed.
        STATE_REFRESHED
    };


    /**
     * ERROR_INIT_FAILED means
     *      that it failed to register a client and its event callback. it's a total failure.
     * ERROR_START_FAILED means
     *      that it failed to establish a session. User can recover with STATE_INIT.
     * ERROR_READ_IMPI_FAILED, ERROR_READ_IMPU_FAILED, and ERROR_READ_DOMAIN_FAILED mean
     *      that it failed to read IMPI, IMPU, and DOMAIN for some reason.
     *      User can resume with STATE_INIT.
     * ERROR_READ_PCSCF_ADDRESS_FAILED means
     *      that it failed to read PCSCF Address for some reason. User can resume with STATE_INIT.
     * ERROR_READ_DENIED means
     *      that it is blocked to access the ISIM application because of PIN code.
     *      User should wait for the following STATE_READY event and read again.
     * ERROR_CARD_ERROR means
     *      a general UICC card error. User can resume the operation with STATE_INIT.
     * ERROR_CARD_REMOVED means
     *      that the UICC card is removed. User can resume the operation with STATE_INIT.
     * ERROR_NO_ISIM_APPLICATION means
     *      that there is no ISIM application in UICC.
     */
    enum
    {
        ERROR_INIT_FAILED = 0,
        ERROR_START_FAILED,
        ERROR_READ_IMPI_FAILED,
        ERROR_READ_IMPU_FAILED,
        ERROR_READ_DOMAIN_FAILED,
        ERROR_READ_IST_FAILED,
        ERROR_READ_PCSCF_ADDRESS_FAILED,
        ERROR_READ_DENIED,
        ERROR_CARD_ERROR,
        ERROR_CARD_REMOVED,
        ERROR_REFRESH_REG_FAILED,
        ERROR_INTERFACE_CHANNEL_ERROR,
        ERROR_REFRESH_ERROR,
        ERROR_NO_ISIM_APPLICATION
    };

    /// Exception result of GET operation
    enum
    {
        RESULT_NO_RECORDS = (-2)
    };
};

#endif
