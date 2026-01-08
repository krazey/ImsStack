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

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class AudioCodecAttributesTest {

    private static final float DELTA = 0.001f;

    @Test
    public void testDefaultConstructor() {
        AudioCodecAttributes attributes = new AudioCodecAttributes();

        assertEquals(0.0f, attributes.mBitrateKbps, DELTA);
        assertEquals(0.0f, attributes.mBitrateStartKbps, DELTA);
        assertEquals(0.0f, attributes.mBitrateEndKbps, DELTA);
        assertEquals(0.0f, attributes.mBandwidthKhz, DELTA);
        assertEquals(0.0f, attributes.mBandwidthStartKhz, DELTA);
        assertEquals(0.0f, attributes.mBandwidthEndKhz, DELTA);
    }

    @Test
    public void testConstructorWithValues() {
        AudioCodecAttributes attributes = new AudioCodecAttributes(
                1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f);

        assertEquals(1.1f, attributes.mBitrateKbps, DELTA);
        assertEquals(2.2f, attributes.mBitrateStartKbps, DELTA);
        assertEquals(3.3f, attributes.mBitrateEndKbps, DELTA);
        assertEquals(4.4f, attributes.mBandwidthKhz, DELTA);
        assertEquals(5.5f, attributes.mBandwidthStartKhz, DELTA);
        assertEquals(6.6f, attributes.mBandwidthEndKhz, DELTA);
    }

    @Test
    public void testCopyConstructor() {
        AudioCodecAttributes original = new AudioCodecAttributes(
                1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f);
        AudioCodecAttributes copied = new AudioCodecAttributes(original);

        assertEquals(original.mBitrateKbps, copied.mBitrateKbps, DELTA);
        assertEquals(original.mBitrateStartKbps, copied.mBitrateStartKbps, DELTA);
        assertEquals(original.mBitrateEndKbps, copied.mBitrateEndKbps, DELTA);
        assertEquals(original.mBandwidthKhz, copied.mBandwidthKhz, DELTA);
        assertEquals(original.mBandwidthStartKhz, copied.mBandwidthStartKhz, DELTA);
        assertEquals(original.mBandwidthEndKhz, copied.mBandwidthEndKhz, DELTA);
    }

    @Test
    public void testCopyFrom() {
        AudioCodecAttributes toUpdate = new AudioCodecAttributes();
        AudioCodecAttributes source = new AudioCodecAttributes(
                1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f);

        toUpdate.copyFrom(source);

        assertEquals(source.mBitrateKbps, toUpdate.mBitrateKbps, DELTA);
        assertEquals(source.mBitrateStartKbps, toUpdate.mBitrateStartKbps, DELTA);
        assertEquals(source.mBitrateEndKbps, toUpdate.mBitrateEndKbps, DELTA);
        assertEquals(source.mBandwidthKhz, toUpdate.mBandwidthKhz, DELTA);
        assertEquals(source.mBandwidthStartKhz, toUpdate.mBandwidthStartKhz, DELTA);
        assertEquals(source.mBandwidthEndKhz, toUpdate.mBandwidthEndKhz, DELTA);
    }

    @Test
    public void testParcelReadWrite() {
        AudioCodecAttributes original = new AudioCodecAttributes(
                1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f);

        Parcel parcel = Parcel.obtain();
        original.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);

        AudioCodecAttributes fromParcel = AudioCodecAttributes.CREATOR.createFromParcel(parcel);

        assertEquals(original.mBitrateKbps, fromParcel.mBitrateKbps, DELTA);
        assertEquals(original.mBitrateStartKbps, fromParcel.mBitrateStartKbps, DELTA);
        assertEquals(original.mBitrateEndKbps, fromParcel.mBitrateEndKbps, DELTA);
        assertEquals(original.mBandwidthKhz, fromParcel.mBandwidthKhz, DELTA);
        assertEquals(original.mBandwidthStartKhz, fromParcel.mBandwidthStartKhz, DELTA);
        assertEquals(original.mBandwidthEndKhz, fromParcel.mBandwidthEndKhz, DELTA);

        parcel.recycle();
    }
}
