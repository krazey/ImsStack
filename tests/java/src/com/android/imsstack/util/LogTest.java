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
import static org.junit.Assert.assertTrue;

import androidx.test.filters.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class LogTest {
    private static final String LOG_MESSAGE = "LogMessage";
    private static final String TEST = "Test";
    private static final String DELIMITER = ".";
    private static final String TEST_LOG_MESSAGE = TEST + DELIMITER + LOG_MESSAGE;

    @Test
    @SmallTest
    public void testInit() {
        int logOptions = ImsUtils.hexStringToInt(Log.DEFAULT_LOG_OPTIONS);
        Log.init(logOptions, true);

        assertTrue(Log.isImsDebugEnabled());
        assertEquals(logOptions, Log.getLogOptions());
    }

    @Test
    @SmallTest
    public void logOperations() throws Exception {
        Log.setImsDebug(true);

        Exception throwable = null;

        try {
            throw new Exception();
        } catch (Exception e) {
            throwable = e;
        }

        // Checks the logs in the logcat.
        Log.d(this, LOG_MESSAGE + ": d");
        Log.e(this, LOG_MESSAGE + ": e");
        Log.e(this, LOG_MESSAGE + ": e w/ exception", throwable);
        Log.i(this, LOG_MESSAGE + ": i");
        Log.v(this, LOG_MESSAGE + ": v");
        Log.w(this, LOG_MESSAGE + ": w");
    }

    @Test
    @SmallTest
    public void debug_conditional() throws Exception {
        // When the debug is not enabled.
        Log.setDebuggable();
        Log.setImsDebug(false);
        Log.dc(this, LOG_MESSAGE);

        // When sImsDebug is true.
        Log.setImsDebug(true);
        Log.dc(this, LOG_MESSAGE + ": dc");
    }

    @Test
    @SmallTest
    public void verbose_conditional() throws Exception {
        // When the debug is not enabled.
        Log.setDebuggable();
        Log.setImsDebug(false);
        Log.vc(this, LOG_MESSAGE);

        // When sImsDebug is true.
        Log.setImsDebug(true);
        Log.vc(this, LOG_MESSAGE + ": vc");
    }

    @Test
    @SmallTest
    public void pii() throws Exception {
        Log.setDebuggable();
        String refinedLogMessage;

        refinedLogMessage = Log.pii(null);
        assertEquals(Log.NULL, refinedLogMessage);

        refinedLogMessage = Log.pii("");
        assertEquals(Log.EMPTY, refinedLogMessage);

        // When the debug is not enabled.
        Log.setImsDebug(false);
        refinedLogMessage = Log.pii(LOG_MESSAGE);
        assertEquals(Log.HIDDEN, refinedLogMessage);

        // When the debug is enabled.
        Log.setImsDebug(true);
        refinedLogMessage = Log.pii(LOG_MESSAGE);
        assertEquals(LOG_MESSAGE, refinedLogMessage);
    }

    @Test
    @SmallTest
    public void setDebuggable() throws Exception {
        // Precondition: disable all debug conditions.
        Log.setDebuggable();
        Log.setImsDebug(false);

        assertFalse(Log.isDebuggable());
    }

    @Test
    @SmallTest
    public void setImsDebug() throws Exception {
        // Precondition: disable all debug conditions.
        Log.setDebuggable();
        Log.setImsDebug(false);

        // When the debug is enabled.
        Log.setImsDebug(true);
        assertTrue(Log.isDebuggable());

        // When the debug is not enabled.
        Log.setImsDebug(false);
        assertFalse(Log.isDebuggable());
    }

    @Test
    @SmallTest
    public void firstSubString() throws Exception {
        String token;

        token = Log.firstSubString(null, DELIMITER, true);
        assertEquals(token, Log.NULL);

        token = Log.firstSubString("", DELIMITER, true);
        assertEquals(token, Log.EMPTY);

        token = Log.firstSubString(TEST_LOG_MESSAGE, DELIMITER, true);
        assertEquals(token, TEST);

        token = Log.firstSubString(TEST_LOG_MESSAGE, DELIMITER, false);
        assertEquals(token, LOG_MESSAGE);

        token = Log.firstSubString(TEST_LOG_MESSAGE, null, true);
        assertEquals(token, TEST_LOG_MESSAGE);

        token = Log.firstSubString(TEST, DELIMITER, true);
        assertEquals(token, TEST);

        token = Log.firstSubString(TEST, DELIMITER, false);
        assertEquals(token, TEST);
    }

    @Test
    @SmallTest
    public void lastSubString_count() throws Exception {
        String token;

        token = Log.lastSubString(null, 0);
        assertEquals(token, Log.NULL);

        token = Log.lastSubString("", 0);
        assertEquals(token, Log.EMPTY);

        token = Log.lastSubString(TEST, 0);
        assertEquals(token, "");

        token = Log.lastSubString(TEST, 1);
        assertEquals(token, "t");

        token = Log.lastSubString(TEST, 3);
        assertEquals(token, "est");

        token = Log.lastSubString(TEST, 4);
        assertEquals(token, TEST);

        token = Log.lastSubString(TEST, 5);
        assertEquals(token, TEST);
    }

    @Test
    @SmallTest
    public void lastSubString_delimiter() throws Exception {
        String token;

        token = Log.lastSubString(null, DELIMITER);
        assertEquals(token, Log.NULL);

        token = Log.lastSubString("", DELIMITER);
        assertEquals(token, Log.EMPTY);

        token = Log.lastSubString(TEST_LOG_MESSAGE, DELIMITER);
        assertEquals(token, LOG_MESSAGE);

        token = Log.lastSubString(TEST_LOG_MESSAGE, null);
        assertEquals(token, TEST_LOG_MESSAGE);

        token = Log.lastSubString(TEST, DELIMITER);
        assertEquals(token, TEST);
    }

    @Test
    @SmallTest
    public void subString() throws Exception {
        String token;

        token = Log.subString(null, 0, 0);
        assertEquals(token, Log.NULL);

        token = Log.subString("", 0, 0);
        assertEquals(token, Log.EMPTY);

        token = Log.subString(TEST, 2, 1);
        assertEquals(token, Log.EMPTY);

        token = Log.subString(TEST, 0, 1);
        assertEquals(token, "T");

        token = Log.subString(TEST, 0, 2);
        assertEquals(token, "Te");

        token = Log.subString(TEST, 1, TEST.length());
        assertEquals(token, "est");

        token = Log.subString(TEST, 0, TEST.length());
        assertEquals(token, TEST);

        token = Log.subString(TEST, -1, -1);
        assertEquals(token, TEST);

        token = Log.subString(TEST, 0, TEST.length() + 1);
        assertEquals(token, TEST);
    }
}
