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

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;

public class MediaInfo implements Parcelable {
    // Audio Quality
    public static final int AUDIO_QUALITY_NONE = 0;
    public static final int AUDIO_QUALITY_AMR_NB = 1;
    public static final int AUDIO_QUALITY_AMR_WB = 2;
    public static final int AUDIO_QUALITY_EVS = 3;
    public static final int AUDIO_QUALITY_G711_PCMU = 4;
    public static final int AUDIO_QUALITY_G711_PCMA = 5;
    public static final int AUDIO_QUALITY_EVS_NB = 6;
    public static final int AUDIO_QUALITY_EVS_WB = 7;
    public static final int AUDIO_QUALITY_EVS_SWB = 8;
    public static final int AUDIO_QUALITY_EVS_FB = 9;
    public static final int AUDIO_QUALITY_MAX = 10;

    public static final int AUDIO_QUALITY_NOTUSED = 99;

    // Video Quality
    public static final int VIDEO_QUALITY_NONE = 0;
    public static final int VIDEO_QUALITY_QCIF = 1;
    public static final int VIDEO_QUALITY_QVGA_LS = 2;
    public static final int VIDEO_QUALITY_QVGA_PR = 3;
    public static final int VIDEO_QUALITY_VGA_LS = 4;
    public static final int VIDEO_QUALITY_VGA_PR = 5;
    public static final int VIDEO_QUALITY_CIF_LS = 6;
    public static final int VIDEO_QUALITY_CIF_PR = 7;
    public static final int VIDEO_QUALITY_QCIF_PR = 8;    //not using in UI right now
    public static final int VIDEO_QUALITY_SQCIF_LS = 9;
    public static final int VIDEO_QUALITY_SQCIF_PR = 10;   //not using in UI right now
    public static final int VIDEO_QUALITY_SIF_LS = 11;
    public static final int VIDEO_QUALITY_SIF_PR = 12;
    public static final int VIDEO_QUALITY_HD_LS = 13;
    public static final int VIDEO_QUALITY_HD_PR = 14;
    public static final int VIDEO_QUALITY_NOTUSED = 99;

    // Direction
    public static final int DIRECTION_INVALID = -1;
    public static final int DIRECTION_INACTIVE = 0;
    public static final int DIRECTION_RECEIVE = 1;
    public static final int DIRECTION_SEND = 2;
    public static final int DIRECTION_SEND_RECEIVE = 3;

    // GTTMode
    public static final int GTTMODE_INVALID = -1;
    public static final int GTTMODE_INACTIVE = 0;
    public static final int GTTMODE_FULL = 1;
    public static final int GTTMODE_HCO = 2;
    public static final int GTTMODE_VCO = 3;

    public int audioQuality;
    public int videoQuality;
    public int audioDir;
    public int videoDir;
    public int textDir;
    public int gttMode;

    private AudioCodecAttributes mAudioCodecAttributes;

    /**
     * Default constructor. Initializes media attributes to default inactive/invalid values.
     */
    public MediaInfo() {
        audioQuality = AUDIO_QUALITY_NONE;
        videoQuality = VIDEO_QUALITY_NONE;
        audioDir = DIRECTION_INACTIVE;
        videoDir = DIRECTION_INVALID;
        textDir = DIRECTION_INVALID;
        gttMode = GTTMODE_INVALID;
    }

    /**
     * Copy constructor.
     *
     * @param mediaInfo The {@link MediaInfo} object to copy from.
     */
    public MediaInfo(MediaInfo mediaInfo) {
        audioQuality = mediaInfo.audioQuality;
        videoQuality = mediaInfo.videoQuality;
        audioDir = mediaInfo.audioDir;
        videoDir = mediaInfo.videoDir;
        textDir = mediaInfo.textDir;
        gttMode = mediaInfo.gttMode;
        if (mediaInfo.getAudioCodecAttributes() != null) {
            mAudioCodecAttributes = new AudioCodecAttributes(mediaInfo.getAudioCodecAttributes());
        }
    }

    /**
     * Constructor for creating an instance from a Parcel.
     *
     * @param source The Parcel to read the object's data from.
     */
    public MediaInfo(Parcel source) {
        audioQuality = source.readInt();
        videoQuality = source.readInt();
        audioDir = source.readInt();
        videoDir = source.readInt();
        textDir = source.readInt();
        gttMode = source.readInt();
        // Only transfer AudioCodecAttributes from the Native to the Java.
        mAudioCodecAttributes = new AudioCodecAttributes(source);
    }

