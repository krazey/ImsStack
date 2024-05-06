/*
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
package com.android.imsstack.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import androidx.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.MockitoAnnotations;

import java.io.FileOutputStream;
import java.io.IOException;
import java.time.Instant;
import java.util.Arrays;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

@RunWith(JUnit4.class)
public class ImsUtilsTest {
    private static final String DIGEST_ALGORITHM = "MD5";
    private static final String MD5_INPUT = "password";
    private static final String MD5_HASH = "5f4dcc3b5aa765d61d8327deb882cf99";
    private static final String REGEX_UTC_TIME_FORMAT =
            "^[0-9]{4}-(?:0[1-9]|1[0-2])-(?:0[1-9]|[1-2][0-9]|3[0-1])T"
            + "(?:[0-1][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]Z$";
    private static final byte[] DATA_BYTES = { 1, 2, '+', '9', ':', 'a', '~' };
    private static final String DATA_HEX_STRING = "01022B393A617E";
    private static final String DATA_LOWERCASE_HEX_STRING = "01022b393a617e";
    private static final String DATA_PLMN = "310120";
    private static final String DATA_PLMN_BCD = "131002";
    private static final String DATA_PLMN_5DIGITS = "31012";
    private static final String DATA_PLMN_5DIGITS_BCD = "1310F2";

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {
        // no-op
    }

    @Test
    @SmallTest
    public void testCloseQuietly() throws Exception {
        FileOutputStream output = mock(FileOutputStream.class);
        ImsUtils.closeQuietly(null);
        ImsUtils.closeQuietly(output);

        verify(output).close();
    }

    @Test
    @SmallTest
    public void testCloseQuietlyWithRuntimeException() throws Exception {
        FileOutputStream output = mock(FileOutputStream.class);
        doThrow(new RuntimeException("close failed.")).when(output).close();

        assertThrows(RuntimeException.class, () -> {
            ImsUtils.closeQuietly(output);
        });
    }

    @Test
    @SmallTest
    public void testCloseQuietlyWithIOException() throws Exception {
        FileOutputStream output = mock(FileOutputStream.class);
        doThrow(new IOException("close failed.")).when(output).close();
        ImsUtils.closeQuietly(output);

        // Expected: exception is ignored.
        verify(output).close();
    }

    @Test
    @SmallTest
    public void testCalculateMessageDigest() {
        // Unknown algorithm
        assertNull(ImsUtils.calculateMessageDigest("abc", ""));
        // Invalid inputs
        assertNull(ImsUtils.calculateMessageDigest(DIGEST_ALGORITHM, null));

        String testResult = ImsUtils.calculateMessageDigest(DIGEST_ALGORITHM, MD5_INPUT);

        assertEquals(MD5_HASH, testResult);
    }

    @Test
    @SmallTest
    public void testGetUtcTimeFormat() {
        Instant instant = Instant.now();
        String testResult = ImsUtils.getUtcTimeFormat(instant.toEpochMilli());
        Pattern pattern = Pattern.compile(REGEX_UTC_TIME_FORMAT);
        Matcher matcher = pattern.matcher(testResult);
        assertTrue(matcher.matches());
    }

    @Test
    @SmallTest
    public void testHexStringToInt() {
        assertEquals(-1, ImsUtils.hexStringToInt(null));
        assertEquals(-1, ImsUtils.hexStringToInt(""));
        assertEquals(-1, ImsUtils.hexStringToInt("123a"));
        assertEquals(-1, ImsUtils.hexStringToInt("FF000020"));
        assertEquals(-1, ImsUtils.hexStringToInt("0x80000000"));
        assertEquals(16, ImsUtils.hexStringToInt("00000020")); // octet
        assertEquals(0, ImsUtils.hexStringToInt("0x00000000"));
        assertEquals(32, ImsUtils.hexStringToInt("0x00000020"));
        assertEquals(0x7FFFFFFF, ImsUtils.hexStringToInt("0x7FFFFFFF"));
    }

    @Test
    @SmallTest
    public void testBytesToHexString() {
        assertNull(ImsUtils.bytesToHexString(null));
        assertEquals("", ImsUtils.bytesToHexString(new byte[0]));
        assertEquals(DATA_HEX_STRING, ImsUtils.bytesToHexString(DATA_BYTES));
        assertEquals(DATA_LOWERCASE_HEX_STRING, ImsUtils.bytesToHexString(DATA_BYTES, false));
    }

    @Test
    @SmallTest
    public void testHexStringToBytes() {
        assertNull(ImsUtils.hexStringToBytes(null));
        assertTrue(Arrays.equals(new byte[0], ImsUtils.hexStringToBytes("")));
        assertTrue(Arrays.equals(DATA_BYTES, ImsUtils.hexStringToBytes(DATA_HEX_STRING)));
    }

    @Test
    @SmallTest
    public void testHexCharToInt() {
        assertEquals(0, ImsUtils.hexCharToInt('0'));
        assertEquals(1, ImsUtils.hexCharToInt('1'));
        assertEquals(2, ImsUtils.hexCharToInt('2'));
        assertEquals(3, ImsUtils.hexCharToInt('3'));
        assertEquals(4, ImsUtils.hexCharToInt('4'));
        assertEquals(5, ImsUtils.hexCharToInt('5'));
        assertEquals(6, ImsUtils.hexCharToInt('6'));
        assertEquals(7, ImsUtils.hexCharToInt('7'));
        assertEquals(8, ImsUtils.hexCharToInt('8'));
        assertEquals(9, ImsUtils.hexCharToInt('9'));

        assertEquals(10, ImsUtils.hexCharToInt('a'));
        assertEquals(11, ImsUtils.hexCharToInt('b'));
        assertEquals(12, ImsUtils.hexCharToInt('c'));
        assertEquals(13, ImsUtils.hexCharToInt('d'));
        assertEquals(14, ImsUtils.hexCharToInt('e'));
        assertEquals(15, ImsUtils.hexCharToInt('f'));

        assertEquals(10, ImsUtils.hexCharToInt('A'));
        assertEquals(11, ImsUtils.hexCharToInt('B'));
        assertEquals(12, ImsUtils.hexCharToInt('C'));
        assertEquals(13, ImsUtils.hexCharToInt('D'));
        assertEquals(14, ImsUtils.hexCharToInt('E'));
        assertEquals(15, ImsUtils.hexCharToInt('F'));

        assertThrows(RuntimeException.class, () -> {
            ImsUtils.hexCharToInt('G');
        });
    }

    @Test
    @SmallTest
    public void testStringToBcdString() {
        assertNull(ImsUtils.stringToBcdString(null));
        assertEquals("", ImsUtils.stringToBcdString(""));
        assertEquals(DATA_PLMN_BCD, ImsUtils.stringToBcdString(DATA_PLMN));
        assertEquals(DATA_PLMN_5DIGITS_BCD, ImsUtils.stringToBcdString(DATA_PLMN_5DIGITS));
    }

    @Test
    @SmallTest
    public void testHasWildValueForDialString() {
        assertFalse(ImsUtils.hasWildValueForDialString(null));
        assertFalse(ImsUtils.hasWildValueForDialString(""));
        assertFalse(ImsUtils.hasWildValueForDialString("1-234567890"));
        assertFalse(ImsUtils.hasWildValueForDialString("1-(234)-567890"));
        assertTrue(ImsUtils.hasWildValueForDialString("1234567890;123"));
        assertTrue(ImsUtils.hasWildValueForDialString("1234567890,123"));
        assertTrue(ImsUtils.hasWildValueForDialString("1234567890N"));
    }

    @Test
    @SmallTest
    public void testHas12KeysOnlyForDialString() {
        assertFalse(ImsUtils.has12KeysOnlyForDialString(null));
        assertFalse(ImsUtils.has12KeysOnlyForDialString(""));
        assertFalse(ImsUtils.has12KeysOnlyForDialString("1-234567890"));
        assertFalse(ImsUtils.has12KeysOnlyForDialString("1-(234)-567890"));
        assertTrue(ImsUtils.has12KeysOnlyForDialString("1234567890"));
        assertTrue(ImsUtils.has12KeysOnlyForDialString("#686#"));
        assertTrue(ImsUtils.has12KeysOnlyForDialString("*1372"));
        assertTrue(ImsUtils.has12KeysOnlyForDialString("#*1372*#"));
    }
}
