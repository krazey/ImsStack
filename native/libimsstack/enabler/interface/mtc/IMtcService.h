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

#ifndef INTERFACE_MTC_SERVICE_H_
#define INTERFACE_MTC_SERVICE_H_

#include "INativeEnabler.h"
#include "ImsTypeDef.h"

class AString;
class ICoreService;
class IJniMtcServiceThread;
class IMtcAosConnector;
class IMtcAosStateListener;
class ISrvccStateListener;
enum class ServiceStatus;
enum class ServiceType;
enum class SrvccState;
enum class TbcwStatus;

class IMtcService : public INativeEnabler
{
public:
    virtual ~IMtcService() {}

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ServiceType GetServiceType() const = 0;

    /**
     * @brief Adds
     *
     * @param piListener
     */
    virtual void AddAosStateListener(IN IMtcAosStateListener* piListener) = 0;

    /**
     * @brief Removes
     *
     * @param piListener
     */
    virtual void RemoveAosStateListener(IN IMtcAosStateListener* piListener) = 0;

    /**
     * @brief Adds
     *
     * @param piListener
     */
    virtual void AddSrvccStateListener(IN ISrvccStateListener* piListener) = 0;

    /**
     * @brief Removes
     *
     * @param piListener
     */
    virtual void RemoveSrvccStateListener(IN ISrvccStateListener* piListener) = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsActive() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEmergency() const = 0;

    /**
     * @brief Checks if the UE is currently in NR.
     *
     * It reads the device's current radio state. So it could be different from the RAT which this
     * service is in.
     *
     * @return True if it's in NR.
     */
    virtual IMS_BOOL IsNr() const = 0;

    /**
     * @brief Checks if the UE is currently in LTE and has combined attach type.
     *
     * @return True if it's in LTE and has combined attach type.
     */
    virtual IMS_BOOL IsEpsCombinedAttach() const = 0;

    /**
     * @brief Checks if the UE is currently in a roaming network.
     *
     * @return True if it's in a roaming network.
     */
    virtual IMS_BOOL IsRoaming() const = 0;

    /**
     * @brief Checks if the UE is currently registered through an ePDG.
     *
     * @return True if it's in ePDG.
     */
    virtual IMS_BOOL IsWlanIpCanType() const = 0;

    /**
     * @brief Checks if CSFB is available based on the UE's network status and carrier
     *        configuration.
     *
     * This method evaluates the UE's network conditions, including NR, EPS Attach Type, Roaming,
     * and WiFi status, against the CSFB block conditions specified in the carrier configuration
     * {@link ConfigVoice#KEY_CSFB_BLOCK_CONDITION_INT_ARRAY}.
     *
     * @return True if CSFB is available based on the network status and carrier configuration;
     *         False otherwise.
     */
    virtual IMS_BOOL IsCsfbAvailable() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ServiceStatus GetOldStatus() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ServiceStatus GetStatus() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ICoreService* GetICoreService() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcAosConnector* GetAosConnector() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IJniMtcServiceThread* GetJniServiceThread() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual SrvccState GetSrvccState() const = 0;

    /**
     * @brief Updates
     *
     * @param eState
     */
    virtual void UpdateSrvccState(IN SrvccState eState) = 0;

    /**
     * @brief Sets
     *
     * @param bEnabled
     */
    virtual void SetTerminalBasedCallWaiting(IN IMS_BOOL bEnabled) = 0;

    /**
     * @brief Opens
     *
     */
    virtual void OpenEmergencyService(IN ServiceType eServiceType) = 0;

    /**
     * @brief Stops
     *
     */
    virtual void StopEmergencyService() = 0;

    /**
     * @brief Sends
     *
     * @return
     */
    virtual void ProcessTestCommand(
            IN IMS_SINT32 nCommand, IN IMS_SINT32 nWParam, IN IMS_SINT32 nLParam) = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual TbcwStatus GetTbcwStatus() const = 0;
};

enum class ServiceStatus
{
    SERVICE_IDLE,
    SERVICE_ACTIVE,
    SERVICE_SUSPENDED,
};

enum class ServiceType
{
    UNKNOWN = 0,
    NORMAL = 1 << 0,
    EMERGENCY = 1 << 1,
};

enum class TbcwStatus
{
    UNPROVISIONED,
    PROVISIONED_ENABLED,
    PROVISIONED_DISABLED,
};

#endif