    /**
     * Constructs a new MediaInfo object with specified media attributes.
     *
     * @param audioQuality The audio quality.
     * @param videoQuality The video quality.
     * @param audioDir The audio direction.
     * @param videoDir The video direction.
     * @param textDir The text direction.
     * @param gttMode The GTT mode.
     */
    public MediaInfo(int audioQuality, int videoQuality,
            int audioDir, int videoDir, int textDir, int gttMode) {
        this.audioQuality = audioQuality;
        this.videoQuality = videoQuality;
        this.audioDir = audioDir;
        this.videoDir = videoDir;
        this.textDir = textDir;
        this.gttMode = gttMode;
    }

    /**
     * Constructs a new MediaInfo object with specified media and audio codec attributes.
     *
     * @param audioQuality The audio quality.
     * @param videoQuality The video quality.
     * @param audioDir The audio direction.
     * @param videoDir The video direction.
     * @param textDir The text direction.
     * @param gttMode The GTT mode.
     * @param audioCodecAttributes The audio codec attributes.
     */
    public MediaInfo(int audioQuality, int videoQuality,
            int audioDir, int videoDir, int textDir, int gttMode,
            @NonNull AudioCodecAttributes audioCodecAttributes) {
        this.audioQuality = audioQuality;
        this.videoQuality = videoQuality;
        this.audioDir = audioDir;
        this.videoDir = videoDir;
        this.textDir = textDir;
        this.gttMode = gttMode;
        mAudioCodecAttributes = new AudioCodecAttributes(audioCodecAttributes);
    }

    /**
     * Gets the audio codec-specific attributes.
     * @return The {@link AudioCodecAttributes}, or {@code null} if not set.
     */
    public @Nullable AudioCodecAttributes getAudioCodecAttributes() {
        return mAudioCodecAttributes;
    }

    /**
     * Sets the audio codec-specific attributes.
     *
     * @param audioCodecAttributes The {@link AudioCodecAttributes} to set.
     */
    public void setAudioCodecAttributes(@NonNull AudioCodecAttributes audioCodecAttributes) {
        this.mAudioCodecAttributes = audioCodecAttributes;
    }

    /**
     * Updates the attributes of this {@code MediaInfo} instance with the values from another.
     *
     * This method copies all media attributes from the provided {@code mediaInfo} object.
     * Note that for the audio codec attributes, this performs a deep copy by creating a new
     * {@link AudioCodecAttributes} instance if the source object contains one.
     *
     * @param mediaInfo The {@link MediaInfo} object to copy attributes from.
     */
    public void update(MediaInfo mediaInfo) {
        audioQuality = mediaInfo.audioQuality;
        videoQuality = mediaInfo.videoQuality;
        audioDir = mediaInfo.audioDir;
        videoDir = mediaInfo.videoDir;
        textDir = mediaInfo.textDir;
        gttMode = mediaInfo.gttMode;
        if (mediaInfo.getAudioCodecAttributes() != null) {
            mAudioCodecAttributes = new AudioCodecAttributes(mediaInfo.getAudioCodecAttributes());
        }

        logLn("update");
    }

    public void logLn(String tag) {
        ImsLog.i(tag + " - audioQuality : " + audioQuality
                + " videoQuality : " + videoQuality
                + " audioDir : " + audioDir
                + " videoDir : " + videoDir
                + " textDir : " + textDir
                + " gttMode : " + gttMode
                 );
    }

    /**
     * Reads the object's values from a Parcel.
     *
     * @param source The Parcel to read from.
     */
    public void readFromParcel(Parcel source) {
        audioQuality = source.readInt();
        videoQuality = source.readInt();
        audioDir = source.readInt();
        videoDir = source.readInt();
        textDir = source.readInt();
        gttMode = source.readInt();
        // Only transfer AudioCodecAttributes from the Native to the Java.
        mAudioCodecAttributes = new AudioCodecAttributes(source);

        logLn("read");
    }

    public void writeToParcel(Parcel dest, int flags) {
        logLn("write");

        dest.writeInt(audioQuality);
        dest.writeInt(videoQuality);
        dest.writeInt(audioDir);
        dest.writeInt(videoDir);
        dest.writeInt(textDir);
        dest.writeInt(gttMode);
        // The AudioCodecAttributes object is intentionally not included in the parcel because
        // it is only used for providing information to the call framework, not for passing
        // data to the native layer.
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<MediaInfo> CREATOR =
            new Parcelable.Creator<MediaInfo>() {
        public MediaInfo createFromParcel(Parcel source) {
            return new MediaInfo(source);
        }

        public MediaInfo[] newArray(int size) {
            return new MediaInfo[size];
        }
    };
}
