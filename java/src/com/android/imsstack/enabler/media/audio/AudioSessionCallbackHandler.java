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
import android.telephony.imsmedia.MediaQualityThreshold;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.util.ImsLog;
import java.util.List;

/**
 * This handles callbacks received from {@link ImsMediaManager} and passes it to
 * ImsStack (Media Enabler Native)
 */
public class AudioSessionCallbackHandler {

    private final MediaSession mMediaSession;

    public AudioSessionCallbackHandler(@NonNull final MediaSession mediaSession){
        mMediaSession = mediaSession;
        ImsLog.v("Constructor - Exit");
    }

    private final MediaSession getMediaSession() {
        return mMediaSession;
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
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(result);

        getMediaSession().sendRequest(parcel);
    }

    /**
     * Handles response when any change occurs to the RTP session
     *
     * @param state session state
     */
    public void sessionChanged(int state) {
        ImsLog.v("sessionChanged");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.RESPONSE_SESSION_CHANGED);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(state);

        getMediaSession().sendRequest(parcel);
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

        getMediaSession().sendRequest(parcel);
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

        getMediaSession().sendRequest(parcel);
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

        getMediaSession().sendRequest(parcel);
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

        getMediaSession().sendRequest(parcel);
    }

    /**
     * Called when RTP header extension received from the other party
     *
     * @param rtpExtensions List of received RTP header extensions
     */
    public void headerExtensionReceived(List<RtpHeaderExtension> rtpExtensions) {
        ImsLog.v("headerExtensionReceived");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_HEADER_EXTENSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(rtpExtensions.size());
        if (!rtpExtensions.isEmpty()) {
            for (int i = 0; i < rtpExtensions.size(); ++i) {
                rtpExtensions.get(i).writeToParcel(parcel,
                    Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
            }
        }

        getMediaSession().sendRequest(parcel);
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
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(packetType);

        getMediaSession().sendRequest(parcel);
    }

    /**
     * Handles notification when RTP packet loss observed as per thresholds set by
     * setMediaQualityThreshold()
     *
     * @param packetLossPercentage percentage of packet loss calculated over the duration
     */
    public void onNotifyPacketLoss(int packetLossPercentage) {
        ImsLog.v("onNotifyPacketLoss");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_PACKET_LOSS);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(packetLossPercentage);

        getMediaSession().sendRequest(parcel);
    }

    /**
     * Handles notification when RTP jitter observed as per thresholds set by
     * setMediaQualityThreshold()
     *
     * @param jitter jitter of the RTP packets in milliseconds calculated over the duration
     */
    public void onNotifyJitter(int jitter) {
        ImsLog.v("onNotifyJitter");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_JITTER);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeInt(jitter);

        getMediaSession().sendRequest(parcel);
    }

    /**
     * Handles notification when a change to media quality is occurred
     *
     * @param callQuality The media quality statistics since last report
     */
    public void mediaQualityChanged(CallQuality callQuality) {
        ImsLog.v("mediaQualityChanged");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(MediaConstants.NOTIFY_MEDIA_QUALITY_CHANGE);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        callQuality.writeToParcel(parcel,  Parcelable.PARCELABLE_WRITE_RETURN_VALUE);

        getMediaSession().sendRequest(parcel);
    }
}