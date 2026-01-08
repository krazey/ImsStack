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

package com.android.imsstack.imsservice.mmtel.sms;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;

import androidx.test.filters.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class BitwiseInputStreamTest {

    @Test
    @SmallTest
    public void read() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[]{(byte) 0b10110101, (byte) 0b11001010};
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        assertEquals(1, bis.read(1));    // Reads "1"
        assertEquals(0, bis.read(1));    // Reads "0"
        assertEquals(6, bis.read(3));    // Reads "110" (value 6)
        assertEquals(1, bis.read(1));    // Reads "1"
        assertEquals(1, bis.read(2));    // Reads "01" (value 1)
        assertEquals(25, bis.read(5));   // Reads "11001" (value 25)
        assertEquals(1, bis.read(2));    // Reads "01" (value 1)
    }

    @Test
    @SmallTest
    public void readCrossByte() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[]{(byte) 0b10110101, (byte) 0b11001010};
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        bis.skip(6);

        assertEquals(0b0111, bis.read(4));
    }

    @Test
    @SmallTest
    public void readByteArray() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[]{(byte) 0x12, (byte) 0x34, (byte) 0x56, (byte) 0x78};
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        byte[] result = bis.readByteArray(16);

        assertArrayEquals(new byte[]{(byte) 0x12, (byte) 0x34}, result);

        bis.skip(4);
        result = bis.readByteArray(8);

        assertEquals(1, result.length);
        // Reads 4 bits from 0x56 ("0110") and 4 bits from 0x78 ("0111") -> "01100111" = 0x67
        assertEquals((byte) 0x67, result[0]);
    }

    @Test
    @SmallTest
    public void readByteArray_nonByteAlignedLength() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[]{(byte) 0b11110000, (byte) 0b10101010};
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        // Read 9 bits. Expect a 2-byte array.
        byte[] result = bis.readByteArray(9);

        assertEquals(2, result.length);
        // First byte should contain the first 8 bits ("11110000")
        assertEquals((byte) 0xF0, result[0]);
        // Second byte should contain the 9th bit ("1"), left-shifted by 7 ("10000000")
        assertEquals((byte) 0x80, result[1]);
    }

    @Test
    @SmallTest
    public void readByteArray_partialByteResult() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[]{(byte) 0b11111100};
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        // Read 6 bits. Expect a 1-byte array.
        byte[] result = bis.readByteArray(6);

        assertEquals(1, result.length);
        // The 6 bits "111111" (value 63) are left-shifted by 2, resulting in "11111100" (0xFC)
        assertEquals((byte) 0xFC, result[0]);
    }

    @Test
    @SmallTest
    public void available() {
        byte[] buf = new byte[10];
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        assertEquals(80, bis.available());
    }

    @Test
    @SmallTest
    public void skip() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[]{(byte) 0b11110000};
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        bis.skip(4);

        assertEquals(0, bis.read(4));
    }

    @Test
    @SmallTest
    public void readTooManyBits() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[1];
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        assertThrows(BitwiseInputStream.AccessException.class, () -> bis.read(9));
    }

    @Test
    @SmallTest
    public void readPastEnd() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[1];
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        bis.read(8);

        assertThrows(BitwiseInputStream.AccessException.class, () -> bis.read(1));
    }

    @Test
    @SmallTest
    public void skipPastEnd() throws BitwiseInputStream.AccessException {
        byte[] buf = new byte[1];
        BitwiseInputStream bis = new BitwiseInputStream(buf);

        assertThrows(BitwiseInputStream.AccessException.class, () -> bis.skip(9));
    }
}
