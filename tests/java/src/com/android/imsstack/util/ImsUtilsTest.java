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
}
