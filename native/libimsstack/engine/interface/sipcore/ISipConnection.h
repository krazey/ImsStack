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
#ifndef INTERFACE_SIP_CONNECTION_H_
#define INTERFACE_SIP_CONNECTION_H_

#include "AString.h"
#include "IConnection.h"
#include "SipTimerValues.h"

class ByteArray;
class ISipDialog;
class ISipErrorListener;
class ISipMessage;
class SipMethod;
class SipProfile;

/**
 * @brief This class provides a base interface for SIP transactions.
 *
 * ISipConnection holds the common properties and methods for subinterfaces
 * ISipClientConnection and ISipServerConnection.
 *
 * @see IConnection, ISipDialog, ISipErrorListener
 */
class ISipConnection : public IConnection
{
public:
    /**
     * @brief Adds a header to the SIP message.
     *
     * If multiple header fields exist, the header value is added topmost of this type of headers.
     *
     * Some SIP headers can occur only once in a message, for these, it is recommended that
     * SetHeader() is used instead of AddHeader().\n
     * The header value string may contain a single value or multiple values as a comma-separated
     * list.\n
     * Note that this feature should not be used for headers (like Authorization header) which use
     * comma as parameter separator instead of semicolon.\n
     * The method works atomically, that is if the strValue argument of this method is a list of
     * comma-separated values then the method should either add all of them or none of them.
     *
     * @param strName Name of the header, either full or compact form
     * @param strValue Value of the header. Empty string or null means a header with no value
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Returns the current SIP dialog.
     *
     * This is available when the ISipConnection belongs to a created ISipDialog,
     * which is in EARLY or CONFIRMED state.
     *
     * The ISipDialog returned from the ISipConnection defines always the peer-to-peer association
     * created by the latest sent or received request or response.\n
     * This method returns null if a terminating error response (3xx - 6xx) is received or sent on
     * the transaction or the transaction is terminated.\n
     * The following rules apply when this method is called on a ISipServerConnection instance:
     *     - If the received request is CANCEL, then the method returns null since the dialog is
     *      in EARLY state and the CANCEL request does not relate to the dialog.
     *     - If the request is ACK, then the method returns the dialog object if it is available.
     *
     * @return Pointer to ISipDialog if this transaction belongs to a dialog.
     *         Otherwise, returns null.
     */
    virtual ISipDialog* GetDialog() const = 0;

