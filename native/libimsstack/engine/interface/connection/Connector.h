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
#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include "AString.h"

class IConnection;

/**
 * @brief This class is a factory class to create IConnection interface.
 */
class Connector
{
public:
    Connector() = delete;

public:
    /**
     * @brief Creates IConnection interface using a specified name.
     *
     * @param strName specify the connection scheme and parameters to be opened
     * @return An IConnection instance or null
     * @see IConnection
     */
    static IConnection* Open(IN const AString& strName);

    /**
     * @brief Creates IConnection interface using a specified scheme, target, and parameters.
     *
     * @param strScheme a scheme to identify IConnection category
     *                  (i.e. "imscore"/"sip")
     * @param strTarget a target information to be opened. It depends on the scheme.
     * @param strParams any parameters for this scheme.
     *                  Multiple parameters are separated by semi-colon
     * @return An IConnection instance or null
     * @see IConnection
     */
    static IConnection* Open(
            IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams);
};

#endif
