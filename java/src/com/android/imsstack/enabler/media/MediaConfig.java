/**
 * Copyright (C) 2023 The Android Open Source Project
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

import static java.lang.Math.toIntExact;

import android.telephony.ims.MediaThreshold;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.RtpConfig;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;
import java.util.stream.IntStream;

/**
 * Class to set media configurations
 *
 */
public class MediaConfig {

    RtpConfig mRtpConfig;

    // Media Thresholds
    private int[] mRtpInactivityImsTimer;
    private int mRtcpInactivityTimer;
    private int[] mRtpInactivityFwkTimer;

    private int[] mRtpPacketLossRate;
    private int[] mRtpJitter;
    private boolean mNotifyCurrentStatus;
    private int[] mRtpInactivityCombinedTimer;

    // Set default values, as it's not set by frameworks or native as of now.
    private static final int RTP_PACKETLOSS_DURATION = 3000;
    private static final int RTP_HYSTERESISTIME = 3000;

    /**
     * Default constructor for MediaConfig.
     */
    public MediaConfig() {
        resetMediaConfig();
    }

    @VisibleForTesting
    void resetMediaConfig() {
        mRtpConfig = null;
        resetMediaQualityThresholds();
    }

    private void resetMediaQualityThresholds() {
        mRtpInactivityImsTimer = new int[0];
        mRtcpInactivityTimer = 0;
        mNotifyCurrentStatus = false;
        mRtpInactivityCombinedTimer = new int[0];
        resetMediaThreshold();
    }

    private void resetMediaThreshold() {
        mRtpInactivityFwkTimer = new int[0];
        mRtpPacketLossRate = new int[0];
        mRtpJitter = new int[0];
    }

    /**
     * Update Rtp Configurations of current media session
     *
     * @param rtpConfig object to Rtp Configurations
     */
    public void updateRtpConfig(RtpConfig rtpConfig) {
        if (mRtpConfig != null && mRtpConfig.getAccessNetwork() != rtpConfig.getAccessNetwork()) {
            mNotifyCurrentStatus = true;
        } else {
            mNotifyCurrentStatus = false;
        }
        ImsLog.d("updateRtpConfig - mNotifyCurrentStatus: " + mNotifyCurrentStatus);
        mRtpConfig = rtpConfig;
    }

    /**
     * Get Rtp Configurations of current media session
     *
     * @return RtpConfig object to Rtp Configurations
     */
    public RtpConfig getRtpConfig() {
        return mRtpConfig;
    }

    /**
     * Get MediaQualityThreshold for media quality status notifications
     *
     * @return MediaQualityThreshold object to set the threshold for media quality status
     *     notifications
     */
    public MediaQualityThreshold getMediaQualityThreshold() {
        return new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(mRtpInactivityCombinedTimer)
                .setRtcpInactivityTimerMillis(mRtcpInactivityTimer)
                .setRtpHysteresisTimeInMillis(RTP_HYSTERESISTIME)
                .setRtpPacketLossDurationMillis(RTP_PACKETLOSS_DURATION)
                .setRtpPacketLossRate(mRtpPacketLossRate)
                .setRtpJitterMillis(mRtpJitter)
                .setNotifyCurrentStatus(mNotifyCurrentStatus)
                .build();
    }

    /**
     * Updates Media Quality Thresholds received from Media native
     *
     * @param mediaThreshold object to set the threshold for media quality status notifications
     * @param needFwkTimer boolean to check if the telephony framework needs timer for the RTP
     *     inactivity for the QNS
     */
    public void updateMediaQualityThreshold(
            MediaQualityThreshold mediaThreshold, boolean needFwkTimer) {
        // set even if threshold values are not changed,
        // because FwkTimer may or maynot have been set

        // update Rtcp Inactivity Timer
        mRtcpInactivityTimer = mediaThreshold.getRtcpInactivityTimerMillis();
        // update Rtp Inactivity Timer
        mRtpInactivityImsTimer = mediaThreshold.getRtpInactivityTimerMillis();

        if (needFwkTimer && mRtpInactivityFwkTimer.length != 0) {
            mRtpInactivityCombinedTimer = IntStream.concat(Arrays.stream(mRtpInactivityImsTimer),
                Arrays.stream(mRtpInactivityFwkTimer)).toArray();
        } else {
            mRtpInactivityCombinedTimer =
                    Arrays.copyOf(mRtpInactivityImsTimer, mRtpInactivityImsTimer.length);
        }
    }

    /**
     * Updates Media Quality Thresholds received from frameworks
     *
     * @param mediaThreshold object to set the threshold for media quality status notifications
     */
    public void updateMediaQualityThreshold(MediaThreshold mediaThreshold) {
        if (mediaThreshold != null) {
            // update Packet Loss Rate
            mRtpPacketLossRate = Arrays.copyOf(mediaThreshold.getThresholdsRtpPacketLossRate(),
                    mediaThreshold.getThresholdsRtpPacketLossRate().length);
            // update Jitter
            mRtpJitter = Arrays.copyOf(mediaThreshold.getThresholdsRtpJitterMillis(),
                    mediaThreshold.getThresholdsRtpJitterMillis().length);
            // update Rtp Inactivity Timer
            long[] thresholds = mediaThreshold.getThresholdsRtpInactivityTimeMillis();
            try {
                mRtpInactivityFwkTimer =
                        Arrays.stream(thresholds).mapToInt(i -> toIntExact(i)).toArray();
                if (mRtpInactivityImsTimer.length == 0) {
                    mRtpInactivityCombinedTimer =
                            Arrays.copyOf(mRtpInactivityFwkTimer, mRtpInactivityFwkTimer.length);
                } else {
                    mRtpInactivityCombinedTimer = IntStream.concat(
                            Arrays.stream(mRtpInactivityImsTimer),
                            Arrays.stream(mRtpInactivityFwkTimer)).toArray();
                }
            } catch (ArithmeticException ae) {
                ImsLog.e("ArithmeticException: " + ae.getMessage());
            }
        } else {
            ImsLog.d("clear MediaThreshold");
            resetMediaThreshold();
            if (mRtpInactivityImsTimer.length != 0) {
                mRtpInactivityCombinedTimer =
                        Arrays.copyOf(mRtpInactivityImsTimer, mRtpInactivityImsTimer.length);
            } else {
                mRtpInactivityCombinedTimer = new int[0];
            }
        }
    }
}