    /**
     * @brief Gets the header field value of the specified header type.
     *
     * This method returns null or empty string if the value was set to be null or empty string.\n
     * It returns null if the current message does not have such a header or the header is
     * for other reason not available (e.g. message is not initialized, the transaction is
     * terminated or the implementation does not allow this header to be read).
     *
     * @param strName Name of the header, either full or compact form
     * @param nIndex Position of the header field
     * @return Header field value at an index.
     */
    virtual AString GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex /* = 0*/) = 0;

    /**
     * @brief Gets the header field value(s) of the specified header type.
     *
     * It returns null if the current message does not have such a header or the header is
     * for other reason not available (e.g. message is not initialized, the transaction is
     * terminated or the implementation does not allow this header to be read).
     *
     * @param strName Name of the header, either full or compact form
     * @return List of header field value(s).
     */
    virtual IMSList<AString> GetHeaders(IN const AString& strName) = 0;

    /**
     * @brief Gets the SIP method.
     *
     * It is available when a message has been initialized or received.\n
     * This method returns null if the SIP method is not available
     * (e.g. message is not initialized, sthe transaction is terminated).
     *
     * @return SIP method object if available.
     */
    virtual const SipMethod& GetMethod() const = 0;

    /**
     * @brief Gets SIP reason phrase.
     *
     * It is available when ISipClientConnection is in PROCEEDING, UNAUTHORIZED, or COMPLETED
     * state or when ISipServerConnection is in INITIALIZED state.\n
     * It returns null if the reason phrase is not available (e.g. message is not initialized
     * or the transaction is terminated).\n
     * It returns an empty string if the reason phrase was set with null or empty string
     * in SetReasonPhrase().
     *
     * @return SIP reason phrase if available.
     */
    virtual const AString& GetReasonPhrase() const = 0;

    /**
     * @brief Gets a Request-URI.
     *
     * If this method is supported, it is available when ISipClientConnection is in INITIALIZED
     * state or when ISipServerConnection is in REQUEST_RECEIVED state.\n
     * It is built from the original URI given in Connector::open(...).\n
     * It is not mandated that this method be supported, an implementation may return null
     * in any state.\n
     * This method returns null if the Request-URI is not available (e.g. message is not initialzed
     * or the transaction is terminated) or getting the Request-URI is not supported.
     *
     * @return A Request-URI if available.
     */
    virtual const AString& GetRequestUri() const = 0;

    /**
     * @brief Gets a SIP response status code.
     *
     * It is available when ISipClientConnection is in PROCEEDING, UNAUTHORIZED or COMPLETED
     * state or when ISipServerConnection is in INITIALIZED state.
     *
     * @return A value between 1xx and 6xx if available. Otherwise, returns zero.
     */
    virtual IMS_SINT32 GetStatusCode() const = 0;

    /**
     * @brief Removes header from the SIP message.
     *
     * If multiple header field values exist, the topmost is removed.\n
     * If the named header is not found, this method does nothing.\n
     * This method only removes one header value even if the header values are stored
     * in a comma-separated list.
     *
     * @param strName Name of the header to be removed, either in full or compact form
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveHeader(IN const AString& strName) = 0;

    /**
     * @brief Sends the SIP message.
     *
     * Errors during the send operation can be handled with the ISipErrorListener mechanism.\n
     * In case of ISipServerConnection, it is possible to resend 2xx responses in COMPLETED
     * state, by calling directly Send().
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Send() = 0;

    /**
     * @brief Sets the listener for error notifications.
     *
     * Applications that want to receive notification about a failure of an asynchronous
     * Send() operation MUST implement the ISipErrorListener interface and register it
     * with a transaction using this method.
     *
     * Only one listener can be set at any time, if a listener is already set, it will be
     * overwritten.\n Setting listener to NULL will remove the current listener.
     *
     * @param piListener Pointer to ISipErrorListener object.
     */
    virtual void SetErrorListener(IN ISipErrorListener* piListener) = 0;

    /**
     * @brief Sets header value in SIP message.
     *
     * If the header does not exist, it will be added to the message,
     * otherwise the existing header is overwritten.\n
     * If multiple header field values exist, the topmost is overwritten.
     *
     * The strValue argument of the method may contain a list of comma-separated header values.\n
     * Note that this feature should not be used for headers (like Authorization header) which use
     * comma as parameter separator instead of semicolon. If there exist headers of the same type,
     * then only the first (topmost) one will be overwritten, indifferently of the number of header
     * values in the comma-separated list.\n
     * The method works atomically, that is if the strValue argument of the method is a list of
     * comma-separated values, then the method should either set all of them or none of them.
     *
     * @param strName Name of the header, either in full or compact form
     * @param strValue Value of the header\n
     *                 Empty string or null means a header with no value
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Gets the message body part from the current SIP message.
     *
     * @return Reference to ByteArray object.
     */
    virtual const ByteArray& GetContent() const = 0;

    /**
     * @brief Sets the message body part to the current SIP message.
     *
     * @param objContent Contents of the message body part
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetContent(IN const ByteArray& objContent) = 0;

    /**
     * @brief Gets the header field count of the specified header type.
     *
     * It returns 0 if the current message does not have such a header or the header is for other
     * reason not available (e.g. message is not initialized, the transaction is terminated
     * or the implementation does not allow this header to be read).
     *
     * @param strName Name of the header, either full or compact form
     * @return Count of header field.
     */
    virtual IMS_SINT32 GetHeaderCount(IN const AString& strName) const = 0;

    /**
     * @brief Gets the current SIP message instance of SIP transaction.
     *
     * @return Pointer to SIPMessage object.
     */
    virtual ISipMessage* GetMessage() const = 0;

    /**
     * @brief Gets the slot-id for this SIP connection.
     *
     * @return Slot id of this SIP connection.
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

    /**
     * @brief Sets the SIP profile for specific configuration of this SIP connection.
     *
     * @param pProfile SIP profile for this SIP connection
     * @note MULTI_REG_SIP_PROFILE
     */
    virtual void SetSipProfile(IN SipProfile* pProfile) = 0;

    /**
     * @brief Sets the SIP transaction timer values.
     *
     * @param objTv Transaction timer values to be set
     */
    virtual void SetTransactionTimerValues(IN const SipTimerValues& objTv) = 0;
};

#endif
