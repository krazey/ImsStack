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
#ifndef INTERFACE_SIP_CONNECTION_FACTORY_H_
#define INTERFACE_SIP_CONNECTION_FACTORY_H_

#include "SipMethod.h"
#include "base/IMethod.h"

class ISipClientConnection;
class ISipConnectionFactoryListener;
class ISipDialog;
class ISipServerConnection;
class SipAddress;

/**
 * @brief This class provides an interface to use J180 interface layer directly
 *        for SIP signalling interworking.
 *
 * @see ISipClientConnection, ISipServerConnection, ISipDialog
 */
class ISipConnectionFactory : public IMethod
{
protected:
    ~ISipConnectionFactory() override = default;

public:
    /**
     * @brief Creates a new SIP client connection (for new standalone tranction).
     *
     * @param objMethod SIP method
     * @param pFrom SIP address for From header\n
     *              If null, the anonymous SIP address will be set
     * @param pTo SIP address for To header\n
                  If null, the anonymous SIP address will be set
     * @return Pointer to ISipClientConnection.
     */
    virtual ISipClientConnection* CreateClientConnection(IN const SipMethod& objMethod,
            IN const SipAddress* pFrom, IN const SipAddress* pTo) = 0;

    /**
     * @brief Creates a new SIP client connection inside of an SIP dialog.
     *
     * @param piDialog Pointer to ISipDialog
     * @param objMethod SIP method
     * @return Pointer to ISipClientConnection.
     */
    virtual ISipClientConnection* CreateClientConnection(
            IN ISipDialog* piDialog, IN const SipMethod& objMethod) = 0;

    /**
     * @brief Creates a SIP response message for SIP server connection.
     *
     * @param piSsc Pointer to ISipServerConnection
     * @param nStatusCode SIP status code
     * @param strPhrase SIP reason phrase
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL CreateResponse(IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
            IN const AString& strPhrase = AString::ConstNull()) = 0;

    /**
     * @brief Gets the initial incoming SIP server connection.
     *
     * @return Pointer to ISipServerConnection.
     */
    virtual ISipServerConnection* GetNewServerConnection() = 0;

    /**
     * @brief Sets the dialog information to receive a new SIP server connection
     *        inside of SIP dialog.
     *
     * @param piDialog Pointer to ISipDialog
     */
    virtual void SetDialog(IN ISipDialog* piDialog) = 0;

    /**
     * @brief Sets the listener to receive a new SIP server connection inside of SIP dialog.
     *
     * @param piListener Pointer to ISipConnectionFactoryListener
     */
    virtual void SetListener(IN ISipConnectionFactoryListener* piListener) = 0;

    /**
     * @brief Sets the INVITE server transaction to handle an incoming CANCEL request.
     *
     * @param piSsc Pointer to ISipServerConnection (INVITE transaction)
     */
    virtual void SetSscForCancel(IN ISipServerConnection* piSsc) = 0;
};

#endif
