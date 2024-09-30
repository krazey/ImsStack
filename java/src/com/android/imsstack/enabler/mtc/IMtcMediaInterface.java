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

package com.android.imsstack.enabler.mtc;

import android.os.Parcel;
import android.telephony.CallQuality;
import android.telephony.ims.MediaThreshold;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.MediaQualityStatus;
import android.telephony.imsmedia.RtpReceptionStats;

import com.android.imsstack.enabler.media.IMediaListener;

import java.util.Set;

/**
 * Interface to interact with MtcMediaSession
 */
public interface IMtcMediaInterface {
    /**
     * Sets MediaSession listener to MtcMediaSession
     * @param listener to media
     */
    void setMediaListener(IMediaListener listener);
    /**
     * Sends parcel to media native
     * @param parcel to send
     */
    void sendRequest(Parcel parcel);
    /**
     * Notified when the remote party has sent text message via RTT
     * @param rttMessage String containing the received characters.
     */
    void rttMessageReceived(String rttMessage);

    /**
     * Notified when the audio session opened
     */
    void audioSessionOpened();

    /**
     * Notified when the audio session closed
     */
    void audioSessionClosed();

    /**
     * Notified when the call quality changed
     * @param callQuality Defined in android.telephony.CallQuality
     */
    void callQualityChanged(CallQuality callQuality);

    /**
     * Notifies when the remote party has sent RTP header extension data
     * @param extensions the RTP header extension data
     */
    void rtpHeaderExtensionsReceived(Set<RtpHeaderExtension> extensions);

    /**
     * Notified when the video session opened
     */
    void videoSessionOpened();

    /**
     * Notified when the audio session got the notification of the rtp reception stats
     * @param type The media session type
     * @param stats The rtp reception stats for the av sync
     */
    void onNotifyRtpReceptionStats(int type, RtpReceptionStats stats);

    /**
     * Get the media threshold information for specific session type
     * @param mediaSessionType media session type for this Threshold info.
     * @return MediaThreshold media threshold information
     */
    MediaThreshold getMediaThreshold(int mediaSessionType);

    /**
     * Notified when the media quality status change
     * @param mediaSessionType media session type for this MediaQualityStatus info.
     * @param accessNetwork Access Network type
     * @param mediaQualityStatus Defined in android.telephony.ims
     */
    void mediaQualityStatusChanged(int mediaSessionType, int accessNetwork,
            MediaQualityStatus mediaQualityStatus);

    /**
     * Trigger Anbr query to discuss with the network whether the current media bitrate
     * can be changed after receiving cmr.
     * @param mediaType is used to identify media stream such as audio or video.
     * @param direction  of this packet stream (e.g. uplink or downlink).
     * @param bitsPerSecond This value is the bitrate requested by the other party UE.
     */
    void triggerAnbrQuery(int mediaType, int direction, int bitsPerSecond);

    /**
     * A notification is sent when an incoming audio dtmf is received.
     * @param dtmfDigit Received incoming dtmf digit
     * @param durationMs Dtmf tone playback time
     */
    void onNotifyIncomingDtmfReceived(int dtmfDigit, int durationMs);
}
