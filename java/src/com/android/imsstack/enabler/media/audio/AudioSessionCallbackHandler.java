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
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityStatus;

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
        ImsLog.v("Constructor - Exit");
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
        ImsLog.v("modifySessionResponse");

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
        ImsLog.v("addConfigResponse");

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
        ImsLog.v("confirmConfigResponse");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_CONFIRM_CONFIG);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles indication when the first Rtp media packet is received
     * @param audioConfig the remote config where media packet is received
     */
    public void firstMediaPacketReceived(AudioConfig audioConfig) {
        ImsLog.v("firstMediaPacketReceived");

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
        ImsLog.v("headerExtensionReceived");

        getMtcMediaInterface().rtpHeaderExtensionsReceived(
                rtpExtensions.stream().collect(Collectors.toSet()));
    }

    /**
     * Handles notification when media quality change observed as per thresholds set by
     * setMediaQualityThreshold()
     *
     * @param qualityStatus The object of MediaQualityStatus with the rtp and the rtcp statistics.
     */
    public void onNotifyMediaQualityStatus(MediaQualityStatus qualityStatus) {
        ImsLog.v("onNotifyMediaQualityStatus");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.NOTIFY_MEDIA_INACTIVITY);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(qualityStatus.getRtpInactivityTimeMillis());
        parcel.writeInt(qualityStatus.getRtcpInactivityTimeMillis());

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles notification when a change to call quality is occurred
     *
     * @param callQuality The call quality statistics since last report
     */
    public void callQualityChanged(CallQuality callQuality) {
        ImsLog.v("callQualityChanged");
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
        ImsLog.v("nofityMediaDetach");

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
        ImsLog.v("onNotifyQosInfo remoteAddress= " + remoteAddress + " remotePort= " + remotePort
                + " result= " + result);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_QOS_INFO);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeString(remoteAddress);
        parcel.writeInt(remotePort);
        parcel.writeBoolean(result);

        getMtcMediaInterface().sendRequest(parcel);
    }
}