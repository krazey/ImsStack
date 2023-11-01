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
package com.android.imsstack.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;

import androidx.test.filters.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.Arrays;

@RunWith(JUnit4.class)
public class SimUtilsTest {
    private static final byte[] DATA_BYTES = {
            (byte) 0xD4, 0x0D, 0x02, 0x02, (byte) 0x82, (byte) 0x81, 0x0A, 0x07,
            (byte) 0x96, 0x00, 0x23, 0x36, 0x38, 0x36, 0x23
    };
    private static final String DATA_HEX_STRING = "D40D020282810A0796002336383623";
    private static final String DATA_PLMN = "310120";
    private static final String DATA_PLMN_BCD = "131002";
    private static final String DATA_PLMN_5DIGITS = "31012";
    private static final String DATA_PLMN_5DIGITS_BCD = "1310F2";

    @Test
    @SmallTest
    public void bytesToHexString() throws Exception {
        String testResult = SimUtils.bytesToHexString(new byte[0]);
        assertEquals(0, testResult.length());

        testResult = SimUtils.bytesToHexString(DATA_BYTES);
        assertEquals(DATA_HEX_STRING, testResult);
    }

    @Test
    @SmallTest
    public void hexStringToBytes() throws Exception {
        assertNull(SimUtils.hexStringToBytes(null));
        assertTrue(Arrays.equals(new byte[0], SimUtils.hexStringToBytes("")));
        assertTrue(Arrays.equals(DATA_BYTES, SimUtils.hexStringToBytes(DATA_HEX_STRING)));
    }

    @Test
    @SmallTest
    public void hexCharToInt() throws Exception {
        assertEquals(0, SimUtils.hexCharToInt('0'));
        assertEquals(1, SimUtils.hexCharToInt('1'));
        assertEquals(2, SimUtils.hexCharToInt('2'));
        assertEquals(3, SimUtils.hexCharToInt('3'));
        assertEquals(4, SimUtils.hexCharToInt('4'));
        assertEquals(5, SimUtils.hexCharToInt('5'));
        assertEquals(6, SimUtils.hexCharToInt('6'));
        assertEquals(7, SimUtils.hexCharToInt('7'));
        assertEquals(8, SimUtils.hexCharToInt('8'));
        assertEquals(9, SimUtils.hexCharToInt('9'));

        assertEquals(10, SimUtils.hexCharToInt('a'));
        assertEquals(11, SimUtils.hexCharToInt('b'));
        assertEquals(12, SimUtils.hexCharToInt('c'));
        assertEquals(13, SimUtils.hexCharToInt('d'));
        assertEquals(14, SimUtils.hexCharToInt('e'));
        assertEquals(15, SimUtils.hexCharToInt('f'));

        assertEquals(10, SimUtils.hexCharToInt('A'));
        assertEquals(11, SimUtils.hexCharToInt('B'));
        assertEquals(12, SimUtils.hexCharToInt('C'));
        assertEquals(13, SimUtils.hexCharToInt('D'));
        assertEquals(14, SimUtils.hexCharToInt('E'));
        assertEquals(15, SimUtils.hexCharToInt('F'));

        assertThrows(RuntimeException.class, () -> {
            SimUtils.hexCharToInt('G');
        });
    }

    @Test
    @SmallTest
    public void stringToBcdString() throws Exception {
        assertNull(SimUtils.stringToBcdString(null));
        assertEquals("", SimUtils.stringToBcdString(""));
        assertEquals(DATA_PLMN_BCD, SimUtils.stringToBcdString(DATA_PLMN));
        assertEquals(DATA_PLMN_5DIGITS_BCD, SimUtils.stringToBcdString(DATA_PLMN_5DIGITS));
    }

    @Test
    @SmallTest
    public void containsWildValue() throws Exception {
        assertFalse(SimUtils.containsWildValue(null));
        assertFalse(SimUtils.containsWildValue(""));
        assertFalse(SimUtils.containsWildValue("1-234567890"));
        assertFalse(SimUtils.containsWildValue("1-(234)-567890"));
        assertTrue(SimUtils.containsWildValue("1234567890;123"));
        assertTrue(SimUtils.containsWildValue("1234567890,123"));
        assertTrue(SimUtils.containsWildValue("1234567890N"));
    }

    @Test
    @SmallTest
    public void contains12KeysOnly() throws Exception {
        assertFalse(SimUtils.contains12KeysOnly(null));
        assertFalse(SimUtils.contains12KeysOnly(""));
        assertFalse(SimUtils.contains12KeysOnly("1-234567890"));
        assertFalse(SimUtils.contains12KeysOnly("1-(234)-567890"));
        assertTrue(SimUtils.contains12KeysOnly("1234567890"));
        assertTrue(SimUtils.contains12KeysOnly("#686#"));
        assertTrue(SimUtils.contains12KeysOnly("*1372"));
        assertTrue(SimUtils.contains12KeysOnly("#*1372*#"));
    }
}
