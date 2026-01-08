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
#ifndef INTERFACE_REGISTRATION_H_
#define INTERFACE_REGISTRATION_H_

#include "IRegBase.h"
#include "IRegContact.h"
#include "IRegParameter.h"
#include "SipAddress.h"

class Credential;
class IRegBindingStateListener;
class IRegSubscription;
class IRegUserIdentityNotifier;
class IRegistrationListener;
class SipProfile;

/**
 * @brief This class provides an interface to access/control IMS registration.
 */
class IRegistration : public IRegBase
{
protected:
    ~IRegistration() override = default;

public:
    /**
     * @brief Creates a registration binding information with the specified service.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @return If the registration binding is successfully created, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL CreateBinding(IN const AString& strAppId, IN const AString& strServiceId) = 0;

    /**
     * @brief Destroys a registration binding information with the specified service.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     */
    virtual void DestroyBinding(IN const AString& strAppId, IN const AString& strServiceId) = 0;

    /**
     * @brief Creates a Contact information with a registration duration.
     *
     * @param objIpAddr IP address of device
     * @param nPort port number of device; listening channel for an incoming SIP message
     * @param nExpiresPolicy policy of expiration value\n
     *                       #POLICY_EXPIRES_CONFIG\n
     *                       #POLICY_EXPIRES_APP\n
     *                       #NO_EXPIRES
     * @param nExpiresValue expiration value (duration of this binding)
     * @return Pointer to IRegContact or null.
     */
    virtual IRegContact* CreateContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nExpiresPolicy = POLICY_EXPIRES_CONFIG,
            IN IMS_UINT32 nExpiresValue = DEFAULT_EXPIRES) = 0;

    /**
     * @brief Destroys the specified Contact.
     *
     * @param piContact Contact to be destroyed
     */
    virtual void DestroyContact(IN IRegContact* piContact) = 0;

    /**
     * @brief Destroys the Contact with the specified IP & Port.
     *
     * @param objIpAddr IP address of Contact
     * @param nPort port number of Contact
     */
    virtual void DestroyContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Checks if the specified registration equals or not.
     *
     * @param piReg pointer to IRegistration
     * @return If the specified registration equals, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL Equals(IN const IRegistration* piReg) const = 0;

    /**
     * @brief Returns the current credential.
     *
     * This method will be returned a valid credential informatioin
     * when resubmitted message is just sent or its registration is in ACTIVE state.
     *
     * @return Pointer of current credential or null.
     */
    virtual const Credential* GetCredential() const = 0;

    /**
     * @brief Returns the public user identity to be registered.
     *
     * It has been used or will be used in From & To header in REGISTER request.
     *
     * @return (Temporary) public user identity.
     */
    virtual const SipAddress& GetAor() const = 0;

    /**
     * @brief Returns the network provisioned user identities.
     *
     * The topmost one is authorized & registered.
     *
     * @return Network provisioned user identities.
     */
    virtual const AStringArray& GetAssociatedUris() const = 0;

    /**
     * @brief Returns the authorized (topmost in P-Associated-URI) public user identity
     *        as SipAddress format.
     *
     * @return Authorized public user identity.
     */
    virtual const SipAddress& GetAuthorizedAor() const = 0;

    /**
     * @brief Returns all the Contacts for this registration.
     *
     * @return List of pointer to IRegContact.
     */
    virtual ImsList<IRegContact*> GetAllContacts() const = 0;

    /**
     * @brief Returns the Contact which are matched with IP & Port.
     *
     * @param objIpAddr IP address to be compared
     * @param nPort port number to be compared
     * @return Pointer to IRegContact.
     */
    virtual IRegContact* GetContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort) const = 0;

    /**
     * @brief Returns the preferred contact (highest 'q' value) among the contacts.
     *
     * @return Pointer to IRegContact.
     */
    virtual IRegContact* GetPreferredContact() const = 0;

    /**
     * @brief Returns the registration parameter container which is maintained during the whole
     *        registration (add/refresh/modify/remove).
     *
     * @return Pointer to IRegParameter.
     */
    virtual IRegParameter* GetParameter() const = 0;

    /**
     * @brief Returns the public IP address of the device.
     *
     * It can be obtained from Via header when receiving SIP response of
     * the first REGISTER request.\n
     * It's available only if the device recognizes that it is located behind NAT device.
     *
     * @return Reference to IpAddress.
     * @note NAT_REQ_UE_PUBLIC_IP
     */
    virtual const IpAddress& GetPublicIpAddress() const = 0;

    /**
     * @brief Returns the service route set of this registration.
     *
     * The returned value will include the preloaded route set & Service-Route header field.
     *
     * @return Route set which is obtained during registration.
     */
    virtual const AStringArray& GetServiceRoutes() const = 0;

    /**
     * @brief Returns the instance of SIP profile of this registration.
     *
     * @return Pointer to SipProfile.
     * @note MULTI_REG_SIP_PROFILE
     */
    virtual SipProfile* GetSipProfile() const = 0;

    /**
     * @brief Returns the state of this registration.
     *
     * @return State of registration.\n
     *         #STATE_CREATED\n
     *         #STATE_INIT\n
     *         #STATE_ACTIVE\n
     *         #STATE_TERMINATED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Checks if the UA is located behind a NAT or not.
     *
     * @return If UA is located behind a NAT / FW, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsBehindNat() const = 0;

    /**
     * @brief Checks if the binding (contact) is update or not.
     *
     * If the return value is TRUE, the application MUST update the registration state.
     *
     * @return If the binding (contact) is updated, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsBindingsUpdated() const = 0;

    /**
     * @brief Checks if the binding is updating or not.
     *
     * If the return value is TRUE, the REGISTER transaction is already started
     * and not completed.
     *
     * @return If the registration is in PENDING state, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsBindingsUpdating() const = 0;

    /**
     * @brief Checks if the registration requires the network interworking or not.
     *
     * If it is not required the network interworking, it will emulate the registration operation.
     * In this case, IRegistration will only support the partial capabilities comparing
     * with the normal registration.
     *
     * @return If the registration is required the network interworking, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsNetworkInterworkingRequired() const = 0;

    /**
     * @brief Checks if the UA is located within the trust domain.
     *
     * @return If UA is located within the trust domain, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsWithinTrustDomain() const = 0;

    /**
     * @brief Sends the REGISTER request (initial/re-/de-/refresh) to the IMS network.
     *
     * @param nExpires Proposal expires value; if not set, the default expires will be used
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Register(IN IMS_SINT32 nExpires = (-1)) = 0;

    /**
     * @brief Sends the de-REGISTER request (with 'expires=0' or 'Expires: 0') to the IMS network.
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Deregister() = 0;

    /**
     * @brief Removes the active bindings forcingly without IMS de-REG.
     *
     * It just changes the logical state of the Services which are already connected.\n
     * Each services receive TERMINATED event callback.\n
     * (But, RegBinding remains in INIT state after this method is invoked)
     *
     * NOTE: It can be invoked to restore the IMS-REG after the device obtains
     * the data connection again with the same IP address.
     */
    virtual void RemoveActiveBindingsForcingly() = 0;

    /**
     * @brief Aborts all the operation and restores the state as an initial state.
     */
    virtual void Restore() = 0;

    /**
     * @brief Restores the active binding if the registration is not explicitly terminated
     *        after the IMS-REG is installed.
     *
     * If the active binding does not exist, it returns non-IMS_SUCCESS value.
     *
     * NOTE: This method can be invoked when the device lost the data connection
     * and after some moment, it obtained the data connection again
     * and received the same IP adddress which was used in the previous IMS-REG procedure.
     *
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     * @note REG_RESTORATION_FOR_ACTIVE_BINDING
     */
    virtual IMS_RESULT RestoreActiveBindings() = 0;

    /**
     * @brief Sets the flag to indicate if the restoration of active binding will be enabled
     *        or not.
     *
     * @param bEnabled flag to indicate if the feature is enabled or not
     * @note REG_RESTORATION_FOR_ACTIVE_BINDING
     */
    virtual void SetActiveBindingsRestorationUsage(IN IMS_BOOL bEnabled) = 0;

    /**
     * @brief Sets the public user identity to be registered.
     *
     * @param objAor public user identity to be registered
     * @param strSubsId an identifier for SubscriberConfig
     * @note MULTI_SUBS
     */
    virtual void SetAor(
            IN const SipAddress& objAor, IN const AString& strSubsId = AString::ConstNull()) = 0;

    /**
     * @brief Sets the listener for this registration.
     *
     * @param piListener Listener interface to be set
     */
    virtual void SetListener(IN IRegistrationListener* piListener) = 0;

    /**
     * @brief Sets the refresh policy for the registration refresh.
     *
     * This policy will be applied from when the refresh operation of the current
     * registration is invoked.
     *
     * @param nPolicy Refresh policy to be set\n
     *                #REFRESH_POLICY_NO_REFRESH\n
     *                #REFRESH_POLICY_SPEC\n
     *                #REFRESH_POLICY_REMAIN_TIME\n
     *                #REFRESH_POLICY_RATIO
     * @param nCriteriaInterval Criteria interval to determine the refresh interval
     * @param nValueEorLt Interval value when the refresh duration is equal or less
     *                    than the criteria interval
     * @param nValueGt Interval value when the refresh duration is greater
                       than the criteria interval
     */
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) = 0;

    /**
     * @brief Sets the SIP profile for this registration.
     *
     * This profile will be used to provision the specific SIP configurations
     * for each registrations when the multiple registration is required
     * and the configuration for each registrations is different.
     *
     * @param pProfile Pointer to SipProfile
     * @note MULTI_REG_SIP_PROFILE
     */
    virtual void SetSipProfile(IN SipProfile* pProfile) = 0;

    /**
     * @brief Sets the listener to monitor the changes of the service's state.
     *
     * @param piListener Pointer to IRegBindingStateListener
     */
    virtual void SetBindingStateListener(IN IRegBindingStateListener* piListener) = 0;

    /**
     * @brief Sets the configuration option for which UE is registered within the trust domain.
     *
     * It will override a default configuration to identify whether UE is within trust domain.
     *
     * @param bWithinTrustDomain Flag to identify whether UE is within trust domain
     */
    virtual void SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain) = 0;

    /**
     * @brief Sets the notifier for reordering the network provisioned user identities.
     *
     * @param piUserIdNotifier Notifier interface to be set
     */
    virtual void SetUserIdentityNotifier(IN IRegUserIdentityNotifier* piUserIdNotifier) = 0;

    /**
     * @brief Sets the user-info field for Contact header in this registration.
     *
     * It will be applied in all the outgoing SIP request and response.\n
     * If user-info is null string, the UE will use the Contact header which is used
     * in IMS registration.\n
     * If user-info is empty string, the UE will not contain the user-info field in
     * the Contact header and it will select this Contact header when sending
     * SIP request/response.
     *
     * @param strUserInfo User-info field value to be set
     */
    virtual void SetUserInfoForContactHeader(IN const AString& strUserInfo) = 0;

    /**
     * @brief Creates the subscription with 'reg' event package for this registration.
     *
     * @param pResourceUri Pointer to the resource uri to subscribe
     * @return Pointer to IRegSubscription.
     */
    virtual IRegSubscription* CreateSubscription(IN SipAddress* pResourceUri = IMS_NULL) = 0;

