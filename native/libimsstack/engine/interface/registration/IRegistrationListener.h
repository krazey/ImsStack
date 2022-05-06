#ifndef _INTERFACE_REGISTRATION_LISTENER_H_
#define _INTERFACE_REGISTRATION_LISTENER_H_

#include "ByteArray.h"

class IRegistration;

/**
 * @brief This class provides a listener interface for IMS registration.
 */
class IRegistrationListener
{
public:
    /**
     * @brief Notifies the application when the registration is challenged by S-CSCF.
     *
     * When the method is invoked, the application can determine whether it sends
     * the second SIP request or not.\n
     * If bResponseToChallenge is IMS_FALSE, the engine do not any action and maintain
     * all the information of this registration, but the state of this registration is
     * in the IRegistration#STATE_TERMINATED.
     *
     * @param nAlgorithm Type of algorithm; refer to #Credential in Credential.h
     * @param bResponseToChallenge Flag to determine whether the second SIP request
     *                             (w/ Authorization header) should be sent or not;
     *                             The default value is IMS_TRUE
     */
    virtual void Registration_AuthenticationChallenged(
            IN IMS_SINT32 nAlgorithm, OUT IMS_BOOL& bResponseToChallenge) = 0;

    /**
     * @brief Notifies the application when the AKA response is received from ISIM application.
     *
     * This method is invoked only and only if the algorithm is AKAv1-MD5.
     *
     * @param nResult Result of AKA authentication; refer to IMS_AKA class in Credential.h
     * @param objIK Integrity key value; it is valid if nResult is only RESULT_OK
     * @param objCK Ciphering key value; it is valid if nResult is only RESULT_OK
     * @param bResultOfSA Result of security association
     */
    virtual void Registration_NotifyAKAResponse(IN IMS_SINT32 nResult, IN CONST ByteArray& objIK,
            IN CONST ByteArray& objCK, OUT IMS_BOOL& bResultOfSA) = 0;

    /**
     * @brief Notifies the application when the registration's refresh timer is expired.
     *
     * The refresh of registration will be handled by the application.
     *
     * @param bDoImplicitRefresh flag to indicate that refresh request is sent from engine or not
     */
    virtual void Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;

    /**
     * @brief Notifies the application when the initial registration is done successfully.
     */
    virtual void Registration_Started() = 0;

    /**
     * @brief Notifies the application when the initial registration is failed.
     *
     * @param nReason Reason code (#IRegistration)
     */
    virtual void Registration_StartFailed(IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Notifies the application when the re-registration(modify) is done successfully.
     */
    virtual void Registration_Updated() = 0;

    /**
     * @brief Notifies the application when the re-registration is failed.
     *
     * @param nReason Reason code (#IRegistration)
     */
    virtual void Registration_UpdateFailed(IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Notifies the application that the registration is successfully removed.
     */
    virtual void Registration_Removed() = 0;

    /**
     * @brief Notifies the application when the de-registration is completed.
     *
     * @param nReason Reason code (#IRegistration)
     */
    virtual void Registration_Terminated(IN IMS_SINT32 nReason) = 0;
};

#endif  // _INTERFACE_REGISTRATION_LISTENER_H_
