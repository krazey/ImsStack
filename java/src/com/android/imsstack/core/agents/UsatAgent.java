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

import android.annotation.NonNull;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * This class provides the implementation of USAT functions to interwork with the UICC.
 */
public class UsatAgent extends Handler implements UsatInterface {
    // Common parameters
    // Device identities
    private static final int TAG_DEVICE_IDENTITIES = 0x02;
    private static final int DEVICE_IDENTITY_UICC = 0x81;
    private static final int DEVICE_IDENTITY_TERMINAL = 0x82; // ME
    private static final int DEVICE_IDENTITY_NETWORK = 0x83;
    // IMS URI: 0x31 or 0xB1
    // private static final int TAG_IMS_URI = 0x31;
    // URI truncated: 0x73 or 0xF3
    // private static final int TAG_URI_TRUNCATED = 0x73;

    // Call control by USIM
    private static final int TAG_CALL_CONTROL = 0xD4;
    // Address: 0x06 or 0x86
    private static final int TAG_ADDRESS = 0x06;
    private static final int TAG_ADDRESS_1 = 0x86;
    // SS string: 0x09 or 0x89
    private static final int TAG_SS_STRING = 0x09;
    private static final int TAG_SS_STRING_1 = 0x89;
    // USSD string: 0x0A or 0x8A
    private static final int TAG_USSD_STRING = 0x0A;
    private static final int TAG_USSD_STRING_1 = 0x8A;
    // Location information: 0x13 or 0x93
    private static final int TAG_LOCATION_INFORMATION = 0x13;
    // Media type: 0x7E or 0xFE
    // Voice: 0x01, Video: 0x02
    private static final int TAG_MEDIA_TYPE = 0x7E;

    // MO SMS control by USIM
    private static final int TAG_MO_SMS_CONTROL = 0xD5;

    // Data download via SMS-PP
    private static final int TAG_SMS_PP_DOWNLOAD = 0xD1;
    // SMS TPDU: 0x0B or 0x8B
    private static final int TAG_SMS_TPDU = 0x0B;

    // Event download: 0xD6
    private static final int TAG_EVENT_DOWNLOAD = 0xD6;
    // Event list: 0x19 or 0x99
    private static final int TAG_EVENT_LIST = 0x19;
    // Ims registration event: 0x17
    private static final int EVENT_IMS_REGISTRATION = 0x17;
    // IMPU list: 0x80 or 0x77
    private static final int TAG_IMPU_LIST = 0x77;
    // IMS status code: 0x80 or 0x78
    private static final int TAG_IMS_STATUS_CODE = 0x78;
    // URI TLV: 0x80
    private static final int TAG_URI_TLV = 0x80;

    static final class DataObject {
        public int tag;
        public byte[] value;

        @Override
        public String toString() {
            return "[ DataObject: tag=0x" + Integer.toHexString(tag)
                    + ", len=" + value.length
                    + ", value="
                    + (value.length == 0
                            ? "(null)"
                            : (ImsLog.DBG
                                    ? ImsUtils.bytesToHexString(value)
                                    : value.length))
                    + " ]";
        }
    }

    static final class UsatResult {
        public int sw1 = 0x93;
        public int sw2 = 0x00;
        public byte[] data = null;

        UsatResult() {
        }

        boolean isOk() {
            return sw1 == 0x90 || sw1 == 0x91 || sw1 == 0x9e || sw1 == 0x9f;
        }

        boolean isValidForClass2Sms() {
            return (sw1 == 0x6F || sw1 == 0x62 || sw1 == 0x63) || isOk();
        }

        @Override
        public String toString() {
            return "[ UsatResult: sw1=0x" + Integer.toHexString(sw1)
                    + ", sw2=0x" + Integer.toHexString(sw2)
                    + ", data="
                    + (data == null
                            ? "(null)"
                            : (ImsLog.DBG
                                    ? ImsUtils.bytesToHexString(data)
                                    : data.length))
                    + " ]";
        }
    }

    // The maximum internal command identifier.
    private static final int MAX_CID = Integer.MAX_VALUE;

    private static final int EVENT_SEND_COMMAND = 1;

    private final Object mLock = new Object();
    private final SimInterface mSim;
    private final SparseArray<Usat.Command> mCommands;
    private int mGlobalCommandId = 1;

    UsatAgent(SimInterface sim) {
        super(AppContext.getInstance().getMainLooper());
        mSim = sim;
        mCommands = new SparseArray<>();
    }

    @Override
    public boolean isServiceAvailable(@Usat.ServiceType int serviceType) {
        byte[] ust = mSim.getUsimServiceTable();
        return ust.length != 0 && isServiceAvailable(ust, serviceType);
    }

    @Override
    public Usat.CallControlCommand createCallControlCommand(
            @Usat.CallControlType int ccType, String dialedString, int networkType,
            @Usat.MediaType int mediaType, Usat.Listener listener) {
        return new Usat.CallControlCommand(getNewCommandId(), listener,
                ccType, dialedString, networkType, mediaType);
    }

    @Override
    public Usat.MoSmsControlCommand createMoSmsControlCommand(
            String rpDestAddress, String tpDestAddress, int networkType, Usat.Listener listener) {
        return new Usat.MoSmsControlCommand(getNewCommandId(), listener,
                rpDestAddress, tpDestAddress, networkType);
    }

