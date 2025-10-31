/*
 * Copyright (C) 2025 The Android Open Source Project
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

/**
 * Represents the attributes of an audio codec, such as bitrate and bandwidth.
 * This class is used to convey detailed codec parameters.
 */
public class AudioCodecAttributes implements Parcelable {
    public float mBitrateKbps;
    public float mBitrateStartKbps;
    public float mBitrateEndKbps;
    public float mBandwidthKhz;
    public float mBandwidthStartKhz;
    public float mBandwidthEndKhz;

    public AudioCodecAttributes() {
        mBitrateKbps = 0.0f;
        mBitrateStartKbps = 0.0f;
        mBitrateEndKbps = 0.0f;
        mBandwidthKhz = 0.0f;
        mBandwidthStartKhz = 0.0f;
        mBandwidthEndKhz = 0.0f;
    }

    public AudioCodecAttributes(AudioCodecAttributes audioCodecAttributes) {
        mBitrateKbps = audioCodecAttributes.mBitrateKbps;
        mBitrateStartKbps = audioCodecAttributes.mBitrateStartKbps;
        mBitrateEndKbps = audioCodecAttributes.mBitrateEndKbps;
        mBandwidthKhz = audioCodecAttributes.mBandwidthKhz;
        mBandwidthStartKhz = audioCodecAttributes.mBandwidthStartKhz;
        mBandwidthEndKhz = audioCodecAttributes.mBandwidthEndKhz;
    }

    public AudioCodecAttributes(Parcel source) {
        mBitrateKbps = source.readFloat();
        mBitrateStartKbps = source.readFloat();
        mBitrateEndKbps = source.readFloat();
        mBandwidthKhz = source.readFloat();
        mBandwidthStartKhz = source.readFloat();
        mBandwidthEndKhz = source.readFloat();
    }

    public AudioCodecAttributes(float bitrateKbps, float bitrateStartKbps, float bitrateEndKbps,
            float bandwidthKhz, float bandwidthStartKhz, float bandwidthEndKhz) {
        mBitrateKbps = bitrateKbps;
        mBitrateStartKbps = bitrateStartKbps;
        mBitrateEndKbps = bitrateEndKbps;
        mBandwidthKhz = bandwidthKhz;
        mBandwidthStartKhz = bandwidthStartKhz;
        mBandwidthEndKhz = bandwidthEndKhz;
    }

    /**
     * Copies the attributes of another {@link AudioCodecAttributes} object to this object.
     *
     * @param audioCodecAttributes The source object to copy from.
     */
    public void copyFrom(AudioCodecAttributes audioCodecAttributes) {
        mBitrateKbps = audioCodecAttributes.mBitrateKbps;
        mBitrateStartKbps = audioCodecAttributes.mBitrateStartKbps;
        mBitrateEndKbps = audioCodecAttributes.mBitrateEndKbps;
        mBandwidthKhz = audioCodecAttributes.mBandwidthKhz;
        mBandwidthStartKhz = audioCodecAttributes.mBandwidthStartKhz;
        mBandwidthEndKhz = audioCodecAttributes.mBandwidthEndKhz;
    }

    @Override
    public String toString() {
        return "AudioCodecAttributes: "
                + "mBitrateKbps: " + mBitrateKbps
                + " mBitrateStartKbps: " + mBitrateStartKbps
                + " mBitrateEndKbps: " + mBitrateEndKbps
                + " mBandwidthKhz: " + mBandwidthKhz
                + " mBandwidthStartKhz: " + mBandwidthStartKhz
                + " mBandwidthEndKhz: " + mBandwidthEndKhz
                + "}";
    }

    /**
     * Reads the object's members from a Parcel.
     *
     * @param source The Parcel to read from.
     */
    public void readFromParcel(Parcel source) {
        mBitrateKbps = source.readFloat();
        mBitrateStartKbps = source.readFloat();
        mBitrateEndKbps = source.readFloat();
        mBandwidthKhz = source.readFloat();
        mBandwidthStartKhz = source.readFloat();
        mBandwidthEndKhz = source.readFloat();
    }

    /**
     * Writes the object's members to a Parcel.
     *
     * @param dest The Parcel to write to.
     * @param flags Additional flags about how the object should be written.
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeFloat(mBitrateKbps);
        dest.writeFloat(mBitrateStartKbps);
        dest.writeFloat(mBitrateEndKbps);
        dest.writeFloat(mBandwidthKhz);
        dest.writeFloat(mBandwidthStartKhz);
        dest.writeFloat(mBandwidthEndKhz);
    }

    /** {@inheritDoc} */
    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * Parcelable creator.
     */
    public static final Parcelable.Creator<AudioCodecAttributes> CREATOR =
            new Parcelable.Creator<AudioCodecAttributes>() {
                public AudioCodecAttributes createFromParcel(Parcel source) {
                    return new AudioCodecAttributes(source);
                }
                public AudioCodecAttributes[] newArray(int size) {
                    return new AudioCodecAttributes[size];
                }
            };
}
