/**
 * Copyright (C) 2024 The Android Open Source Project
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

import android.telephony.imsmedia.RtpReceptionStats;

import androidx.annotation.NonNull;

import com.android.imsstack.util.ImsLog;

public class AudioVideoSync {
    private class MarkedRtpStats {
        public long time;
        public RtpReceptionStats stats;
    }

    private static final long MAX_VALUE_32BIT = 4294967295L;
    private MarkedRtpStats mAudioStats;
    private MarkedRtpStats mVideoStats;
    private int mAudioClockRate;
    private int mVideoClockRate;
    private int mVideoDelay;
    private boolean mAudioStatUpdated;
    private boolean mVideoStatUpdated;


    public AudioVideoSync() {
        mAudioStats = null;
        mAudioClockRate = 0;
        mVideoStats = null;
        mVideoClockRate = 0;
        mVideoDelay = 0;
        mAudioStatUpdated = false;
        mVideoStatUpdated = false;
    }

    /**
     * Set the audio clock rate of the rtp timestamp
     */
    public void setAudioClockRate(int rateKHz) {
        mAudioClockRate = rateKHz;
    }

    /**
     * Set the video clock rate of the rtp timestamp
     */
    public void setVideoClockRate(int rateKHz) {
        mVideoClockRate = rateKHz;
    }

    /**
     * Set the RtpReceptionStats of the audio session
     * @param stats The rtp reception stats to set.
     */
    public void setAudioStats(@NonNull RtpReceptionStats stats) {
        if (mAudioStats == null) {
            mAudioStats = new MarkedRtpStats();
        }
        mAudioStats.time = System.currentTimeMillis();
        mAudioStatUpdated = (mAudioStats.stats == null
                || mAudioStats.stats.getRtpTimestamp() != stats.getRtpTimestamp());
        mAudioStats.stats = stats;
    }

    /**
     * Set the RtpReceptionStats of the video session
     * @param stats The rtp reception stats to set.
     */
    public void setVideoStats(@NonNull RtpReceptionStats stats) {
        if (mVideoStats == null) {
            mVideoStats = new MarkedRtpStats();
        }
        mVideoStats.time = System.currentTimeMillis();
        mVideoStatUpdated = (mVideoStats.stats == null
                || mVideoStats.stats.getRtpTimestamp() != stats.getRtpTimestamp());
        mVideoStats.stats = stats;
    }

    /**
     * Get the video delay which is how late compared with the audio delay
     */
    public int getVideoDelay() {
        calculateVideoDelay();
        return mVideoDelay;
    }

    private void calculateVideoDelay() {
        if (mAudioStats == null || mVideoStats == null
                || mAudioClockRate == 0 || mVideoClockRate == 0 || !mAudioStatUpdated
                || !mVideoStatUpdated) {
            mVideoDelay = 0;
            return;
        }
        int audioTimeDiffMs = (mAudioStats.stats.getRtcpSrTimestamp()
                - mAudioStats.stats.getRtpTimestamp()) / mAudioClockRate;
        int audioNtpTime = (int) convertNtpToMillisec(mAudioStats.stats.getRtcpSrNtpTimestamp())
                - audioTimeDiffMs - mAudioStats.stats.getJitterBufferMs();
        int videoTimeDiffMs = (mVideoStats.stats.getRtcpSrTimestamp()
                - mVideoStats.stats.getRtpTimestamp()) / mVideoClockRate;
        int videoNtpTime = (int) convertNtpToMillisec(mVideoStats.stats.getRtcpSrNtpTimestamp())
                - videoTimeDiffMs - mVideoStats.stats.getJitterBufferMs();

        long reportTimeDiff = mVideoStats.time - mAudioStats.time;
        int jbmDiff = mVideoStats.stats.getJitterBufferMs() - mAudioStats.stats.getJitterBufferMs();
        int ntpDiff = videoNtpTime - audioNtpTime;

        // use the jbm diff
        mVideoDelay += jbmDiff < 0 ? trimOff(-jbmDiff, 10, 10) : -trimOff(jbmDiff, 0, 10);
        if (mVideoDelay < 0) {
            mVideoDelay = 0;
        }
        if (mVideoDelay > 400) {
            mVideoDelay = 400;
        }
        ImsLog.d("report=" + reportTimeDiff + ", ntp=" + ntpDiff + ", jbm=" + jbmDiff);
    }

    private double convertNtpToMillisec(long ntp) {
        int ntp_high = (int) (ntp >> 32);
        int ntp_low = (int) (ntp << 32 >> 32);
        double ntpDouble = ntp_high + (double) ntp_low / MAX_VALUE_32BIT;
        return ntpDouble * 1000;
    }

    private int trimOff(int num, int offset, int denum) {
        if (denum <= 0) {
            return 0;
        }
        return (num + offset) / denum * denum;
    }

}
