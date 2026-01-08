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
#ifndef INTERFACE_CORE_SERVICE_H_
#define INTERFACE_CORE_SERVICE_H_

#include "IService.h"

class ICapabilities;
class ICoreServiceListener;
class IDirectCoreServiceListener;
class IPageMessage;
class IPublication;
class IReference;
class ISession;
class ISipConnectionFactory;
class ISubscription;

/**
 * @brief This class provides an interface to give the application the possibility to
 *        call remote peers over the IMS network.
 *
 * @see IServiceMethod, ICoreServiceListener
 */
class ICoreService : public IService
{
protected:
    ~ICoreService() override = default;

public:
    /**
     * @brief Creates a ICapabilities with strFrom as sender, addressed to strTo.
     *
     * @param strFrom The sender SIP or TEL URI with an optional display name\n
     *                If null, it is assumed to be the local user identity of the IService.
     * @param strTo The recipient SIP or TEL URI with an optional display name
     * @return Pointer to new ICapabilities.
     */
    virtual ICapabilities* CreateCapabilities(
            IN const AString& strFrom, IN const AString& strTo) = 0;

    /**
     * @brief Creates a IPageMessage with strFrom as sender, addressed to strTo.
     *
     * @param strFrom The sender SIP or TEL URI with an optional display name\n
     *                If null, it is assumed to be the local user identity of the IService.
     * @param strTo The recipient SIP or TEL URI with an optional display name
     *              to send a IPageMessage to
     * @return Pointer to new IPageMessage.
     */
    virtual IPageMessage* CreatePageMessage(IN const AString& strFrom, IN const AString& strTo) = 0;

    /**
     * @brief Creates a IPublication for an event package with strFrom as sender
     *        and strTo as the user identity to publish event state on.
     *
     * The event package MUST be set with the SetRegistry() method in the Configuration class.
     *
     * @param strFrom The sender SIP or TEL URI with an optional display name\n
     *                If null, it is assumed to be the local user identity of the IService.
     * @param strTo The recipient SIP or TEL URI with an optional display name
     *              to publish event state information on\n
     *              If null, it is assumed to be the local user identity of the IService.
     * @param strEvent The event package to publish event state information on
     * @return Pointer to new IPublication.
     */
    virtual IPublication* CreatePublication(
            IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent) = 0;

    /**
     * @brief Creates a IReference with strFrom as sender, addressed to strTo and strReferTo
     *        as the URI to refer to.
     *
     * @param strFrom The sender SIP or TEL URI with an optional display name\n
     *                If null, it is assumed to be the local user identity of the IService.
     * @param strTo The recipient SIP or TEL URI with an optional display name
     * @param strReferTo Any URI, not just a user id
     * @param strReferMethod The reference method to be used by the reference request,
     *                       e.g. "INVITE", "BYE" or null
     * @return Pointer to new IReference.
     */
    virtual IReference* CreateReference(IN const AString& strFrom, IN const AString& strTo,
            IN const AString& strReferTo, IN const AString& strReferMethod) = 0;

    /**
     * @brief Creates a ISession with strFrom as sender, with strTo as recipient.
     *
     * @param strFrom The sender SIP or TEL URI with an optional display name\n
     *                If the argument is null, then it is by default set to the preferred
     *                public user identity from this core service.\n
     *                If it has a domain name of "anonymous.invalid", then the originator
     *                will be kept anonymous according to [RFC3323] and [RFC3325].
     * @param strTo The recipient SIP or TEL URI with an optional display name
     * @return Pointer to new ISession.
     */
    virtual ISession* CreateSession(IN const AString& strFrom, IN const AString& strTo) = 0;

    /**
     * @brief Creates a ISubscription for an event package with strFrom as sender
     *        and strTo as the user identity to subscribe event state on.
     *
     * The event package MUST be set with the SetRegistry() method in the Configuration class.
     *
     * @param strFrom The sender SIP or TEL URI with an optional display name\n
     *                If null, it is assumed to be the local user identity of the IServce.
     * @param strTo The recipient SIP or TEL URI with an optional display name
     *              to subscribe state information on\n
     *              If null, it is assumed to be the local user identity of the IService.
     * @param strEvent The event package to subscribe state information on
     * @return Pointer to new ISubscription.
     */
    virtual ISubscription* CreateSubscription(
            IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent) = 0;

    /**
     * @brief Returns the display name and public user identity for the ICoreService.
     *
     * This is either the value of the "userId=" parameter of the "imscore:" locator used
     * to create the core service, or default value of the device.
     *
     * @return A public user identity, possibly with a display name.
     */
    virtual AString GetLocalUserId() const = 0;

    /**
     * @brief Sets a listener for this ICoreService, replacing any previous ICoreServiceListener.
     *
     * A null removes any existing listener.
     *
     * @param piListener Listener to be set, or null
     */
    virtual void SetListener(IN ICoreServiceListener* piListener) = 0;

    /**
     * @brief Creates the SIP connection factory.
     *
     * @return Pointer to ISipConnectionFactory
     */
    virtual ISipConnectionFactory* CreateSipConnectionFactory() = 0;

    /**
     * @brief Sets a listener for this ICoreService, replacing any previous
     *        IDirectCoreServiceListener.
     *
     * A null removes any existing listener.
     *
     * @param piListener Listener to be set, or null
     */
    virtual void SetDirectListener(IN IDirectCoreServiceListener* piListener) = 0;

public:
    /// Result of the direct transaction handling
    enum
    {
        /// The transaction will be handled by the owner of direct listener.\n
        /// The owner has a responsibility of the resource release to SipServerConnection.
        RESULT_DIRECT_TXN_HANDLED = 0,
        /// The transaction is not handled by the owner of direct listener.\n
        /// The invoker should release the SIP server connection calling close() method.
        RESULT_DIRECT_TXN_NOT_HANDLED = 1,
        /// The transaction is handled by the owner of direct listener.\n
        /// But, it also should be handled by the default listener.\n
        /// It's usage is only for SIP message modification after receiving the message.
        RESULT_DIRECT_TXN_BYPASS = 2
    };
};

#endif
