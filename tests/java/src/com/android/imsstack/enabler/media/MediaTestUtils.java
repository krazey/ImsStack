/**
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

package com.android.imsstack.enabler.media;

import static org.junit.Assert.assertArrayEquals;

import android.net.InetAddresses;
import android.os.Parcel;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AmrParams;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.EvsParams;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.RtcpConfig;
import android.telephony.imsmedia.RtpConfig;
import android.telephony.imsmedia.VideoConfig;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;

public class MediaTestUtils {

    //Media Quality Settings
    public static final int PACKET_LOSS_PERCENT = 30;
    public static final int JITTER = 100;

    // AudioConfig
    private static final String REMOTE_RTP_ADDRESS = "2401:4900:4b88:94fd:2:2:666c:7803";
    private static final int REMOTE_RTP_PORT = 26468;
    private static final byte DSCP = 46;
    private static final byte RX_PAYLOAD = 99;
    private static final byte TX_PAYLOAD = 97;
    private static final byte SAMPLING_RATE = 16;
    private static final int MAX_MTU_BYTES = 1500;
    private static final byte PTIME = 20;
    private static final int MAX_PTIME = 240;
    private static final byte CMR = 100;
    private static final boolean DTX_ENABLED = true;
    private static final byte DTMF_PAYLOAD = 100;
    private static final byte DTMF_SAMPLING_RATE = 16;

    // AmrParams
    private static final boolean OCTET_ALIGNED = true;
    private static final int MAX_REDUNDANCY_MILLIS = 220;

    // EvsParams
    private static final byte CHANNEL_AWARE_MODE = 7;
    private static final boolean USE_HEADER_FULL_ONLY_TX = true;
    private static final boolean USE_HEADER_FULL_ONLY_RX = false;

    // RtcpConfig
    private static final String CANONICAL_NAME = "name";
    private static final int RTCP_PORT = 50011;
    private static final int RTCP_INTERVAL = 500;

    // CallQuality
    private static final int DOWNLINK_CALLQUALITY_LEVEL = CallQuality.CALL_QUALITY_GOOD;
    private static final int UPLINK_CALLQUALITY_LEVEL = CallQuality.CALL_QUALITY_FAIR;
    private static final int CALL_DURATION = 4000;
    private static final int NUM_RTPPACKETS_TRANSMITTED = 500;
    private static final int NUM_RTPPACKETS_RECEIVED = 600;
    private static final int NUM_RTPPACKETS_TRANSMITTED_LOST = 70;
    private static final int NUM_RTPPACKETS_NOT_RECEIVED = 42;
    private static final int AVERAGE_RELATIVE_JITTER = 30;
    private static final int MAX_RELATIVE_JITTER = 40;
    private static final int AVERAGE_ROUND_TRIP_TIME = 100;
    private static final int CODEC_TYPE = 1;

    //RtpHeaderExtension
    private static final int LOCAL_IDENTIFIER = 1;
    private static final int MAX_BYTES = 1;
    private static final byte BYTE_DATA = 10;

    // MediaQualityThreshold
    private static final int RTP_TIMEOUT = 20000;
    private static final int RTCP_TIMEOUT = 20000;
    private static final int PACKET_LOSS_PERIOD = 30000;
    private static final int PACKET_LOSS_RATE = 30;
    private static final int JITTER_PERIOD = 1000;
    private static final int JITTER_THRESHOLD = 1000;

    // VideoConfig
    private static final int VIDEO_FRAMERATE = 15;
    private static final int VIDEO_BITRATE = 384;
    private static final int IDR_INTERVAL = 1;
    private static final int CAMERA_ID = 0;
    private static final int CAMERA_ZOOM = 5;
    static final int RESOLUTION_WIDTH = 240;
    static final int RESOLUTION_HEIGHT = 320;
    private static final String IMAGE_PATH = "data/user_de/0/com.android.imsstack/testvideo.jpg";
    private static final int CVO_VALUE = 2;
    private static final int RTCP_FB_TYPES =
            VideoConfig.RTPFB_NACK | VideoConfig.RTPFB_TMMBR | VideoConfig.RTPFB_TMMBN;
    static final long DATA_BYTES = 200;

    static void assertParcelEquals(Parcel testParcel, Parcel actualParcel) {
        byte[] testByte = testParcel.marshall();
        testParcel.recycle();
        testParcel = null;
        byte[] actualByte = actualParcel.marshall();
        actualParcel.recycle();
        actualParcel = null;
        assertArrayEquals(testByte,actualByte);
    }

    static AudioConfig createAudioConfig() {
        return new AudioConfig.Builder()
                .setMediaDirection(RtpConfig.MEDIA_DIRECTION_SEND_RECEIVE)
                .setAccessNetwork(AccessNetworkType.EUTRAN)
                .setRemoteRtpAddress(new InetSocketAddress(
                     InetAddresses.parseNumericAddress(REMOTE_RTP_ADDRESS), REMOTE_RTP_PORT))
                .setRtcpConfig(createRtcpConfig())
                .setDscp(DSCP)
                .setRxPayloadTypeNumber(RX_PAYLOAD)
                .setTxPayloadTypeNumber(TX_PAYLOAD)
                .setSamplingRateKHz(SAMPLING_RATE)
                .setPtimeMillis(PTIME)
                .setMaxPtimeMillis(MAX_PTIME)
                .setDtxEnabled(DTX_ENABLED)
                .setCodecModeRequest(CMR)
                .setCodecType(AudioConfig.CODEC_EVS)
                .setDtmfPayloadTypeNumber(DTMF_PAYLOAD)
                .setDtmfSamplingRateKHz(DTMF_SAMPLING_RATE)
                .setAmrParams(createAmrParams())
                .setEvsParams(createEvsParams())
                .build();
    }

    private static RtcpConfig createRtcpConfig() {
        return new RtcpConfig.Builder()
            .setCanonicalName(CANONICAL_NAME)
            .setTransmitPort(RTCP_PORT)
            .setIntervalSec(RTCP_INTERVAL)
            .setRtcpXrBlockTypes(RtcpConfig.FLAG_RTCPXR_DLRR_REPORT_BLOCK)
            .build();
    }

    private static AmrParams createAmrParams() {
        return new AmrParams.Builder()
            .setAmrMode(AmrParams.AMR_MODE_7)
            .setOctetAligned(OCTET_ALIGNED)
            .setMaxRedundancyMillis(MAX_REDUNDANCY_MILLIS)
            .build();
    }

    private static EvsParams createEvsParams() {
        return new EvsParams.Builder()
            .setEvsbandwidth(EvsParams.EVS_SUPER_WIDE_BAND)
            .setEvsMode(EvsParams.EVS_MODE_8)
            .setChannelAwareMode(CHANNEL_AWARE_MODE)
            .setHeaderFullOnlyOnTx(USE_HEADER_FULL_ONLY_TX)
            .setHeaderFullOnlyOnRx(USE_HEADER_FULL_ONLY_RX)
            .build();
    }

    static List<RtpHeaderExtension> createRtpExtensions() {
        byte[] testBytes = new byte[MAX_BYTES];
        testBytes[0] = BYTE_DATA;
        ArrayList extensions = new ArrayList<RtpHeaderExtension>();
        extensions.add(new RtpHeaderExtension(LOCAL_IDENTIFIER, testBytes));
        return extensions;
    }

    static CallQuality createCallQuality() {
        return  new CallQuality(
                DOWNLINK_CALLQUALITY_LEVEL,
                UPLINK_CALLQUALITY_LEVEL,
                CALL_DURATION,
                NUM_RTPPACKETS_TRANSMITTED,
                NUM_RTPPACKETS_RECEIVED,
                NUM_RTPPACKETS_TRANSMITTED_LOST,
                NUM_RTPPACKETS_NOT_RECEIVED,
                AVERAGE_RELATIVE_JITTER,
                MAX_RELATIVE_JITTER,
                AVERAGE_ROUND_TRIP_TIME,
                CODEC_TYPE);
    }

    static MediaQualityThreshold createMediaQualityThreshold() {
        return new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(RTP_TIMEOUT)
                .setRtcpInactivityTimerMillis(RTCP_TIMEOUT)
                .setPacketLossPeriodMillis(PACKET_LOSS_PERIOD)
                .setPacketLossThreshold(PACKET_LOSS_RATE)
                .setJitterPeriodMillis(JITTER_PERIOD)
                .setJitterThresholdMillis(JITTER_THRESHOLD)
                .build();
    }

    static VideoConfig createVideoConfig() {
        return new VideoConfig.Builder()
                .setMediaDirection(RtpConfig.MEDIA_DIRECTION_SEND_RECEIVE)
                .setAccessNetwork(AccessNetworkType.EUTRAN)
                .setRemoteRtpAddress(
                        new InetSocketAddress(
                                InetAddresses.parseNumericAddress(REMOTE_RTP_ADDRESS),
                                REMOTE_RTP_PORT))
                .setRtcpConfig(createRtcpConfig())
                .setDscp(DSCP)
                .setRxPayloadTypeNumber(RX_PAYLOAD)
                .setTxPayloadTypeNumber(TX_PAYLOAD)
                .setSamplingRateKHz(SAMPLING_RATE)
                .setMaxMtuBytes(MAX_MTU_BYTES)
                .setVideoMode(VideoConfig.VIDEO_MODE_RECORDING)
                .setCodecType(VideoConfig.VIDEO_CODEC_AVC)
                .setFramerate(VIDEO_FRAMERATE)
                .setBitrate(VIDEO_BITRATE)
                .setCodecProfile(VideoConfig.AVC_PROFILE_BASELINE)
                .setCodecLevel(VideoConfig.AVC_LEVEL_12)
                .setIntraFrameIntervalSec(IDR_INTERVAL)
                .setPacketizationMode(VideoConfig.MODE_NON_INTERLEAVED)
                .setCameraId(CAMERA_ID)
                .setCameraZoom(CAMERA_ZOOM)
                .setResolutionWidth(RESOLUTION_WIDTH)
                .setResolutionHeight(RESOLUTION_HEIGHT)
                .setPauseImagePath(IMAGE_PATH)
                .setDeviceOrientationDegree(0)
                .setCvoValue(CVO_VALUE)
                .setRtcpFbTypes(RTCP_FB_TYPES)
                .build();
    }
}
