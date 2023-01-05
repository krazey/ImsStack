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
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include "IAppConfig.h"
#include "IMediaConfig.h"
#include "ISipConfig.h"
#include "ISubscriberConfig.h"

/**
 * @brief This class provides the configuration interface for each IMS services.
 */
class Configuration
{
private:
    inline Configuration() {}

public:
    inline ~Configuration() {}

    Configuration(IN const Configuration&) = delete;
    Configuration& operator=(IN const Configuration&) = delete;

public:
    /**
     * @brief Returns all application identifiers for the local point.
     *
     * An empty string array will be returned if no application identifiers could be retrieved.
     *
     * @param nSlotId slot id to be retrieved
     * @return A string array containing all application identifiers for the local point
     */
    AStringArray GetLocalAppIds(IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Returns the registry for an IMS application with the specified application
     *        identifier.
     *
     * @param strAppId an IMS application identifier
     * @param nSlotId slot id to be retrieved
     * @return Pointer to IAppConfig or null
     * @remark It replaces the below method:\n
     *           ImsRegistry* GetRegistry(IN const AString &strAppId) const;
     */
    const IAppConfig* GetAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Returns IMS_TRUE if there is a registry for the IMS application
     *        with the specified application identity, else IMS_FALSE.
     *
     * @param strAppId an IMS application identifier
     * @param nSlotId slot id to be retrieved
     * @return If AppConfig exists, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     * @remark It replaces the below method:\n
     *           IMS_BOOL HasRegistry(IN const AString &strAppId) const;
     */
    IMS_BOOL HasAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Removes the registry for the IMS application and deletes the binding to the owning
     *        IMS (JAVA) application.
     *
     * @param strAppId an IMS application identifier
     * @param nSlotId slot id to be retrieved
     * @remark This applies to both static and dynamic installations.\n
     *         If this method is invoked, the local endpoint will no longer be able to create new
     *         services with the application identity specified by the strAppId argument,
     *         this does not affect services that are already created.\n
     *         It replaces the below method:\n
     *             void RemoveRegistry(IN const AString &strAppId);
     */
    void RemoveAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId);

    /**
     * @brief Sets the registry for an IMS application and binds it to a parent UI(JAVA)
     *        application.
     *
     * Any previous registry and binding for that IMS application is deleted
     * (including all properties), this does not affect services that are already created.
     *
     * @param strAppId an IMS application identifier
     * @param nSlotId slot id to be retrieved
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @remark It replaces the below method:\n
     *           void SetRegistry(IN const AString &strAppId, IN const AString &strClassName,
     *               IN const ImsRegistry &objRegistry);
     */
    IMS_RESULT SetAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId);

    /**
     * @brief Sets the registry for an IMS application and binds it to a parent UI(JAVA)
     *        application.
     *
     * Any previous registry and binding for that IMS application is deleted
     * (including all properties), this does not affect services that are already created.
     *
     * @param strAppId an IMS application identifier
     * @param strClassName the name of JAVA application that the IMS application is bound to
     * @param objRegistry an array of arrays, specifying key and value(s)
     * @param nSlotId slot id to be retrieved
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @remark The strClassName argument identifies a JAVA application according to the particular
     *         JAVA Runtime Environment.\n
     *         For a MIDP2-based implementation, the class name MUST be registered
     *         with the Midlet-<n> attribute.\n
     *         \n
     *         The objRegistry argument identifies a registry as an unordered array of IMS
     *         properties.\n
     *         The general pattern of the objRegistry argument is as follows.\n
     *         {\n
     *             { "Stream", "Media Types" },\n
     *             { "Framed", "MIME Content Types", "max-size" },\n
     *             { "Basic", "MIME Content Types" },\n
     *             { "Event", "Event package names" },\n
     *             { "CoreService", "ServiceId", "Zero or one IARI", "Zero or more ICSIs",
     *                "Zero or more Feature Tags" },\n
     *             { "Qos", "ServiceId", "MIME Content Types", "Flowspec send",
     *                "Flowspec receive" },\n
     *             { "Reg", "ServiceId", "Header 1", ..., "Header n" },\n
     *             { "Write", "Write access SIP headers" },\n
     *             { "Read", "Read access SIP headers" },\n
     *             { "Cap", "SectorId", "MessageType", "SDP field 1", ..., "SDP field n" },\n
     *             { "Mprof", "ServiceId", "Media profile" },\n
     *             { "Connection", "ServiceId" }\n
     *         }\n
     *         \n
     *         It replaces the below method:\n
     *           void SetRegistry(IN const AString &strAppId, IN const AString &strClassName,
     *               IN const ImsRegistry &objRegistry);
     */
    IMS_RESULT SetAppConfig(IN const AString& strAppId, IN const AString& strClassName,
            IN const ImsRegistry& objRegistry, IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns a Configuration that enables dynamic installation of IMS applications.
     * @return An instance of Configuration that enables dynamic installation of IMS applications.
     */
    static Configuration* GetInstance();

    /**
     * @brief Returns a configuration information of a media profile.
     *
     * @param nSlotId slot id to be retrieved
     * @return Pointer to IMediaConfig or null
     * @note IMS EXTENSION METHOD
     */
    const IMediaConfig* GetMediaConfig(IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Returns a configuration information of a SIP signalling.
     *
     * @param nSlotId slot id to be retrieved
     * @return Pointer to ISipConfig or null
     * @note IMS EXTENSION METHOD
     */
    const ISipConfig* GetSipConfig(IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Returns a configuration information of an IMS subscriber which has a public user
     *        identities, home domain name, and so on.
     *
     * @param nSlotId slot id to be retrieved
     * @param strId an identifier to find out a subscriber's configuration\n
     *              Pre-defined identifiers:\n
     *              - "default" : default subscriber info.
     *              - "fake" : fake subscriber info. (for fake registration)
     * @return Pointer to ISubscriberConfig or null
     * @note IMS EXTENSION METHOD
     */
    const ISubscriberConfig* GetSubscriberConfig(
            IN IMS_SINT32 nSlotId, IN const AString& strId = AString::ConstNull()) const;

    /**
     * @brief Returns the trace modules. Refer to ITraceTag.h.
     *
     * @param nSlotId slot id to be retrieved
     * @return the trace modules as bitmask
     * @note IMS EXTENSION METHOD
     */
    IMS_UINT32 GetTraceModule(IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Returns the trace option. Refer to ITraceOption.h.
     *
     * @param nSlotId slot id to be retrieved
     * @return the trace options as bitmask
     * @note IMS EXTENSION METHOD
     */
    IMS_UINT32 GetTraceOption(IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Checks if the server info. MUST be hidden in the release mode or not.
     *
     * @param nSlotId slot id to be checked
     * @return If the service information MUST be hidden in the logs, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     * @note IMS EXTENSION METHOD
     */
    IMS_BOOL IsServerInfoHiddenInLog(IN IMS_SINT32 nSlotId) const;

    /**
     * @brief Initializes the configurations which needs to be initialized for each slot.
     *
     * This method is invoked from enabler thread for specified slot.
     *
     * @param nSlotId slot id to be initialized
     * @note IMS EXTENSION METHOD, SystemApi
     */
    void InitConfigs(IN IMS_SINT32 nSlotId);

    /**
     * @brief Refreshes all the configurations which are managed by ConfigurationManager
     *        for specified slot.
     *
     * This method is invoked from enabler thread for specified slot.
     *
     * @param nSlotId slot id to be refreshed
     * @note IMS EXTENSION METHOD, SystemApi
     */
    void RefreshConfigs(IN IMS_SINT32 nSlotId);
};

#endif
