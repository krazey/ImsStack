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
package com.android.imsstack.core.agents;

import com.android.imsstack.system.IpSecSaParameter;

import java.io.FileDescriptor;

public interface IpSecInterface extends IAgent {
    /**
     * Add an IpSec security association parameter.
     *
     * @param param The IpSec SA parameter.
     * @return One of {@link SystemCallInterface#RESULT_ERROR}
     *         or {@link SystemCallInterface#RESULT_OK}.
     */
    int addIpSecSaParameter(IpSecSaParameter param);

    /**
     * Remove an IpSec security association parameter with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     */
    void removeIpSecSaParameter(int ipSecId);

    /**
     * Apply the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     * @return One of {@link SystemCallInterface#RESULT_ERROR}
     *         or {@link SystemCallInterface#RESULT_OK}.
     */
    int applyIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd);

    /**
     * Remove the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     */
    void removeIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd);
}
