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

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;

public class MediaInfo implements Parcelable {

    public int            AQuality;
    public int            VQuality;
    public int            ADir;
    public int            VDir;
    public int            TDir;
    public int            GTTMode;

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

    //------------------------------------------------------------------------------------------//
    public MediaInfo() {
        AQuality = AUDIO_QUALITY_NONE;
        VQuality = VIDEO_QUALITY_NONE;
        ADir = DIRECTION_INACTIVE;
        VDir = DIRECTION_INVALID;
        TDir = DIRECTION_INVALID;
        GTTMode = GTTMODE_INVALID;

        logLn("init");
    }
    public MediaInfo(MediaInfo mediaInfo) {
        AQuality = mediaInfo.AQuality;
        VQuality = mediaInfo.VQuality;
        ADir = mediaInfo.ADir;
        VDir = mediaInfo.VDir;
        TDir = mediaInfo.TDir;
        GTTMode = mediaInfo.GTTMode;

        logLn("init");
    }

    public MediaInfo(Parcel source) {
        readFromParcel(source);
    }

    public MediaInfo(int _AQuality, int _VQuality, int _ADir, int _VDir, int _TDir, int _GTTMode) {
        AQuality = _AQuality;
        VQuality = _VQuality;
        ADir = _ADir;
        VDir = _VDir;
        TDir = _TDir;
        GTTMode = _GTTMode;

        logLn("init");
    }

    public void update(MediaInfo mediaInfo) {
        AQuality = mediaInfo.AQuality;
        VQuality = mediaInfo.VQuality;
        ADir = mediaInfo.ADir;
        VDir = mediaInfo.VDir;
        TDir = mediaInfo.TDir;
        GTTMode = mediaInfo.GTTMode;

        logLn("update");
    }

    public void logLn(String tag) {
        ImsLog.i(tag + " - AQuality : " + AQuality
                + " VQuality : " + VQuality
                + " ADir : " + ADir
                + " VDir : " + VDir
                + " TDir : " + TDir
                + " GTTMode : " + GTTMode
                 );
    }

    public void readFromParcel(Parcel source) {
        AQuality = source.readInt();
        VQuality = source.readInt();
        ADir = source.readInt();
        VDir = source.readInt();
        TDir = source.readInt();
        GTTMode = source.readInt();

        logLn("read");
    }

    public void writeToParcel(Parcel dest, int flags) {
        logLn("write");

        dest.writeInt(AQuality);
        dest.writeInt(VQuality);
        dest.writeInt(ADir);
        dest.writeInt(VDir);
        dest.writeInt(TDir);
        dest.writeInt(GTTMode);
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