    @Override
    public Usat.SmsPpDownloadCommand createSmsPpDownloadCommand(
            String rpOriginatingAddress, boolean uriTruncated, byte[] tpdu,
            String originatingAddress, Usat.Listener listener) {
        return new Usat.SmsPpDownloadCommand(getNewCommandId(), listener,
                rpOriginatingAddress, uriTruncated, tpdu, originatingAddress);
    }

    @Override
    public Usat.RegEventDownloadCommand createRegEventDownloadCommand(
            int statusCode, @NonNull Set<Uri> impus, Usat.Listener listener) {
        return new Usat.RegEventDownloadCommand(getNewCommandId(), listener, statusCode, impus);
    }

    @Override
    public void cancelCommand(Usat.Command command) {
        if (command == null) {
            // Do nothing.
            return;
        }

        logd(this, "cancelCommand - " + command);

        synchronized (mLock) {
            if (mCommands.contains(command.getCid())) {
                command.markAsAborted();
            }
        }
    }

    @Override
    public void sendCommand(Usat.Command command) {
        if (command == null) {
            return;
        }

        logi(this, "sendCommand - " + command);

        synchronized (mLock) {
            mCommands.put(command.getCid(), command);
        }

        Message.obtain(this, EVENT_SEND_COMMAND, command).sendToTarget();
    }

    @Override
    public void handleMessage(@NonNull Message msg) {
        logi(this, "handleMessage - msg=" + msg.what);

        switch (msg.what) {
            case EVENT_SEND_COMMAND: {
                Usat.Command cmd = (Usat.Command) msg.obj;
                handleUsatCommand(cmd);
                break;
            }
            default:
                break;
        }
    }

    /**
     * Handles a USAT command.
     *
     * @param cmd The command to be handled.
     */
    private void handleUsatCommand(Usat.Command cmd) {
        switch (cmd.getServiceType()) {
            case Usat.SERVICE_CALL_CONTROL:
                handleCallControlCommand((Usat.CallControlCommand) cmd);
                break;
            case Usat.SERVICE_MO_SMS_CONTROL:
                handleMoSmsControlCommand((Usat.MoSmsControlCommand) cmd);
                break;
            case Usat.SERVICE_DATA_DOWNLOAD_VIA_SMS_PP:
                handleSmsPpDownloadCommand((Usat.SmsPpDownloadCommand) cmd);
                break;
            case Usat.SERVICE_REGISTRATION_EVENT_DOWNLOAD:
                handleRegEventDownloadCommand((Usat.RegEventDownloadCommand) cmd);
                break;
            default:
                logd(this, "unknown command - " + cmd);
                break;
        }
    }

    /**
     * Checks if the command is aborted by the caller or not.
     *
     * @param cmd The command to be checked.
     * @return true if it's aborted, false otherwise.
     */
    private boolean isCommandAborted(Usat.Command cmd) {
        synchronized (mLock) {
            if (cmd.isAborted()) {
                return true;
            }
        }

        return false;
    }

    /**
     * Notifies the caller that the command response of a specific command is received.
     *
     * @param response The command response to be notified.
     */
    private void notifyCommandResponse(Usat.CommandResponse response) {
        removeCommand(response.getCommand());

        if (isCommandAborted(response.getCommand())) {
            logi(this, "command is already aborted - "
                    + response.getCommand());
            return;
        }

        Usat.Listener listener = response.getCommand().getListener();

        if (listener != null) {
            listener.onCommandResponse(response);
        }
    }

    /**
     * Handles the call control command.
     *
     * @param cmd The call control command.
     */
    private void handleCallControlCommand(final Usat.CallControlCommand cmd) {
        logd(this, "handleCallControlCommand");

        String encodedCommand = encodeCommandForCallControl(cmd);
        String response = "";

        if (encodedCommand != null) {
            response = sendEnvelopeWithStatus(encodedCommand);
        }

        UsatResult result = createUsatResult(response);

        if (ImsLog.DBG) {
            logd(this, "call-control - encodedCommand=" + encodedCommand
                    + ", response=" + response + ", result=" + result);
        }

        if (!result.isOk()) {
            logw(this, "call-control failed - " + result);
        }

        Usat.CallControlCommandResponse cmdResponse =
                decodeCommandResponseForCallControl(cmd, result);

        if (cmdResponse != null) {
            notifyCommandResponse(cmdResponse);
        } else {
            logi(this, "call-control command aborted");
        }
    }

    /**
     * Encodes the call control command to be sent to the UICC.
     *
     * @param cmd The call control command.
     * @return A hexadecimal string format of this command.
     */
    private String encodeCommandForCallControl(Usat.CallControlCommand cmd) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        buffer.write(TAG_CALL_CONTROL);
        // Sets an approximate length.
        // This will be adjusted after encoding all the parameters.
        buffer.write(0x00);

        // Device identities
        writeDeviceIdentities(buffer, DEVICE_IDENTITY_TERMINAL, DEVICE_IDENTITY_UICC);

        // Address or SS string or USSD string
        int ccType = cmd.getCcType();

