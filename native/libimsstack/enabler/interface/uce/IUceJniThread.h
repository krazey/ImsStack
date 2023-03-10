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
#ifndef INTERFACE_JNI_UCE_SERVICE_THREAD_H_
#define INTERFACE_JNI_UCE_SERVICE_THREAD_H_

#include "ImsTypeDef.h"
#include "IJniEnablerThread.h"

class IUceTerminatedReason;

class IUceJniThread : public IJniEnablerThread
{
public:
    virtual ~IUceJniThread() {}

    /**
     * Notify the application that the device is disconnected to the IMS network.
     */
    virtual IMS_BOOL NotifyImsDeregistered() = 0;

    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param registeredService is type of registered service.
     * @param registeredNetwork is type of registered network.
     */
    virtual IMS_BOOL NotifyImsRegistered(
            IN IMS_UINT32 registeredService, IN IMS_SINT32 registeredNetwork) = 0;

    /**
     * Notify the application that the device is received response to publish.
     *
     * @param key is a value that can identify the request.
     * @param responseCode The SIP response code sent from the network for the operation
     *      token specified
     * @param capability The value converted from pidf xml. When the publish is successful,
     *      this is set to the latest capability.
     * @param reason The optional reason response from the network. If there is a reason header
     *      included in the response, that should take precedence over the reason provided in the
     *      status line. If the network provided no reason with the sip code, the string should be
     *      empty.
     * @param reasonHeaderCause The “cause” parameter of the “reason” header
     *      included in the SIP message.
     * @param reasonHeaderText The “text” parameter of the “reason” header
     *      included in the SIP message.
     * @param etag is a value of the SIP IF Match header when received response to publish.
     * @param needToRetry is a value that this request need to retry.
     */
    virtual IMS_BOOL PublishResponseInd(IMS_UINT32 key, IMS_UINT32 responseCode,
            IMS_UINT32 capability, AString reason, IMS_UINT32 reasonHeaderCause,
            AString reasonHeaderText, AString etag, IMS_UINT32 needToRetry) = 0;

    /**
     * Notify the application that the publish status is change.
     *
     * @param capability The value converted from pidf xml. When the publish is successful,
     *      this is set to the latest capability.
     * @param responseCode The SIP response code sent from the network for the operation
     *      token specified
     * @param reason The optional reason response from the network. If there is a reason header
     *      included in the response, that should take precedence over the reason provided in the
     *      status line. If the network provided no reason with the sip code, the string should be
     *      empty.
     * @param reasonHeaderCause The “cause” parameter of the “reason” header
     *      included in the SIP message.
     * @param reasonHeaderText The “text” parameter of the “reason” header
     *      included in the SIP message.
     * @param etag is a value of the SIP IF Match header when received response to publish.
     * @param needToRetry is a value that this request need to retry.
     */
    virtual IMS_BOOL PublishUpdatedInd(IMS_UINT32 capability, IMS_SINT32 responseCode,
            AString reason, IMS_SINT32 reasonHeaderCause, AString reasonHeaderText, AString eTag,
            IMS_UINT32 needToRetry) = 0;

    /**
     * Notify the application that the publish request is failed due to internal error
     *
     * @param key is a value that can identify the request.
     * @param commandError is the internal error value when the request occurred.
     */
    virtual IMS_BOOL PublishErrorInd(IMS_UINT32 key, IMS_UINT32 commandError) = 0;

    /**
     * Notify the application that the publish is disconnected
     *
     */
    virtual IMS_BOOL UnPublishedInd() = 0;

    /**
     * Notify the application that the device is received response to subscribe.
     *
     * @param key is a value that can identify the request.
     * @param responseCode The SIP response code sent from the network for the operation
     *      token specified
     * @param reason The optional reason response from the network. If there is a reason header
     *      included in the response, that should take precedence over the reason provided in the
     *      status line. If the network provided no reason with the sip code, the string should be
     *      empty.
     * @param reasonHeaderCause The “cause” parameter of the “reason” header
     *      included in the SIP message.
     * @param reasonHeaderText The “text” parameter of the “reason” header
     *      included in the SIP message.
     */
    virtual IMS_BOOL SubscribeResponseInd(IMS_UINT32 key, IMS_SINT32 responseCode, AString reason,
            IMS_SINT32 reasonHeaderCause, AString reasonHeaderText) = 0;

    /**
     * Notify the application that the device is received notify method
     *
     * @param key is a value that can identify the request.
     * @param count is number of pidfXmls.
     * @param pidfXmls The list of the PIDF XML data for the contact URIs that it subscribed for.
     */
    virtual IMS_BOOL NotifyInd(IMS_UINT32 key, IMS_UINT32 count, ImsList<AString> pidfXmls) = 0;

    /**
     * Notify the application that subscribe request is failed due to internal error
     *
     * @param key is a value that can identify the request.
     * @param commandError is the internal error value when the request occurred.
     */
    virtual IMS_BOOL SubscribeErrorInd(IMS_UINT32 key, IMS_UINT32 commandError) = 0;

    /**
     * Notify the application that Notify message has a terminated resource
     *
     * @param key is a value that can identify the request.
     * @param count is number of terminateContacts.
     * @param terminateContacts The contact URIs which have been terminated.
     */
    virtual IMS_BOOL SubscribeResourceTerminatedInd(
            IMS_UINT32 key, IMS_UINT32 count, ImsList<IUceTerminatedReason*> terminateContacts) = 0;

    /**
     * Notify the application that subscribe request is terminated
     *
     * @param key is a value that can identify the request.
     * @param reason The reason for the request being unable to process.
     * @param retryAfterMillsecond The time in milliseconds the requesting application should
     *      wait before retrying, if non-zero.
     */
    virtual IMS_BOOL SubscribeTerminatedInd(
            IMS_UINT32 key, AString reason, IMS_UINT32 retryAfterMillsecond) = 0;

    /**
     * Notify the application that the device is received response to options.
     *
     * @param key is a value that can identify the request.
     * @param responseCode The SIP response code sent from the network for the operation
     *      token specified
     * @param reason The optional SIP response reason sent by the network.
     *      If none was sent, this should be an empty string.
     * @param theirCaps The contact's UCE capabilities associated with the capability request.
     */
    virtual IMS_BOOL OptionsResponseInd(
            IMS_UINT32 key, IMS_UINT32 responseCode, AString reason, IMS_UINT32 theirCaps) = 0;

    /**
     * Notify the application that options request is failed due to internal error
     *
     * @param key is a value that can identify the request.
     * @param commandError is the internal error value when the request occurred.
     */
    virtual IMS_BOOL OptionsErrorInd(IMS_UINT32 key, IMS_UINT32 commandError) = 0;

    /**
     * Notify the application that options request is received
     *
     * @param key is a value that can identify the request.
     * @param remote The URI associated with the remote contact that is requesting capabilities.
     * @param remoteCaps The remote contact's capability information. The capability
     *      information is in the format defined in RCC.07 section 2.6.1.3.
     */
    virtual IMS_BOOL OptionsReceivedInd(IMS_UINT32 key, AString remote, IMS_UINT32 remoteCaps) = 0;

    /**
     * Notify the application that the device is refreshed to the IMS network.
     *
     * @param registeredNetwork is type of registered network.
     */
    virtual IMS_BOOL NotifyImsRegiRefreshed(IN IMS_SINT32 registeredNetwork) = 0;

    /**
     * Notify the application that the device network changed
     *
     * @param changedNetwork is type of changed network.
     */
    virtual IMS_BOOL NotifyNetworkChanged(IN IMS_SINT32 changedNetwork) = 0;
};

#endif
