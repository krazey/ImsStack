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
import android.telephony.ims.RtpHeaderExtension;

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
     * Notified when the call qualtiy changed
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
}