        if (ccType == Usat.CALL_CONTROL_TYPE_MO_CALL) {
            if (!writeAddress(buffer, cmd.getDialedString())) {
                logw(this, "writing address failed");
                return null;
            }
        } else if (ccType == Usat.CALL_CONTROL_TYPE_SS) {
            if (!writeSsString(buffer, cmd.getDialedString())) {
                logw(this, "writing SS string failed");
                return null;
            }
        } else if (ccType == Usat.CALL_CONTROL_TYPE_USSD) {
            if (!writeUssdString(buffer, cmd.getDialedString())) {
                logw(this, "writing USSD string failed");
                return null;
            }
        }

        // Location information if available
        int networkType = cmd.getNetworkType();

        if (networkType != TelephonyManager.NETWORK_TYPE_UNKNOWN
                && networkType != TelephonyManager.NETWORK_TYPE_IWLAN) {
            byte[] locationInfo = getLocationInfo(networkType);

            buffer.write(TAG_LOCATION_INFORMATION);

            if (locationInfo != null) {
                buffer.write(locationInfo.length);
                buffer.writeBytes(locationInfo);
            } else {
                logd(this, "no location information.");
                buffer.write(0);
            }
        }

        // Media type supports
        if (isServiceAvailable(Usat.SERVICE_MEDIA_TYPE_SUPPORT)) {
            int mediaType = cmd.getMediaType();

            if (mediaType == Usat.MEDIA_TYPE_VOICE
                    || mediaType == Usat.MEDIA_TYPE_VIDEO) {
                buffer.write(TAG_MEDIA_TYPE);
                buffer.write(0x01);
                buffer.write((byte) (1 << mediaType));
            }
        }

        byte[] data = buffer.toByteArray();

        // Adjust the length field of BER-TLV data object.
        data = refineBerTlvDataObject(data);

