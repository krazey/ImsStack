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

/**
 * This interface provides the operations for USAT functions to interwork with the UICC.
 */
public interface UsatInterface {
    /**
     * Checks if the specified service is available or not in the USIM service table.
     *
     * @param serviceType The service type to be evaluated. Valid values are
     *                    {@link #SERVICE_CALL_CONTROL},
     *                    {@link #SERVICE_MO_SMS_CONTROL},
     *                    {@link #SERVICE_DATA_DOWNLOAD_VIA_SMS_PP},
     *                    {@link #SERVICE_MEDIA_TYPE_SUPPORT}.
     *
     * @return true if the specified service is available or false otherwise.
     */
    boolean isServiceAvailable(@Usat.ServiceType int serviceType);

    /**
     * Creates a USAT command for call control.
     *
     * @param ccType The call control type. Valid values are
     *               {@link #CALL_CONTROL_TYPE_MO_CALL},
     *               {@link #CALL_CONTROL_TYPE_SS},
     *               {@link #CALL_CONTROL_TYPE_USSD}.
     * @param dialedString The dialed string - address or SS string or USSD string.
     * @param networkType The network type that the call is performed. Valid values are
     *                    {@link TelephonyManager#NETWORK_TYPE_UNKNOWN},
     *                    {@link TelephonyManager#NETWORK_TYPE_NR},
     *                    {@link TelephonyManager#NETWORK_TYPE_LTE},
     *                    {@link TelephonyManager#NETWORK_TYPE_HSPA},
     *                    {@link TelephonyManager#NETWORK_TYPE_GPRS},
     *                    {@link TelephonyManager#NETWORK_TYPE_IWLAN}.
     * @param mediaType The media type. Valid values are
     *                  {@link #MEDIA_TYPE_NONE},
     *                  {@link #MEDIA_TYPE_VOICE},
     *                  {@link #MEDIA_TYPE_VIDEO}.
     * @param listener The listener for receiving the response.
     *
     * @return A USAT command for call control.
     */
    Usat.CallControlCommand createCallControlCommand(
            @Usat.CallControlType int ccType, String dialedString, int networkType,
            @Usat.MediaType int mediaType, Usat.Listener listener);

    /**
     * Creates a USAT command for MO SMS control.
     *
     * @param rpDestAddress The RP_Destination_Address.
     * @param tpDestAddress The TP_Destination_Address.
     * @param networkType The network type that the call is performed. Valid values are
     *                    {@link TelephonyManager#NETWORK_TYPE_UNKNOWN},
     *                    {@link TelephonyManager#NETWORK_TYPE_NR},
     *                    {@link TelephonyManager#NETWORK_TYPE_LTE},
     *                    {@link TelephonyManager#NETWORK_TYPE_HSPA},
     *                    {@link TelephonyManager#NETWORK_TYPE_GPRS},
     *                    {@link TelephonyManager#NETWORK_TYPE_IWLAN}.
     * @param listener The listener for receiving the response.
     *
     * @return A USAT command for MO SMS control.
     */
    Usat.MoSmsControlCommand createMoSmsControlCommand(
            String rpDestAddress, String tpDestAddress, int networkType, Usat.Listener listener);

    /**
     * Creates a USAT command for SMS-PP download.
     *
     * @param rpOriginatingAddress The RP_Originating_Address.
     * @param uriTruncated The flag to indicate that the RP_Originating_Address
     *                     is the URI type and it's truncated.
     * @param tpdu The SMS TPDU (SMS-DELIVER) data.
     * @param originatingAddress The originating address.
     * @param listener The listener for receiving the response.
     *
     * @return A USAT command for SMS-PP download.
     */
    Usat.SmsPpDownloadCommand createSmsPpDownloadCommand(
            String rpOriginatingAddress, boolean uriTruncated, byte[] tpdu,
            String originatingAddress, Usat.Listener listener);

    /**
     * Cancels the given command that was previously sent and has not received
     * the response yet.
     *
     * @param command The command to be canceled.
     */
    void cancelCommand(Usat.Command command);

    /**
     * Sends the command to the UICC with the given command parameters.
     * It will encode the command parameters as an envelope message and send it to the UICC.
     *
     * @param command The command for an envelope message.
     */
    void sendCommand(Usat.Command command);
}
