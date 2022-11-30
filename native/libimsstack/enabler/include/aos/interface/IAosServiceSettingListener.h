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
#ifndef INTERFACE_AOS_SERVICE_SETTING_LISTENER_H_
#define INTERFACE_AOS_SERVICE_SETTING_LISTENER_H_

enum class RoamingPreferredVoiceNetwork;
enum class ServiceSetting;

class IAosServiceSettingListener
{
public:
    virtual ~IAosServiceSettingListener(){};

    /**
     * Called to notify the change of airplane setting.
     * Called by AosService (Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void ServiceSetting_AirplaneChanged(IN IMS_BOOL bIsOn) = 0;

    /**
     * Called to notify the change of data roaming setting.
     * Called by AosService (Java)
     *
     * @param bIsAllowed {@code IMS_TRUE} if allowed, {@code IMS_FALSE} if not allowed.
     */
    virtual void ServiceSetting_DataRoamingChanged(IN IMS_BOOL bIsAllowed) = 0;

    /**
     * Called to notify the change of mobile data setting.
     * Called by AosService (Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void ServiceSetting_MobileDataChanged(IN IMS_BOOL bIsOn) = 0;

    /**
     * Called to notify the change of roaming preferred voice network.
     * Called by AosService (Java)
     *
     * @param eState is type of RoamingPreferredVoiceNetwork.
     * @see enum class {@link #RoamingPreferredVoiceNetwork}
     */
    virtual void ServiceSetting_RoamingPreferredVoiceNetworkChanged(
            IN RoamingPreferredVoiceNetwork eState) = 0;

    /**
     * Called to notify the change of service setting.
     * Called by AosService (Java)
     *
     * @param nState is type of ServiceSetting.
     * @param nServiceBits : bits an integer. Valid values are the following :
     * M_APP_UC(0x00000001), M_APP_SMS(0x00000002), M_APP_VT(0x00000004), M_SERVICE_ALL(0xFFFFFFFF)
     * @see class {@link #ServiceSetting}
     * @see class {@link #IUIMS}
     */
    virtual void ServiceSetting_ServiceChanged(
            IN ServiceSetting eState, IN IMS_UINT32 nServiceBits) = 0;

    /**
     * Called to notify the change of TTY setting.
     * Called by AosService (Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void ServiceSetting_TtyChanged(IN IMS_BOOL bIsOn) = 0;

    /**
     * Called to notify the change of video setting.
     * Called by AosService (Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void ServiceSetting_VideoChanged(IN IMS_BOOL bIsOn) = 0;

    /**
     * Called to notify the change of VoLTE setting.
     * Called by AosService (Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void ServiceSetting_VolteChanged(IN IMS_BOOL bIsOn) = 0;

    /**
     * Called to notify the change of WFC setting.
     * Called by AosService (Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void ServiceSetting_WfcChanged(IN IMS_BOOL bIsOn) = 0;
};

/**
 * Roaming Preferred Voice Network
 */
enum class RoamingPreferredVoiceNetwork
{
    CELLULAR = 0,
    WIFI = 1
};

/**
 * Service setting
 */
enum class ServiceSetting
{
    OFF = 0,
    ON = 1,
    PRESENTITY = 2
};

class AosServiceSettingListener : public IAosServiceSettingListener
{
public:
    inline void ServiceSetting_AirplaneChanged(IN IMS_BOOL /*bIsOn*/) override{};
    inline void ServiceSetting_DataRoamingChanged(IN IMS_BOOL /*bIsAllowed*/) override{};
    inline void ServiceSetting_MobileDataChanged(IN IMS_BOOL /*bIsOn*/) override{};
    inline void ServiceSetting_RoamingPreferredVoiceNetworkChanged(
            IN RoamingPreferredVoiceNetwork /*eState*/) override{};
    inline void ServiceSetting_ServiceChanged(
            IN ServiceSetting /*eState*/, IN IMS_UINT32 /*nServiceBits*/) override{};
    inline void ServiceSetting_TtyChanged(IN IMS_BOOL /*bIsOn*/) override{};
    inline void ServiceSetting_VideoChanged(IN IMS_BOOL /*bIsOn*/) override{};
    inline void ServiceSetting_VolteChanged(IN IMS_BOOL /*bIsOn*/) override{};
    inline void ServiceSetting_WfcChanged(IN IMS_BOOL /*bIsOn*/) override{};
};

#endif  // INTERFACE_AOS_SERVICE_SETTING_LISTENER_H_