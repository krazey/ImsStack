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
#ifndef INTERFACE_UCE_SERVICE_H_
#define INTERFACE_UCE_SERVICE_H_

#include "INativeEnabler.h"

class AString;

class IUceJni : public INativeEnabler
{
public:
    /**
     * UCE(Java) -> UCeApp(Native)
     */

    /**
     * Receives publish request from the Framework.
     *
     * @param key is a value that can identify the request.
     * @param extended is a value that set expires header for Non volte case.
     * @param capability The value converted from pidf xml. When the publish is successful,
     *      this is set to the latest capability.
     * @param pidfXml The XML PIDF document containing the capabilities of this device to be sent
     *      to the carrier’s presence server.
     * @param eTag is a value of the SIP IF Match header when sending the publish requeset.
     * @return Returns IMS_TRUE when the request is successfully passed.
     *      Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL SendPublishCmd(IMS_UINT32 key, IMS_UINT32 extended, IMS_UINT32 capability,
            const AString& pidfXml, const AString& eTag) = 0;

    /**
     * Receives single subscribe request from the Framework.
     *
     * @param key is a value that can identify the request.
     * @param user The target of capability query.
     * @return Returns IMS_TRUE when the request is successfully passed.
     *      Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL SendSingleSubscribeCmd(IMS_UINT32 key, const AString& user) = 0;

    /**
     * Receives list subscribe request from the Framework.
     *
     * @param key is a value that can identify the request.
     * @param user The target of capability query.
     * @return Returns IMS_TRUE when the request is successfully passed.
     *      Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL SendListSubscribeCmd(IMS_UINT32 key, const ImsList<AString>& userList) = 0;

    /**
     * Receives options request from the Framework.
     *
     * @param key is a value that can identify the request.
     * @param myCaps is my capability.
     * @param remoteUri The target of capability query.
     * @return Returns IMS_TRUE when the request is successfully passed.
     *      Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL SendOptionsCmd(
            IMS_UINT32 key, IMS_UINT32 myCaps, const AString& remoteUri) = 0;

    /**
     * Receives a response for the received options from the Framework.
     *
     * @param key is a value that can identify the request.
     * @param responseCode is that response code for the request received
     * @param reason A non-null String containing the reason associated with the SIP code.
     * @param myCaps is my capability.
     * @return Returns IMS_TRUE when the request is successfully passed.
     *      Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL SendOptionsRespCmd(
            IMS_UINT32 key, IMS_SINT32 responseCode, const AString& reason, IMS_UINT32 myCaps) = 0;

    /**
     * Receives a request to get th current ims registration status.
     * @return Returns IMS_TRUE when the request is successfully passed.
     *      Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL ImsRegistrationCheck() = 0;
};
#endif  // INTERFACE_UCE_SERVICE_H_
