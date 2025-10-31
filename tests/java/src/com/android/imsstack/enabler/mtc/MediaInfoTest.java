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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class MediaInfoTest {

    private static final float DELTA = 0.001f;

    private AudioCodecAttributes createTestAudioCodecAttributes() {
        return new AudioCodecAttributes(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f);
    }

    private void assertAudioCodecAttributesEquals(
            AudioCodecAttributes expected, AudioCodecAttributes actual) {
        if (expected == null) {
            assertNull(actual);
            return;
        }
        assertEquals(expected.mBitrateKbps, actual.mBitrateKbps, DELTA);
        assertEquals(expected.mBitrateStartKbps, actual.mBitrateStartKbps, DELTA);
        assertEquals(expected.mBitrateEndKbps, actual.mBitrateEndKbps, DELTA);
        assertEquals(expected.mBandwidthKhz, actual.mBandwidthKhz, DELTA);
        assertEquals(expected.mBandwidthStartKhz, actual.mBandwidthStartKhz, DELTA);
        assertEquals(expected.mBandwidthEndKhz, actual.mBandwidthEndKhz, DELTA);
    }

    @Test
    public void testDefaultConstructor() {
        MediaInfo mediaInfo = new MediaInfo();
        assertEquals(MediaInfo.AUDIO_QUALITY_NONE, mediaInfo.audioQuality);
        assertEquals(MediaInfo.VIDEO_QUALITY_NONE, mediaInfo.videoQuality);
        assertEquals(MediaInfo.DIRECTION_INACTIVE, mediaInfo.audioDir);
        assertEquals(MediaInfo.DIRECTION_INVALID, mediaInfo.videoDir);
        assertEquals(MediaInfo.DIRECTION_INVALID, mediaInfo.textDir);
        assertEquals(MediaInfo.GTTMODE_INVALID, mediaInfo.gttMode);
        assertAudioCodecAttributesEquals(null, mediaInfo.getAudioCodecAttributes());
    }

    @Test
    public void testConstructorWithoutAudioCodecAttributes() {
        MediaInfo mediaInfo = new MediaInfo(1, 2, 3, 4, 5, 6);
        assertEquals(1, mediaInfo.audioQuality);
        assertEquals(2, mediaInfo.videoQuality);
        assertEquals(3, mediaInfo.audioDir);
        assertEquals(4, mediaInfo.videoDir);
        assertEquals(5, mediaInfo.textDir);
        assertEquals(6, mediaInfo.gttMode);
        assertAudioCodecAttributesEquals(null, mediaInfo.getAudioCodecAttributes());
    }

    @Test
    public void testConstructorWithAudioCodecAttributes() {
        AudioCodecAttributes attributes = createTestAudioCodecAttributes();
        MediaInfo mediaInfo = new MediaInfo(1, 2, 3, 4, 5, 6, attributes);

        assertEquals(1, mediaInfo.audioQuality);
        assertEquals(2, mediaInfo.videoQuality);
        assertEquals(3, mediaInfo.audioDir);
        assertEquals(4, mediaInfo.videoDir);
        assertEquals(5, mediaInfo.textDir);
        assertEquals(6, mediaInfo.gttMode);
        assertAudioCodecAttributesEquals(attributes, mediaInfo.getAudioCodecAttributes());
    }

    @Test
    public void testCopyConstructor() {
        AudioCodecAttributes attributes = createTestAudioCodecAttributes();
        MediaInfo original = new MediaInfo(1, 2, 3, 4, 5, 6, attributes);
        MediaInfo copied = new MediaInfo(original);

        assertEquals(original.audioQuality, copied.audioQuality);
        assertEquals(original.videoQuality, copied.videoQuality);
        assertEquals(original.audioDir, copied.audioDir);
        assertEquals(original.videoDir, copied.videoDir);
        assertEquals(original.textDir, copied.textDir);
        assertEquals(original.gttMode, copied.gttMode);
        assertAudioCodecAttributesEquals(
                original.getAudioCodecAttributes(), copied.getAudioCodecAttributes());
    }

    @Test
    public void testUpdate() {
        AudioCodecAttributes attributes = createTestAudioCodecAttributes();
        MediaInfo original = new MediaInfo(1, 2, 3, 4, 5, 6, attributes);
        MediaInfo toUpdate = new MediaInfo();
        toUpdate.setAudioCodecAttributes(new AudioCodecAttributes());

        toUpdate.update(original);

        assertEquals(original.audioQuality, toUpdate.audioQuality);
        assertEquals(original.videoQuality, toUpdate.videoQuality);
        assertEquals(original.audioDir, toUpdate.audioDir);
        assertEquals(original.videoDir, toUpdate.videoDir);
        assertEquals(original.textDir, toUpdate.textDir);
        assertEquals(original.gttMode, toUpdate.gttMode);
        assertAudioCodecAttributesEquals(
                original.getAudioCodecAttributes(), toUpdate.getAudioCodecAttributes());
    }

    @Test
    public void testParcelWrite() {
        AudioCodecAttributes attributes = createTestAudioCodecAttributes();
        MediaInfo original = new MediaInfo(1, 2, 3, 4, 5, 6, attributes);

        Parcel parcel = Parcel.obtain();
        original.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);

        // The expected parcel size is 24 bytes, accounting for 6 integer fields (audioQuality,
        // videoQuality, audioDir, videoDir, textDir, gttMode), each using 4 bytes.
        // The AudioCodecAttributes object is intentionally not included in the parcel because
        // it is only used for providing information to the call framework, not for passing
        // data to the native layer.
        assertEquals(24, parcel.dataSize());
        parcel.recycle();
    }

    @Test
    public void testParcelRead() {
        AudioCodecAttributes attributes = createTestAudioCodecAttributes();

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(1);
        parcel.writeInt(2);
        parcel.writeInt(3);
        parcel.writeInt(4);
        parcel.writeInt(5);
        parcel.writeInt(6);
        parcel.writeFloat(attributes.mBitrateKbps);
        parcel.writeFloat(attributes.mBitrateStartKbps);
        parcel.writeFloat(attributes.mBitrateEndKbps);
        parcel.writeFloat(attributes.mBandwidthKhz);
        parcel.writeFloat(attributes.mBandwidthStartKhz);
        parcel.writeFloat(attributes.mBandwidthEndKhz);
        parcel.setDataPosition(0);

        MediaInfo fromParcel = MediaInfo.CREATOR.createFromParcel(parcel);

        assertEquals(1, fromParcel.audioQuality);
        assertEquals(2, fromParcel.videoQuality);
        assertEquals(3, fromParcel.audioDir);
        assertEquals(4, fromParcel.videoDir);
        assertEquals(5, fromParcel.textDir);
        assertEquals(6, fromParcel.gttMode);
        assertAudioCodecAttributesEquals(attributes,
                fromParcel.getAudioCodecAttributes());

        parcel.recycle();
    }
}
