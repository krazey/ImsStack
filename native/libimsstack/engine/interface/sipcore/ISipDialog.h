#ifndef _INTERFACE_SIP_DIALOG_H_
#define _INTERFACE_SIP_DIALOG_H_

#include "ISipObject.h"
#include "ISipClientConnection.h"

class ISIPHeader;

/**
 * @brief This class provides an interface to access/control a SIP dialog.
 *
 * The ISIPDialog can be retrieved from a ISIPConnection object,
 * when it is available (at earliest after provisional 101-199 response with local or remote tag).
 * \nThree SIP requests can open a dialog : INVITE, SUBSCRIBE - NOTIFY, REFER - NOTIFY.
 *
 * @see ISIPConnection, ISIPClientConnection
 */
class ISIPDialog
    : public ISIPObject
{
public:
    /**
     * @brief Clones this SIP dialog.
     *
     * @return Pointer to ISIPDialog.
     */
    virtual ISIPDialog* Clone() const = 0;

    /**
     * @brief Compares if the specified ISIPDialog equals or not.
     *
     * @param piDialog Pointer to ISIPDialog to be compared
     * @return If the specified SIP dialog equals to this dialog, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL Equals(IN CONST ISIPDialog *piDialog) = 0;

    /**
     * @brief Returns the ID (Call-ID + Local Tag + Remote Tag) of the SIP dialog.
     *
     * It returns null if the dialog is terminated.
     *
     * @return Textual format of the dialog ID.
     */
    virtual AString GetDialogID() = 0;

    /**
     * @brief Returns a new ISIPClientConnection in this dialog.
     *
     * The returned ISIPClientConnection will be in INITIALIZED state.\n
     * The object is initialized with the given method and default headers.
     *
     * The following headers will be set by the method:
     *     - To
     *     - From
     *     - CSeq
     *     - Call-ID
     *     - Via
     *     - Route
     *     - Contact
     *     - Max-Forwards
     *
     * @param strMethod SIP method name to be created
     * @return Pointer to ISIPClientConnection object with preset method and headers.
     */
    virtual ISIPClientConnection* GetNewClientConnection(IN CONST AString &strMethod) = 0;

    /**
     * @brief Returns the state of the SIP dialog.
     *
     * @return SIP dialog state.\n
     *         #STATE_INIT\n
     *         #STATE_TERMINATED\n
     *         #STATE_EARLY\n
     *         #STATE_CONFIRMED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Compares if the given ISIPConnection belongs to this dialog or not.
     *
     * @param piSC Pointer to ISIPConnection object to be compared
     * @return If the given SIP transaction belongs to the current dialog, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsSameDialog(IN CONST ISIPConnection *piSC) = 0;

    /**
     * @brief Returns the component of SIP dialog (call-id, local-tag, remote-tag).
     *
     * @param nType Type of dialog component\n
     *              #COMPONENT_CALL_ID\n
     *              #COMPONENT_LOCAL_TAG\n
     *              #COMPONENT_REMOTE_TAG
     * @return Dialog component to the specified type.
     */
    virtual AString GetComponent(IN IMS_SINT32 nType) const = 0;

    /**
     * @brief Returns the ID (Call-ID + Local Tag + Remote Tag) of the SIP dialog.
     *
     * It returns non-null dialog id even though the dialog was terminated.
     *
     * @return Textual format of the dialog ID.
     * @note BYE_REQUEST_ON_DIALOG_TERMINATED
     */
    virtual AString GetDialogIDEx() = 0;

    /**
     * @brief Returns the local contact address of this dialog.
     *
     * @return Pointer to ISIPHeader.
     */
    virtual const ISIPHeader* GetContactHeader() const = 0;

    /**
     * @brief Sets the contact header parameter on the early or confirmed state.
     *
     * @param strParameter Header parameter
     * @param nOperation Operation (0: add or 1: remove)
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @note CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
     */
    virtual IMS_RESULT SetContactParameter(IN CONST AString &strParameter,
            IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */) = 0;

    /**
     * @brief Terminates the SIP dialog usage explicitly.
     *
     * Use Case) remove the dialog usage for "refer" event package subscription.
     */
    virtual void TerminateDialogUsage() = 0;

public:
    /// State of SIP dialog
    enum
    {
        /// When SIP dialog is created
        STATE_INIT = (-1),
        /// When SIP dialog is terminated
        STATE_TERMINATED = 0,
        /// When receiving a provisional response which contains to-tag parameter in To header
        STATE_EARLY = 1,
        /// When receiving a successful response or NOTIFY request
        STATE_CONFIRMED = 2
    };

    /// Component of SIP dialog (call-id, local-tag, remote-tag)
    enum
    {
        /// Call-ID header of SIP message
        COMPONENT_CALL_ID = 1,
        /// UAC: From header of SIP message, UAS: To header of SIP message
        COMPONENT_LOCAL_TAG,
        /// UAC: To header of SIP message, UAS: From header of SIP message
        COMPONENT_REMOTE_TAG
    };
};

#endif // _INTERFACE_SIP_DIALOG_H_
