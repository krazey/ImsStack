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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.Date;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

@RunWith(JUnit4.class)
public class SystemUtilsTest {
    private static final String DIGEST_ALGORITHM = "MD5";
    private static final String MD5_INPUT = "password";
    private static final String MD5_HASH = "5f4dcc3b5aa765d61d8327deb882cf99";
    private static final String REGEX_UTC_TIME_FORMAT =
            "^[0-9]{4}-(?:0[1-9]|1[0-2])-(?:0[1-9]|[1-2][0-9]|3[0-1])T"
            + "(?:[0-1][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]Z$";
    private static final byte[] DATA_BYTES = { 1, 2, '+', '9', ':', 'a', '~' };
    private static final String DATA_HEX_STRING = "01022b393a617e";

    @Test
    @SmallTest
    public void calculateMessageDigest() throws Exception {
        // Unknown algorithm
        assertNull(SystemUtils.calculateMessageDigest("abc", ""));
        // Invalid inputs
        assertNull(SystemUtils.calculateMessageDigest(DIGEST_ALGORITHM, null));

        String testResult = SystemUtils.calculateMessageDigest(DIGEST_ALGORITHM, MD5_INPUT);

        assertEquals(MD5_HASH, testResult);
    }

    @Test
    @SmallTest
    public void getUtcTimeFormat() throws Exception {
        Date current = new Date();
        String testResult = SystemUtils.getUtcTimeFormat(current.getTime());
        Pattern pattern = Pattern.compile(REGEX_UTC_TIME_FORMAT);
        Matcher matcher = pattern.matcher(testResult);
        assertTrue(matcher.matches());
    }

    @Test
    @SmallTest
    public void hexStringToInt() throws Exception {
        assertEquals(-1, SystemUtils.hexStringToInt(null));
        assertEquals(-1, SystemUtils.hexStringToInt(""));
        assertEquals(-1, SystemUtils.hexStringToInt("123a"));
        assertEquals(-1, SystemUtils.hexStringToInt("FF000020"));
        assertEquals(-1, SystemUtils.hexStringToInt("0x80000000"));
        assertEquals(16, SystemUtils.hexStringToInt("00000020")); // octet
        assertEquals(0, SystemUtils.hexStringToInt("0x00000000"));
        assertEquals(32, SystemUtils.hexStringToInt("0x00000020"));
        assertEquals(0x7FFFFFFF, SystemUtils.hexStringToInt("0x7FFFFFFF"));
    }

    @Test
    @SmallTest
    public void toHexString() throws Exception {
        assertNull(SystemUtils.toHexString(null));
        assertEquals("", SystemUtils.toHexString(new byte[0]));
        assertEquals(DATA_HEX_STRING, SystemUtils.toHexString(DATA_BYTES));
    }
}
