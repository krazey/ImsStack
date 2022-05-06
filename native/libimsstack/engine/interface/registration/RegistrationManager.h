#ifndef _REGISTRATION_MANAGER_H_
#define _REGISTRATION_MANAGER_H_

#include "IRegistration.h"

class IRegistrationListener;
class RegistrationManagerPrivate;

/**
 * @brief This class provides an interface to create/destroy IMS registrations.
 */
class RegistrationManager
{
private:
    RegistrationManager();
    RegistrationManager(IN CONST RegistrationManager& objRHS);
    RegistrationManager& operator=(IN CONST RegistrationManager& objRHS);

public:
    ~RegistrationManager();

public:
    /**
     * @brief Creates Registration object for IMS registration.
     *
     * @param nFlowId Registration flow id
     * @param strAOR Address-Of-Record(public user identity to be registered)
     * @param bFake Flag to indicate whether it's used for fake or not
     * @param strSubsId Subscriber configuration's identifier as a string
     * @param pSIPProfile SIP profile for this registration
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    IMS_BOOL CreateRegistration(IN IMS_UINT32 nFlowId, IN CONST AString& strAOR,
            IN IMS_BOOL bFake = IMS_FALSE, IN CONST AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pSIPProfile = IMS_NULL);

    /**
     * @brief Creates Registration object for IMS registration.
     *
     * @param nFlowId Registration flow id
     * @param strAOR Address-Of-Record(public user identity to be registered)
     * @param bFake Flag to indicate whether it's used for fake or not
     * @param strSubsId Subscriber configuration's identifier as a string
     * @param pSIPProfile SIP profile for this registration
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    IMS_BOOL CreateRegistration(IN IMS_UINT32 nFlowId, IN CONST SipAddress& objAOR,
            IN IMS_BOOL bFake = IMS_FALSE, IN CONST AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pSIPProfile = IMS_NULL);

    /**
     * @brief Destroys the specified Registration object.
     *
     * @param piReg Pointer to IRegistration to be destroyed
     * @param bByForce Flag to indicate whether it's aborted locally or not\n
     *                 If it sets to IMS_FALSE, the registration transaction will be preserved
     *                 if possible
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    void DestroyRegistration(IN IRegistration* piReg, IN IMS_BOOL bByForce = IMS_FALSE);

    /**
     * @brief Checks if "reg" event package subscription is supported or not.
     *
     * @param nSlotId Slot id
     * @param pSIPProfile SIP profile to be checked
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    IMS_BOOL IsRegSubscriptionSupported(
            IN IMS_SINT32 nSlotId = IMS_SLOT_0, IN SipProfile* pSIPProfile = IMS_NULL) const;

    /**
     * @brief Gets the Registration object with the specified slot-id and flow id.
     *
     * @param nSlotId Slot id
     * @param nFlowId Registration flow id
     */
    IRegistration* GetRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const;

    /**
     * @brief Gets RegistrationManager's instance.
     */
    static RegistrationManager* GetInstance();

private:
    RegistrationManagerPrivate* pRegMngrP;
};

#endif  // _REGISTRATION_MANAGER_H_
