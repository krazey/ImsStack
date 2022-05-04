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
#ifndef INTERFACE_SIP_KEEP_ALIVE_HELPER_H_
#define INTERFACE_SIP_KEEP_ALIVE_HELPER_H_

#include "IPAddress.h"
#include "ISipObject.h"
#include "Sip.h"

class ISipKeepAliveHelperListener;

/**
 * @brief This class provides a helper interface to control SIP keep-alive operations.
 */
class ISipKeepAliveHelper :
        public ISipObject
{
public:
    /**
     * @brief Sends the keep-alive packet to the specified destination.
     *
     * @param objPacket Keep-alive packet to be sent
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SendPacket(IN const ByteArray& objPacket) = 0;

    /**
     * @brief Sets the keep-alive helper listener to get the pong message from the server.
     *
     * @param piListener Keep-alive helper listener
     */
    virtual void SetListener(IN ISipKeepAliveHelperListener* piListener) = 0;

     /**
      * @brief Sets the destination transport information to transmit the keep-alive packet.
      *
      * @param objIpAddr Destination IP address
      * @param nPort Destination port number
      */
    virtual void SetTransportTupleD(IN const IPAddress& objIpAddr, IN IMS_SINT32 nPort) = 0;

    /**
      * @brief Sets the source transport information to transmit the keep-alive packet.
      *
      * @param objIpAddr Source IP address
      * @param nPort Source port number (In case of TCP with no-ipsec, it MUST be 0)
      * @param nProtocol Transport procotol to be sent\n
      *                  #Sip#TRANSPORT_UDP\n
      *                  #Sip#TRANSPORT_TCP
      */
    virtual void SetTransportTupleS(IN const IPAddress& objIpAddr, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nProtocol = Sip::TRANSPORT_UDP) = 0;
};

#endif