public:
    /// Default expiration time value
    enum
    {
        DEFAULT_EXPIRES = 3600
    };

    /// Policy for expiration value
    enum
    {
        /// Default policy; Set from the configuration
        POLICY_EXPIRES_CONFIG = 0,
        /// Set by the application explicitly
        POLICY_EXPIRES_APP,
        /// No expires in REGISTER request
        NO_EXPIRES
    };

    /// Policy for registration refresh
    enum
    {
        /// No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        /// Default policy; Select the refresh time according to 3GPP spec.\n
        ///    nCriteriaInterval : Criteria value for the refresh duration\n
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100; default 50)\n
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        /// Set the remain time before it is expired\n
        ///    nCriteriaInterval : Criteria value for the refresh duration\n
        ///    nValueEorLT : Interval value when the refresh duration is equal or less
        ///              than the criteria interval\n
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        /// Set the ratio before it is expired\n
        ///    nCriteriaInterval : Criteria value for the refresh duration\n
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100)\n
        ///    nValueGT : Ratio when the refresh duration is greater
        ///              than the criteria interval (1 ~ 100)\n
        /// Ex) Expires: 3600, Ratio: 10\n
        ///        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };

    /// Reason codes which can be occurred on the registration procedure
    enum
    {
        REASON_NONE = 0,
        REASON_STATUS_CODE,
        REASON_NO_EXPIRES,
        /// Server's abnormal behavior : no matched contacts
        REASON_NO_ACTIVE_BINDINGS,
        REASON_INTERNAL_ERROR,
        REASON_TRANSACTION_TIMEOUT,
        REASON_REFRESH_TIMEOUT,
        REASON_REFRESH_INTERNAL_ERROR,
        REASON_CLIENT_SOCKET_ERROR,
        REASON_SERVER_SOCKET_ERROR
    };

    /// State of registration
    enum
    {
        /// When the registration object is created
        STATE_CREATED = 0,
        /// When the registration is launched
        STATE_INIT,
        /// When receiving the successful response to the initial registration
        /// or when receiving the notification to 'reg' event package
        /// with the attribute, 'state=active'
        STATE_ACTIVE,
        /// When receiving the successful response to the de-registration
        /// or when receiving the notification to 'reg' event package
        /// with the attribute, 'state=terminated'
        STATE_TERMINATED
    };
};

#endif
