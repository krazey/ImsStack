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
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.VideoConfig;

import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.util.ImsLog;

/**
 * This handles Video Session callbacks received from {@link ImsMediaManager} and passes it to
 * ImsStack (Media Enabler Native)
 */
public class VideoSessionCallbackHandler {

    private final IMtcMediaInterface mMtcMediaInterface;

    public VideoSessionCallbackHandler(@NonNull final IMtcMediaInterface mtcMediaInterface) {
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
        ImsLog.d("openSession Result=" + result);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles response for Modify Session request
     *
     * @param videoConfig The VideoConfig passed in ImsMediaSession#modifySession()
     * @param result The result of modify session
     */
    public void modifySessionResponse(VideoConfig videoConfig, int result) {
        ImsLog.v("modifySessionResponse");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.RESPONSE_MODIFY_SESSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        videoConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles indication when the first Rtp media packet is received
     *
     * @param videoConfig the remote config where media packet is received
     */
    public void firstMediaPacketReceived(VideoConfig videoConfig) {
        ImsLog.v("firstMediaPacketReceived");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_FIRST_PACKET);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        videoConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles notification when media inactivity observed as per thresholds set by
     * setMediaQualityThreshold()
     *
     * @param packetType either RTP or RTCP
     */
    public void onNotifyMediaInactivity(int packetType) {
        ImsLog.v("onNotifyMediaInactivity");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_MEDIA_INACTIVITY);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        parcel.writeInt(packetType);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles notification when the video bitrate decreased below the threshold set by
     * setMediaQualityThreshold() API
     *
     * @param bitrate The bitrate of sending video packets in bps unit
     */
    public void onNotifyBitrate(int bitrate) {
        ImsLog.v("onNotifyBitrate");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_BITRATE);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        parcel.writeInt(bitrate);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles notification of accumulated video data usage in the current session.
     *
     * @param bytes bytes of send and received rtp video data.
     */
    public void onNotifyVideoDataUsage(long bytes) {
        ImsLog.v("onNotifyVideoDataUsage: size=" + bytes);
        // TODO: send it to video provider
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
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        parcel.writeString(remoteAddress);
        parcel.writeInt(remotePort);
        parcel.writeBoolean(result);

        getMtcMediaInterface().sendRequest(parcel);
    }
}
