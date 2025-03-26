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

import android.annotation.NonNull;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.CallQuality;
import android.telephony.ims.MediaThreshold;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityStatus;
import android.telephony.imsmedia.RtpReceptionStats;

import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.util.ImsLog;

import java.util.List;
import java.util.stream.Collectors;

/**
 * This handles callbacks received from {@link ImsMediaManager} and passes it to
 * ImsStack (Media Enabler Native)
 */
public class AudioSessionCallbackHandler {

    private final IMtcMediaInterface mMtcMediaInterface;

    public AudioSessionCallbackHandler(@NonNull final IMtcMediaInterface mtcMediaInterface) {
        mMtcMediaInterface = mtcMediaInterface;
        ImsLog.d("Constructor - Exit");
    }

    private IMtcMediaInterface getMtcMediaInterface() {
        return mMtcMediaInterface;
    }

    /**
     * Handles the response for open session request
     *
     * @param result result of the open session request
     */
    public void openSessionResponse(@ImsMediaSession.SessionOperationResult int result) {
        ImsLog.d("openSessionResponse Result=" + result);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(result);

        if (result == ImsMediaSession.RESULT_SUCCESS) {
            getMtcMediaInterface().audioSessionOpened();
        }
        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles response for Modify Session request
     *
     * @param audioConfig The AudioConfig passed in ImsMediaSession#modifySession()
     * @param result The result of modify session
     */
    public void modifySessionResponse(AudioConfig audioConfig, int result) {
        ImsLog.d("modifySessionResponse");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_MODIFY_SESSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles response for addConfig() request
     *
     * @param audioConfig The RTP config passed in ImsMediaSession#addConfig()
     * @param result The result of adding a configuration
     */
    public void addConfigResponse(AudioConfig audioConfig, int result) {
        ImsLog.d("addConfigResponse");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_ADD_CONFIG);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles response for confirmConfig() request
     *
     * @param audioConfig The RTP config passed in ImsMediaSession#confirmConfig()
     * @param result The result of confirm configuration
     */
    public void confirmConfigResponse(AudioConfig audioConfig, int result) {
        ImsLog.d("confirmConfigResponse");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_CONFIRM_CONFIG);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles indication when the first Rtp media packet is received
     *
     * @param audioConfig the remote config where media packet is received
     */
    public void firstMediaPacketReceived(AudioConfig audioConfig) {
        ImsLog.d("firstMediaPacketReceived");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.NOTIFY_FIRST_PACKET);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Called when RTP header extension received from the other party
     *
     * @param rtpExtensions List of received RTP header extensions
     */
    public void headerExtensionReceived(List<RtpHeaderExtension> rtpExtensions) {
        ImsLog.d("headerExtensionReceived");

        getMtcMediaInterface()
                .rtpHeaderExtensionsReceived(rtpExtensions.stream().collect(Collectors.toSet()));
    }

    /**
     * A notification is sent when an incoming audio dtmf is received.
     *
     * @param dtmfDigit Received incoming dtmf digit
     * @param durationMs Dtmf tone playback time in milliseconds
     */
    public void onNotifyIncomingDtmfReceived(int dtmfDigit, int durationMs) {
        ImsLog.v("onNotifyIncomingDtmfReceived: dtmfDigit= " + dtmfDigit + " duration= "
                + durationMs);

        getMtcMediaInterface().onNotifyIncomingDtmfReceived(dtmfDigit, durationMs);
    }

    /**
     * Handles notification when media quality change observed as per thresholds set by
     * setMediaQualityThreshold()
     *
     * @param qualityStatus The object of MediaQualityStatus with the rtp and the rtcp statistics.
     */
    public void onNotifyMediaQualityStatus(int accessNetwork, MediaQualityStatus qualityStatus) {
        ImsLog.d("onNotifyMediaQualityStatus");

        // Send Rtp/Rtcp Inactivity information to native
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.NOTIFY_MEDIA_INACTIVITY);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(qualityStatus.getRtpInactivityTimeMillis());
        parcel.writeInt(qualityStatus.getRtcpInactivityTimeMillis());
        getMtcMediaInterface().sendRequest(parcel);

        // Send MediaQualityStatus to frameworks
        getMtcMediaInterface().mediaQualityStatusChanged(ImsMediaSession.SESSION_TYPE_AUDIO,
                accessNetwork, qualityStatus);
    }

    /**
     * Get the media threshold information for specific session type
     *
     * @param mediaSessionType media session type for this Threshold info.
     * @return MediaThreshold media threshold information
     */
    public MediaThreshold getMediaThreshold(int mediaSessionType) {
        ImsLog.d("getMediaThreshold for sessionType[" + mediaSessionType + "]");

        return getMtcMediaInterface().getMediaThreshold(mediaSessionType);
    }

    /**
     * Handles notification when a change to call quality is occurred
     *
     * @param callQuality The call quality statistics since last report
     */
    public void callQualityChanged(CallQuality callQuality) {
        ImsLog.d("callQualityChanged");
        getMtcMediaInterface().callQualityChanged(callQuality);
    }

    /**
     * Handles notification when the AudioSession is closed
     */
    public void closeSessionResponse() {
        ImsLog.d("closeSessionResponse");
        getMtcMediaInterface().audioSessionClosed();
    }

    /**
     * Handles notification when the ImsMedia service is disconnected
     */
    public void nofityMediaDetach() {
        ImsLog.d("nofityMediaDetach");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.NOTIFY_MEDIA_DETACH);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles Qos notification
     *
     * @param remoteAddress Remote address
     * @param remotePort Remote port
     * @param result QoS connection result
     */
    public void onNotifyQosInfo(@NonNull String remoteAddress, int remotePort, boolean result) {
        ImsLog.d("onNotifyQosInfo remoteAddress= " + remoteAddress + " remotePort= " + remotePort
                + " result= " + result);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_QOS_INFO);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeString(remoteAddress);
        parcel.writeInt(remotePort);
        parcel.writeBoolean(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles indication when triggerAnbrQuery is received
     *
     * @param mediaType is used to identify media stream such as audio or video.
     * @param direction of this packet stream (e.g. uplink or downlink).
     * @param bitsPerSecond This value is the bitrate requested by the other party UE through RTP
     *     CMR, RTCPAPP or TMMBR, and ImsStack converts this value to the MAC bitrate (defined in
     *     TS36.321, range: 0 ~ 8000 kbit/s).
     */
    public void triggerAnbrQuery(int mediaType, int direction, int bitsPerSecond) {
        ImsLog.d("triggerAnbrQuery");

        getMtcMediaInterface().triggerAnbrQuery(mediaType, direction, bitsPerSecond);
    }

    /**
     * Notify received Anbr parameters to media logic
     *
     * @param mediaType is used to identify media stream such as audio or video.
     * @param direction of this packet stream (e.g. uplink or downlink).
     * @param bitsPerSecond This value is the bitrate requested by the other party UE through RTP
     *     CMR, RTCPAPP or TMMBR, and ImsStack converts this value to the MAC bitrate (defined in
     *     TS36.321, range: 0 ~ 8000 kbit/s).
     */
    public void notifyAnbrReceived(int mediaType, int direction, int bitsPerSecond) {
        ImsLog.d("notifyAnbrReceived - NOTIFY_ANBR_RECEIVED");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.NOTIFY_ANBR_RECEIVED);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(mediaType);
        parcel.writeInt(direction);
        parcel.writeInt(bitsPerSecond);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles the notification of the rtp reception stats
     *
     * @param stats The object of the RtpReceptionStats
     */
    public void onNotifyRtpReceptionStats(final RtpReceptionStats stats) {
        ImsLog.d("onNotifyRtpReceptionStats: stats= " + stats);

        getMtcMediaInterface().onNotifyRtpReceptionStats(ImsMediaSession.SESSION_TYPE_AUDIO, stats);
    }
}
