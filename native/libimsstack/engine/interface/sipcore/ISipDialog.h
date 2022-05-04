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
#ifndef INTERFACE_SIP_DIALOG_H_
#define INTERFACE_SIP_DIALOG_H_

#include "ISipClientConnection.h"
#include "ISipObject.h"

class ISipHeader;

/**
 * @brief This class provides an interface to access/control a SIP dialog.
 *
 * The ISipDialog can be retrieved from a ISipConnection object,
 * when it is available (at earliest after provisional 101-199 response with local or remote tag).
 * \nThree SIP requests can open a dialog : INVITE, SUBSCRIBE - NOTIFY, REFER - NOTIFY.
 *
 * @see ISipConnection, ISipClientConnection
 */
class ISipDialog :
        public ISipObject
{
public:
    /**
     * @brief Clones this SIP dialog.
     *
     * @return Pointer to ISipDialog.
     */
    virtual ISipDialog* Clone() const = 0;

    /**
     * @brief Compares if the specified ISipDialog equals or not.
     *
     * @param piDialog Pointer to ISipDialog to be compared
     * @return If the specified SIP dialog equals to this dialog, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL Equals(IN const ISipDialog* piDialog) = 0;

    /**
     * @brief Returns the ID (Call-ID + Local Tag + Remote Tag) of the SIP dialog.
     *
     * It returns null if the dialog is terminated.
     *
     * @return Textual format of the dialog ID.
     */
    virtual AString GetDialogId() = 0;

    /**
     * @brief Returns a new ISipClientConnection in this dialog.
     *
     * The returned ISipClientConnection will be in INITIALIZED state.\n
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
     * @return Pointer to ISipClientConnection object with preset method and headers.
     */
    virtual ISipClientConnection* GetNewClientConnection(IN const AString& strMethod) = 0;

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
     * @brief Compares if the given ISipConnection belongs to this dialog or not.
     *
     * @param piSc Pointer to ISipConnection object to be compared
     * @return If the given SIP transaction belongs to the current dialog, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsSameDialog(IN const ISipConnection* piSc) = 0;

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
    virtual AString GetDialogIdEx() = 0;

    /**
     * @brief Returns the local contact address of this dialog.
     *
     * @return Pointer to ISipHeader.
     */
    virtual const ISipHeader* GetContactHeader() const = 0;

    /**
     * @brief Sets the contact header parameter on the early or confirmed state.
     *
     * @param strParameter Header parameter
     * @param nOperation Operation (0: add or 1: remove)
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @note CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
     */
    virtual IMS_RESULT SetContactParameter(IN const AString& strParameter,
            IN IMS_SINT32 nOperation = 0/*(0: ADD, 1: REMOVE)*/) = 0;

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

#endif