        return ImsUtils.bytesToHexString(data);
    }

    /**
     * Decodes the response data for call control command received from the UICC.
     *
     * @param cmd The call control command.
     * @param result The UsatResult containing the response data.
     * @return The command response of call control.
     */
    private Usat.CallControlCommandResponse decodeCommandResponseForCallControl(
            Usat.CallControlCommand cmd, UsatResult result) {
        if (isCommandAborted(cmd)) {
            return null;
        }

        int cmdResult = result.isOk() ? Usat.RESULT_ALLOWED : Usat.RESULT_NOT_ALLOWED;
        int ccType = Usat.CALL_CONTROL_TYPE_NONE;
        String dialedString = null;
        int mediaType = Usat.MEDIA_TYPE_NONE;

        int responseResult = (result.data != null && result.data.length > 0)
                ? (result.data[0] & 0xFF) : -1;

        if (responseResult == 0x00) {
            cmdResult = Usat.RESULT_ALLOWED;
        } else if (responseResult == 0x01) {
            cmdResult = Usat.RESULT_NOT_ALLOWED;
        } else if (responseResult == 0x02) {
            cmdResult = Usat.RESULT_ALLOWED_WITH_MODIFICATION;

            ArrayList<DataObject> dataObjects = new ArrayList<>();

            if (!extractDataObjectFromBuffer(result.data, dataObjects)) {
                // Malformed data format. Do not allow the call.
                cmdResult = Usat.RESULT_NOT_ALLOWED;
            } else {
                logi(this, "response data objects=" + dataObjects.size());

                if (dataObjects.isEmpty()) {
                    cmdResult = Usat.RESULT_NOT_ALLOWED;
                }
            }

            for (DataObject object : dataObjects) {
                final int tag = object.tag;
                final byte[] value = object.value;

                if (tag == TAG_ADDRESS || tag == TAG_ADDRESS_1) {
                    ccType = Usat.CALL_CONTROL_TYPE_MO_CALL;

                    if (value.length != 0) {
                        dialedString = PhoneNumberUtils.calledPartyBCDToString(
                                value, 0, value.length,
                                PhoneNumberUtils.BCD_EXTENDED_TYPE_EF_ADN);
                    }
                } else if (tag == TAG_SS_STRING || tag == TAG_SS_STRING_1) {
                    ccType = Usat.CALL_CONTROL_TYPE_SS;

                    if (value.length != 0) {
                        dialedString = PhoneNumberUtils.calledPartyBCDToString(
                                value, 0, value.length,
                                PhoneNumberUtils.BCD_EXTENDED_TYPE_EF_ADN);
                    }
                } else if (tag == TAG_USSD_STRING || tag == TAG_USSD_STRING_1) {
                    ccType = Usat.CALL_CONTROL_TYPE_USSD;

                    if (value.length != 0) {
                        dialedString = new String(value, StandardCharsets.UTF_8);
                    }
                } else if (tag == TAG_MEDIA_TYPE || tag == (TAG_MEDIA_TYPE | 0x80)) {
                    if (value.length != 0) {
                        int type = value[0] & 0xFF;

                        if ((type & (1 << Usat.MEDIA_TYPE_VOICE)) != 0) {
                            mediaType = Usat.MEDIA_TYPE_VOICE;
                        } else if ((type & (1 << Usat.MEDIA_TYPE_VIDEO)) != 0) {
                            mediaType = Usat.MEDIA_TYPE_VIDEO;
                        }
                    }
                }
            }
        }

        return new Usat.CallControlCommandResponse(
                cmd, cmdResult, ccType, dialedString, mediaType);
    }

    /**
     * Handles the MO SMS control command.
     *
     * @param cmd The MO SMS command.
     */
    private void handleMoSmsControlCommand(final Usat.MoSmsControlCommand cmd) {
        logd(this, "handleMoSmsControlCommand");

        String encodedCommand = encodeCommandForMoSmsControl(cmd);
        String response = "";

        if (encodedCommand != null) {
            response = sendEnvelopeWithStatus(encodedCommand);
        }

        UsatResult result = createUsatResult(response);

        if (ImsLog.DBG) {
            logd(this, "mo-sms-control - encodedCommand=" + encodedCommand
                    + ", response=" + response + ", result=" + result);
        }

        if (!result.isOk()) {
            logw(this, "mo-sms-control failed - " + result);
        }

        Usat.MoSmsControlCommandResponse cmdResponse =
                decodeCommandResponseForMoSmsControl(cmd, result);

        if (cmdResponse != null) {
            notifyCommandResponse(cmdResponse);
        } else {
            logi(this, "mo-sms-control command aborted");
        }
    }

    /**
     * Encodes the MO SMS control command to be sent to the UICC.
     *
     * @param cmd The MO SMS control command.
     * @return A hexadecimal string format of this command.
     */
    private String encodeCommandForMoSmsControl(Usat.MoSmsControlCommand cmd) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        buffer.write(TAG_MO_SMS_CONTROL);
        // Sets an approximate length.
        // This will be adjusted after encoding all the parameters.
        buffer.write(0x00);

        // Device identities (Source(TERMINAL/ME) --> Destination(UICC))
        writeDeviceIdentities(buffer, DEVICE_IDENTITY_TERMINAL, DEVICE_IDENTITY_UICC);

        // RP_Destination_Address of the Service Center
        if (!writeAddress(buffer, cmd.getRpDestinationAddress())) {
            logw(this, "writing RP_Destination_Address failed");
            return null;
        }

        // TP_Destination_Address
        if (!writeAddress(buffer, cmd.getTpDestinationAddress())) {
            logw(this, "writing TP_Destination_Address failed");
            return null;
        }

        // Location information (if available) based on network type
        int networkType = cmd.getNetworkType();

        byte[] locationInfo = getLocationInfo(networkType);

        buffer.write(TAG_LOCATION_INFORMATION);

        if (locationInfo != null) {
            buffer.write(locationInfo.length);
            buffer.writeBytes(locationInfo);
        } else {
            logd(this, "no location information.");
            buffer.write(0);
        }

        byte[] data = buffer.toByteArray();

        // Adjust the length field of BER-TLV data object.
        data = refineBerTlvDataObject(data);

        return ImsUtils.bytesToHexString(data);
    }

    /**
     * Decodes the response data for MO SMS control command received from the UICC.
     *
     * @param cmd The MO SMS control command.
     * @param result The UsatResult containing the response data.
     * @return The command response of MO SMS control.
     */
    private Usat.MoSmsControlCommandResponse decodeCommandResponseForMoSmsControl(
            Usat.MoSmsControlCommand cmd, UsatResult result) {
        if (isCommandAborted(cmd)) {
            return null;
        }

        if (!result.isOk()) {
            return new Usat.MoSmsControlCommandResponse(cmd, Usat.RESULT_NOT_ALLOWED, null, null);
        }

        String[] addresses = new String[] { null, null };
        int responseResult = (result.data != null && result.data.length > 0)
                ? (result.data[0] & 0xFF) : -1;
        int cmdResult = 0;

        if (responseResult == 0x00) {
            cmdResult = Usat.RESULT_ALLOWED;
        } else if (responseResult == 0x01) {
            cmdResult = Usat.RESULT_NOT_ALLOWED;
        } else if (responseResult == 0x02) {
            cmdResult = Usat.RESULT_ALLOWED_WITH_MODIFICATION;

            ArrayList<DataObject> dataObjects = new ArrayList<>();

            if (!extractDataObjectFromBuffer(result.data, dataObjects)) {
                return new Usat.MoSmsControlCommandResponse(cmd, Usat.RESULT_NOT_ALLOWED, null,
                        null);
            }

            logi(this, "response data objects size = " + dataObjects.size());

            for (int i = 0; i < dataObjects.size(); i++) {
                if (i == 2) break;
                DataObject object = dataObjects.get(i);
                final int tag = object.tag;
                final byte[] value = object.value;
                if (tag == TAG_ADDRESS || tag == TAG_ADDRESS_1) {
                    if (value.length != 0) {
                        addresses[i] = PhoneNumberUtils.calledPartyBCDToString(
                                value, 0, value.length, PhoneNumberUtils.BCD_EXTENDED_TYPE_EF_ADN);
                    }
                }
            }

            if (addresses[0] == null || addresses[1] == null) {
                cmdResult = Usat.RESULT_NOT_ALLOWED;
                addresses[0] = null;
                addresses[1] = null;
            }
        }

        return new Usat.MoSmsControlCommandResponse(cmd, cmdResult, addresses[0], addresses[1]);
    }

    /**
     * Handles the SMS-PP download command.
     *
     * @param cmd The SMS-PP download command.
     */
    private void handleSmsPpDownloadCommand(final Usat.SmsPpDownloadCommand cmd) {
        logd(this, "handleSmsPpDownloadCommand");

        String encodedCommand = encodeCommandForSmsPpDownload(cmd);
        String response = "";

        if (encodedCommand != null) {
            response = sendEnvelopeWithStatus(encodedCommand);
        }

        UsatResult result = createUsatResult(response);

        if (ImsLog.DBG) {
            logd(this, "sms-pp-download - encodedCommand=" + encodedCommand
                    + ", response=" + response + ", result=" + result);
        }

        if (!result.isOk()) {
            logw(this, "sms-pp-download failed - " + result);
        }

        Usat.SmsPpDownloadCommandResponse cmdResponse =
                decodeCommandResponseForSmsPpDownload(cmd, result);

        if (cmdResponse != null) {
            notifyCommandResponse(cmdResponse);
        } else {
            logi(this, "sms-pp-download command aborted");
        }
    }

    /**
     * Encodes the SMS-PP download command to be sent to the UICC.
     *
     * @param cmd The SMS-PP download command.
     * @return A hexadecimal string format of this command.
     */
    private String encodeCommandForSmsPpDownload(Usat.SmsPpDownloadCommand cmd) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        buffer.write(TAG_SMS_PP_DOWNLOAD);
        // This will be adjusted after encoding all the parameters.
        buffer.write(0x00);

        // Device identities (Source(NETWORK) --> Destination(UICC))
        writeDeviceIdentities(buffer, DEVICE_IDENTITY_NETWORK, DEVICE_IDENTITY_UICC);

        // writeAddress (RP_Originating Address of Service Centre)
        if (!writeAddress(buffer, cmd.getRpOriginatingAddress())) {
            logw(this, "writing RP_Originating_Address failed");
            return null;
        }

        // SMS TPDU
        buffer.write(TAG_SMS_TPDU);
        byte[] tpdu = cmd.getTpdu();
        writeLength(buffer, tpdu.length);
        buffer.writeBytes(tpdu);

        byte[] data = buffer.toByteArray();

        // Adjust the length field of BER-TLV data object.
        data = refineBerTlvDataObject(data);

        return ImsUtils.bytesToHexString(data);
    }

    /**
     * Decodes the response data for SMS-PP download command received from the UICC.
     *
     * @param cmd The SMS-PP download command.
     * @param result The UsatResult containing the response data.
     * @return The command response of SMS-PP download.
     */
    private Usat.SmsPpDownloadCommandResponse decodeCommandResponseForSmsPpDownload(
            Usat.SmsPpDownloadCommand cmd, UsatResult result) {
        if (isCommandAborted(cmd)) {
            return null;
        }

        int cmdResult;
        if (result.isValidForClass2Sms()) {
            cmdResult = Usat.RESULT_DATA_DOWNLOAD_OK;
            logi(this, "sms-pp download ok");
        } else {
            cmdResult = Usat.RESULT_DATA_DOWNLOAD_ERROR;
            logi(this, "sms-pp download error");
        }

        return new Usat.SmsPpDownloadCommandResponse(cmd, cmdResult, result.data);
    }

    /**
     * Gets the location information as a byte array.
     *
     * @param networkType The network type.
     * @return A byte array containing the location information.
     */
    private byte[] getLocationInfo(int networkType) {
        IDcUtils dcUtils = DcFactory.getDcAgent(IDcUtils.class, getSlotId());

        if (dcUtils == null) {
            return null;
        }

        IDcUtils.AccessNetworkInfo dcAni = dcUtils.getAccessNetworkInfo(networkType);
        String[] ani = (dcAni != null) ? dcAni.mAni : null;

        if (ani == null) {
            return null;
        }

        if (ani.length < 4
                || TextUtils.isEmpty(ani[0])
                || TextUtils.isEmpty(ani[1])
                || TextUtils.isEmpty(ani[2])
                || TextUtils.isEmpty(ani[3])) {
            return null;
        }

        String plmn = ani[0];

        if (ani[1].length() == 2) {
            plmn += "F";
        }

        plmn += ani[1];

        StringBuilder sb = new StringBuilder(ImsUtils.stringToBcdString(plmn));

        int tacOrLacLen = 4; // E-UTRAN / UTRAN / GERAN
        int cellIdLen = 7; // E-UTRAN / UTRAN

        if (dcAni.mNetworkType == TelephonyManager.NETWORK_TYPE_NR) {
            tacOrLacLen = 6;
            cellIdLen = 9;
        } else if (dcAni.mNetworkType == TelephonyManager.NETWORK_TYPE_GPRS
                || dcAni.mNetworkType == TelephonyManager.NETWORK_TYPE_EDGE) {
            cellIdLen = 4;
        }

        // TAC or LAC
        if (ani[3].length() < tacOrLacLen) {
            for (int i = 0; i < tacOrLacLen - ani[3].length(); i++) {
                sb.append("0");
            }
        }

        sb.append(ani[3]);

        // Cell identifier
        if (ani[2].length() < cellIdLen) {
            for (int i = 0; i < cellIdLen - ani[2].length(); i++) {
                sb.append("0");
            }
        }

        sb.append(ani[2]);

        if (cellIdLen % 2 != 0) {
            sb.append("F");
        }

        return ImsUtils.hexStringToBytes(sb.toString());
    }

    /**
     * Handles the registration event download command.
     *
     * @param cmd The registration event download command.
     */
    private void handleRegEventDownloadCommand(final Usat.RegEventDownloadCommand cmd) {
        logd(this, "handleRegEventDownloadCommand");

        String encodedCommand = encodeCommandForRegEventDownload(cmd);
        String response = "";

        if (encodedCommand != null) {
            response = sendEnvelopeWithStatus(encodedCommand);
        }

        UsatResult result = createUsatResult(response);

        if (ImsLog.DBG) {
            logd(this, "reg-event-download - encodedCommand="
                    + encodedCommand + ", response=" + response + ", result=" + result);
        }

        if (!result.isOk()) {
            logw(this, "reg-event-download failed - " + result);
        }

        Usat.CommandResponse cmdResponse = new Usat.CommandResponse(cmd,
                result.isOk() ? Usat.RESULT_REGISTRATION_EVENT_OK
                        : Usat.RESULT_REGISTRATION_EVENT_ERROR);
        if (cmdResponse != null) {
            notifyCommandResponse(cmdResponse);
        } else {
            logi(this, "reg-event-download command aborted");
        }
    }

    /**
     * Encodes the registration event download command to be sent to the UICC.
     *
     * @param cmd The registration event download command.
     * @return A hexadecimal string format of this command.
     */
    private String encodeCommandForRegEventDownload(Usat.RegEventDownloadCommand cmd) {

        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        // Event download tag (Table 7.17 of TS101.220 : 0xD6)
        buffer.write(TAG_EVENT_DOWNLOAD);

        // Length (A+B+C) or (A+B+D)
        //     A: Event list
        //     B: Device identities
        //     C: IMPU list
        //     D: IMS status code
        buffer.write(0x00); // place holder

        writeEventList(buffer, EVENT_IMS_REGISTRATION);

        writeDeviceIdentities(buffer, DEVICE_IDENTITY_NETWORK, DEVICE_IDENTITY_UICC);

        int statusCode = cmd.getStatusCode();
        if (statusCode == 200) {
            try {
                writeImpuList(buffer, cmd.getImpus());
            } catch (IOException e) {
                loge(this, e.toString());
                return null;
            }
        } else {
            writeImsStatusCode(buffer, statusCode);
        }

        byte[] data = buffer.toByteArray();

        // Adjust the length field of BER-TLV data object.
        data = refineBerTlvDataObject(data);

        return ImsUtils.bytesToHexString(data);
    }

    private void logd(Object o, String s) {
        ImsLog.d(o, getSlotId(), "USAT: " + s);
    }

    private void loge(Object o, String s) {
        ImsLog.e(o, getSlotId(), "USAT: " + s);
    }

    private void logi(Object o, String s) {
        ImsLog.i(o, getSlotId(), "USAT: " + s);
    }

    private void logw(Object o, String s) {
        ImsLog.w(o, getSlotId(), "USAT: " + s);
    }

    private static void loge(String s) {
        ImsLog.e(null, "USAT: " + s);
    }

    /**
     * Writes the event list.
     *     Event list tag (Table 7.23 of TS101.220 : 0x19 or 0x99)
     *     Length (0x01)
     *     Event value (8.25 of TS131.111 : 0x17)
     *
     * @param buffer The output stream to be written.
     * @param event The event value to be written.
     */
    private static void writeEventList(ByteArrayOutputStream buffer, int event) {
        buffer.write(TAG_EVENT_LIST); // 0x19
        buffer.write(0x01);
        buffer.write(event);
    }

    /**
     * Writes the IMPU list.
     *     IMPU list tag (Table 7.23 of TS101.220 : 0x80 or 0x77)
     *     Length
     *     URI TLV tag (8.111 of TS131.111 : 0x80)
     *     URI TLV length
     *     IMPU list
     *
     * @param buffer The output stream to be written.
     * @param impus The set of IMPU to be written.
     * @throws IOException  if an I/O error occurs.
     */
    private static void writeImpuList(ByteArrayOutputStream buffer, Set<Uri> impus)
            throws IOException {
        ByteArrayOutputStream bufferUriTlv = new ByteArrayOutputStream();
        for (Uri impu : impus) {
            writeUriTlv(bufferUriTlv, impu);
        }

        buffer.write(TAG_IMPU_LIST); // 0x77
        buffer.write(bufferUriTlv.size());
        buffer.write(bufferUriTlv.toByteArray());
    }

    /**
     * Writes the URI TLV.
     *
     * @param buffer The output stream to be written.
     * @param uri The URI to be written.
     * @throws IOException if an I/O error occurs.
     */
    private static void writeUriTlv(ByteArrayOutputStream buffer, Uri uri) throws IOException {
        buffer.write(TAG_URI_TLV); // 0x80

        byte[] uriBytes = uri.toString().getBytes(StandardCharsets.UTF_8);
        buffer.write(uriBytes.length); // URI TLV length
        buffer.write(uriBytes);
    }

    /**
     * Writes the IMS status code.
     *     Status Code (8.111 of TS131.111)
     *         IMS Status-code Tag (Table 7.23 of TS101.220 : 0x80 or 0x78)
     *         Length
     *         IMS Status-code
     * @param buffer The output stream to be written.
     * @param code The IMS status code to be written.
     */
    private static void writeImsStatusCode(ByteArrayOutputStream buffer, int code) {
        buffer.write(TAG_IMS_STATUS_CODE);

        String codeStr = String.valueOf(code);
        if (TextUtils.isEmpty(codeStr)) {
            buffer.write(0);
            return;
        }

        int codeLength = codeStr.length();
        buffer.write(codeLength);

        for (int i = 0; i < codeLength; i++) {
            buffer.write(codeStr.charAt(i));
        }
    }

    /**
     * Sends an envelope message to UICC through the Telephony Service.
     *
     * @param content The envelope message which contains a specific command parameters.
     * @return A response data from UICC.
     */
    private @NonNull String sendEnvelopeWithStatus(String content) {
        int subId = MSimUtils.getSubId(getSlotId());
        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(subId);
        return tmp.sendEnvelopeWithStatus(content);
    }

    /**
     * Returns the slot-id of this object.
     *
     * @return The slot-id.
     */
    private int getSlotId() {
        return mSim.getSlotId();
    }

    /**
     * Returns new command identifier.
     *
     * @return New command identifier or -1.
     */
    private int getNewCommandId() {
        int newCid = (-1);

        synchronized (mLock) {
            int cid = mGlobalCommandId;

            while (true) {
                if (!mCommands.contains(cid)) {
                    newCid = cid;
                    break;
                }

                cid++;

                if (cid == MAX_CID) {
                    cid = 1;
                }
            }

            if (newCid > 0) {
                mGlobalCommandId = newCid + 1;

                if (mGlobalCommandId == MAX_CID) {
                    resetGlobalCommandId();
                }
            }
        }

        return newCid;
    }

    /**
     * Resets the global command identifier.
     */
    private void resetGlobalCommandId() {
        mGlobalCommandId = 1;
    }

    /**
     * Removes the specified command.
     *
     * @param cmd The command to be removed.
     */
    private void removeCommand(Usat.Command cmd) {
        if (cmd == null) {
            return;
        }

        synchronized (mLock) {
            mCommands.remove(cmd.getCid());

            if (mCommands.size() == 0) {
                resetGlobalCommandId();
            }
        }
    }

    /**
     * Checks if the service table supports the specified service or not.
     *
     * @param serviceTable The service table in USIM or ISIM.
     * @param service The specified service to be evaluated.
     * @return true if the specified service is available, false otherwise.
     */
    private static boolean isServiceAvailable(byte[] serviceTable, int service) {
        int offset = service / 8;

        if (offset >= serviceTable.length) {
            return false;
        }

        int bit = service % 8;

        return (serviceTable[offset] & (1 << bit)) != 0;
    }

    /**
     * Refines the length field of the first object after encoding all the parameters.
     * The given buffer contains the tag and 1 byte length of the BER-TLV data object.
     *
     * @param buf The buffer to be refined
     * @return The existing buffer or newly created buffer if the length is coded as 2 bytes.
     */
    private static byte[] refineBerTlvDataObject(byte[] buf) {
        // Minus: tag (1 byte) and 1 byte length of BER-TLV data object.
        int len = buf.length - 2;

        if (len <= 0x7F) {
            buf[1] = (byte) len;
            return buf;
        }

        byte[] data = new byte[buf.length + 1]; // for 2-bytes length field

        data[0] = buf[0]; // Tag
        data[1] = (byte) 0x81; // Identifier for 2-bytes length
        data[2] = (byte) (len & 0xFF);

        // All the COMPREHENSION-TLV data objects
        System.arraycopy(buf, 2, data, 3, len);

        return data;
    }

    /**
     * Writes the device identities.
     *
     * @param buffer The output stream to be written.
     * @param srcId The source device identity.
     * @param dstId The destination device identity.
     */
    private static void writeDeviceIdentities(ByteArrayOutputStream buffer, int srcId, int dstId) {
        buffer.write(TAG_DEVICE_IDENTITIES);
        buffer.write(0x02);
        buffer.write(srcId);
        buffer.write(dstId);
    }

    /**
     * Writes the address.
     *
     * @param buffer The output stream to be written.
     * @param address The address.
     * @return true if the operation is successfully done, false otherwise.
     */
    private static boolean writeAddress(ByteArrayOutputStream buffer, String address) {
        return writeDataAsEfAdn(buffer, TAG_ADDRESS, address);
    }

    /**
     * Writes the SS string.
     *
     * @param buffer The output stream to be written.
     * @param ssString The SS string.
     * @return true if the operation is successfully done, false otherwise.
     */
    private static boolean writeSsString(ByteArrayOutputStream buffer, String ssString) {
        return writeDataAsEfAdn(buffer, TAG_SS_STRING, ssString);
    }

    /**
     * Writes the USSD string.
     *
     * @param buffer The output stream to be written.
     * @param ussdString The USSD string.
     * @return true if the operation is successfully done, false otherwise.
     */
    private static boolean writeUssdString(ByteArrayOutputStream buffer, String ussdString) {
        byte[] ussdBytes = ussdString.getBytes(StandardCharsets.UTF_8);

        if (ussdBytes == null) {
            return false;
        }

        buffer.write(TAG_USSD_STRING);
        // DCS: 1 byte, PFI (Packet Format Information): 1 byte, USSD string length
        int len = (1 + 1 + ussdBytes.length);

        writeLength(buffer, len);
        buffer.write(0x96);
        buffer.write(0x00); // Proprietary Application Data Format
        buffer.writeBytes(ussdBytes);

        return true;
    }

    /**
     * Writes the data which is coded as for EF-ADN format.
     *
     * @param buffer The output stream to be written.
     * @param tag The tag identifier.
     * @param data The data to be coded.
     * @return true if the operation is successfully done, false otherwise.
     */
    private static boolean writeDataAsEfAdn(ByteArrayOutputStream buffer, int tag, String data) {
        byte[] bcdBytes = PhoneNumberUtils.networkPortionToCalledPartyBCD(data);

        if (bcdBytes == null) {
            return false;
        }

        int len = bcdBytes.length;

        buffer.write(tag);
        writeLength(buffer, len);
        buffer.write(bcdBytes, 0, len);

        return true;
    }

    /**
     * Writes one or two bytes length according to the data length.
     *
     * @param buffer The output stream to be written.
     * @param length The length value.
     */
    private static void writeLength(ByteArrayOutputStream buffer, int length) {
        if (length <= 0x7F) {
            buffer.write(length);
        } else {
            buffer.write(0x81);
            buffer.write(length);
        }
    }

    /**
     * Creates a UsatResult object with the given response from UICC.
     *
     * @return A UsatResult object
     */
    private static UsatResult createUsatResult(String response) {
        UsatResult result = new UsatResult();

        if (response == null || response.isEmpty()) {
            return result;
        }

        byte[] res = ImsUtils.hexStringToBytes(response);

        if (res == null || res.length < 2) {
            // Invalid response data.
            return result;
        }

        result.sw1 = (res[res.length - 2] & 0xFF);
        result.sw2 = (res[res.length - 1] & 0xFF);
        result.data = new byte[res.length - 2];

        if (result.data.length != 0) {
            System.arraycopy(res, 0, result.data, 0, result.data.length);
        }

        return result;
    }

    /**
     * Returns the size of the length field using the first byte value in the length field.
     *
     * @return 1 if a byte is less and equal that 0x7F, 2 if a byte is 0x81, 0 otherwise.
     */
    private static int sizeOfLengthField(byte firstByteOfLength) {
        if ((int) firstByteOfLength <= 0x7F) {
            return 1;
        } else if (firstByteOfLength == (byte) 0x81) {
            return 2;
        }
        return 0;
    }

    /**
     * Extracts the TLV data object from the given buffer (response data).
     * The buffer is a BER-TLV data object.
     *
     * @param buf The buffer to be parsed
     * @param dataObjects The list of TlvObject
     * @return true if the encoded data is valid, false otherwise.
     */
    private static boolean extractDataObjectFromBuffer(byte[] buf, List<DataObject> dataObjects) {
        int totalLen = buf.length;

        if (totalLen <= 1) {
            return false;
        }

        int tag = buf[0] & 0xFF;
        int offset = 1; // 0: tag, 1: length (all the parameters)
        int sizeOfLen = sizeOfLengthField(buf[offset]);
        int paramLen = 0;

        if (sizeOfLen == 1) {
            paramLen = buf[offset] & 0xFF;
        } else if (sizeOfLen == 2) {
            offset++;

            if (totalLen > 2) {
                paramLen = buf[offset] & 0xFF;
            } else {
                loge("BER-TLV: Invalid 2 bytes length; tag=0x"
                        + Integer.toHexString(tag));
                return false;
            }
        } else {
            loge("BER-TLV: Invalid length; tag=0x" + Integer.toHexString(tag));
            return false;
        }

        // Check with the buffer length except for the tag and size of length field
        if (paramLen != (totalLen - (1 + sizeOfLen))) {
            loge("BER-TLV: Length mismatched; tag=0x" + Integer.toHexString(tag)
                    + ", paramLen=" + paramLen
                    + ", totalLen=" + totalLen
                    + ", sizeOfLen=" + sizeOfLen);
            return false;
        }

        // Move to COMPREHENSION-TLV data offset
        offset++;

        while (offset < totalLen) {
            DataObject object = new DataObject();

            object.tag = buf[offset++] & 0xFF;

            if (offset < totalLen) {
                int valueLen = -1;

                sizeOfLen = sizeOfLengthField(buf[offset]);

                if (sizeOfLen == 1) {
                    valueLen = buf[offset] & 0xFF;
                } else if (sizeOfLen == 2) {
                    offset++;

                    if (offset < totalLen) {
                        valueLen = buf[offset] & 0xFF;
                    }
                }

                if (valueLen < 0) {
                    loge("COMPREHENSION-TLV: Invalid length; tag=0x"
                            + Integer.toHexString(object.tag));
                    return false;
                }

                if ((offset + valueLen) < totalLen) {
                    offset++;

                    object.value = new byte[valueLen];

                    if (valueLen != 0) {
                        System.arraycopy(buf, offset, object.value, 0, valueLen);
                        offset += valueLen;
                    }

                    dataObjects.add(object);
                } else {
                    loge("COMPREHENSION-TLV: Invalid value; tag=0x"
                            + Integer.toHexString(object.tag));
                    return false;
                }
            }
        }

        return true;
    }
}
