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

package com.android.imsstack.system;

import com.android.imsstack.core.config.CarrierConfig;

import java.io.FileDescriptor;

public interface SystemCallInterface {
    /** Result code of execution with no error. */
    int RESULT_OK = 1;
    /** Result code of execution for operation failure. */
    int RESULT_FAIL = 0;
    /** Result code of execution with a specific error. */
    int RESULT_ERROR = -1;

    //// ConfigInterface {
    /**
     * Returns the carrier configuration.
     *
     * @return A CarrierConfig object.
     */
    CarrierConfig getCarrierConfig();
    //// }

    //// IpSecInterface {
    /**
     * Add an IpSec security association parameter.
     *
     * @param param The IpSec SA parameter.
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
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
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
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
    ////}

    //// SIM interface {
    /**
     * Returns the ISIM state as a string.
     *
     * @return The ISIM state string.
     */
    String getIsimState();

    /**
     * Reads the file attributes of the specified ISIM record.
     *
     * @param fileId The file id to be read.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int readIsimFileAttributes(int fileId);

    /**
     * Reads the value of the specified ISIM record.
     *
     * @param fileId The file id to be read.
     * @param index The index of the record for the given file.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int readIsimRecord(int fileId, int index);

    /**
     * Returns the response of ISIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int requestIsimAuthentication(String nonce, long owner);

    /**
     * Returns the response of USIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int requestUsimAuthentication(String nonce, long owner);
    ////}

    /**
     * Returns the flag specifying whether the IMS voice call(vops) is supported on the LTE network.
     *
     * @return true if the IMS voice call is supported, false otherwise.
     */
    boolean isImsVoiceCallSupported();

    /**
     * Updates the native service ready state.
     *
     * @param serviceReady A flag specifying whether the native service is ready or not.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int updateNativeServiceReady(boolean serviceReady);

    /**
     * Returns the best location information from the last known location.
     *
     * @param category The location category. Possible values are:
     *                 {@link LocationInterface#LOCATION_CATEGORY_ALL},
     *                 {@link LocationInterface#LOCATION_CATEGORY_POSITION_N_COUNTRY},
     *                 {@link LocationInterface#LOCATION_CATEGORY_POSITION}
     */
    String[] getLastKnownLocation(int category);

    /**
     * Starts listening the location information with the given interval.
     *
     * @param updateIntervalSec The location update interval in seconds.
     */
    void startListeningForLocation(int updateIntervalSec);

    /**
     * Stops listening the location information.
     */
    void stopListeningForLocation();

    /**
     * Starts an instant location update (one-time update).
     */
    void startInstantLocationUpdate();
}
