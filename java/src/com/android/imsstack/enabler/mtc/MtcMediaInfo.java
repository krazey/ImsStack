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

package com.android.imsstack.enabler.mtc;

/**
 * Wrapper class for MediaInfo class.
 */
public final class MtcMediaInfo {
    private final MediaInfo mMediaInfo;

    public MtcMediaInfo() {
        mMediaInfo = new MediaInfo(
                MediaInfo.AUDIO_QUALITY_AMR_WB,
                MediaInfo.VIDEO_QUALITY_NONE,
                MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_INVALID,
                MediaInfo.DIRECTION_INVALID,
                MediaInfo.GTTMODE_INVALID);
    }

    public MtcMediaInfo(MediaInfo mi) {
        mMediaInfo = mi;
    }

    public int getAudioQuality() {
        return getAudioQuality(mMediaInfo);
    }

    public int getAudioDirection() {
        return getAudioDirection(mMediaInfo);
    }

    public int getVideoQuality() {
        return getVideoQuality(mMediaInfo);
    }

    public int getVideoDirection() {
        return getVideoDirection(mMediaInfo);
    }

    public int getTextDirection() {
        return getTextDirection(mMediaInfo);
    }

    public int getGttMode() {
        return getGttMode(mMediaInfo);
    }

    public void setAudioQuality(int quality) {
        setAudioQuality(mMediaInfo, quality);
    }

    public void setAudioDirection(int direction) {
        setAudioDirection(mMediaInfo, direction);
    }

    public void setVideoQuality(int quality) {
        setVideoQuality(mMediaInfo, quality);
    }

    public void setVideoDirection(int direction) {
        setVideoDirection(mMediaInfo, direction);
    }

    public void setTextDirection(int direction) {
        setTextDirection(mMediaInfo, direction);
    }

    public void setGttMode(int gttMode) {
        setGttMode(mMediaInfo, gttMode);
    }

    public void updateMediaInfo(MediaInfo mi) {
        mMediaInfo.AQuality = mi.AQuality;
        mMediaInfo.ADir = mi.ADir;
        mMediaInfo.VQuality = mi.VQuality;
        mMediaInfo.VDir = mi.VDir;
        mMediaInfo.TDir = mi.TDir;
        mMediaInfo.GTTMode = mi.GTTMode;
    }

    public static int getAudioQuality(MediaInfo mi) {
        return mi.AQuality;
    }

    public static int getAudioDirection(MediaInfo mi) {
        return mi.ADir;
    }

    public static int getVideoQuality(MediaInfo mi) {
        return mi.VQuality;
    }

    public static int getVideoDirection(MediaInfo mi) {
        return mi.VDir;
    }

    public static int getTextDirection(MediaInfo mi) {
        return mi.TDir;
    }

    public static int getGttMode(MediaInfo mi) {
        return mi.GTTMode;
    }

    public static void setAudioQuality(MediaInfo mi, int quality) {
        mi.AQuality = quality;
    }

    public static void setAudioDirection(MediaInfo mi, int direction) {
        mi.ADir = direction;
    }

    public static void setVideoQuality(MediaInfo mi, int quality) {
        mi.VQuality = quality;
    }

    public static void setVideoDirection(MediaInfo mi, int direction) {
        mi.VDir = direction;
    }

    public static void setTextDirection(MediaInfo mi, int direction) {
        mi.TDir = direction;
    }

    public static void setGttMode(MediaInfo mi, int gttMode) {
        mi.GTTMode = gttMode;
    }
}
