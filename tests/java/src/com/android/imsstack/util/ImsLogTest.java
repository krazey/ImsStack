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

import java.util.Arrays;

@RunWith(JUnit4.class)
public class ImsLogTest {
    private static final int SLOT0 = 0;
    private static final String LOG_MESSAGE = "ImsLogMessage";
    private static final String LOG_MESSAGE_DISABLED = "ImsLogMessageDisabled";
    private static final String TEST = "Test";
    private static final String DELIMITER = ".";
    private static final String TEST_LOG_MESSAGE = TEST + DELIMITER + LOG_MESSAGE;

    @Test
    @SmallTest
    public void logOperations_allEnabled() throws Exception {
        setLogEnabledForAllLevels();

        Exception throwable = null;

        try {
            throw new Exception();
        } catch (Exception e) {
            throwable = e;
        }

        // Checks the logs in the logcat.
        ImsLog.v(LOG_MESSAGE + ": v");
        ImsLog.v(SLOT0, LOG_MESSAGE + ": v");
        ImsLog.d(LOG_MESSAGE + ": d");
        ImsLog.d(SLOT0, LOG_MESSAGE + ": d");
        ImsLog.i(LOG_MESSAGE + ": i");
        ImsLog.i(SLOT0, LOG_MESSAGE + ": i");
        ImsLog.w(LOG_MESSAGE + ": w");
        ImsLog.w(SLOT0, LOG_MESSAGE + ": w");
        ImsLog.e(LOG_MESSAGE + ": e");
        ImsLog.e(SLOT0, LOG_MESSAGE + ": e");
        ImsLog.e(LOG_MESSAGE + ": e w/ exception", throwable);
        ImsLog.e(SLOT0, LOG_MESSAGE + ": e w/ exception", throwable);

        ImsLog.v(this, LOG_MESSAGE + ": v");
        ImsLog.v(this, SLOT0, LOG_MESSAGE + ": v");
        ImsLog.d(this, LOG_MESSAGE + ": d");
        ImsLog.d(this, SLOT0, LOG_MESSAGE + ": d");
        ImsLog.i(this, LOG_MESSAGE + ": i");
        ImsLog.i(this, SLOT0, LOG_MESSAGE + ": i");
        ImsLog.w(this, LOG_MESSAGE + ": w");
        ImsLog.w(this, SLOT0, LOG_MESSAGE + ": w");
        ImsLog.e(this, LOG_MESSAGE + ": e");
        ImsLog.e(this, SLOT0, LOG_MESSAGE + ": e");
        ImsLog.e(this, LOG_MESSAGE + ": e w/ exception", throwable);
        ImsLog.e(this, SLOT0, LOG_MESSAGE + ": e w/ exception", throwable);
    }

    @Test
    @SmallTest
    public void logOperations_logDisabled() throws Exception {
        setLogDisabled();

        Exception throwable = null;

        try {
            throw new Exception();
        } catch (Exception e) {
            throwable = e;
        }

        // Checks the logs in the logcat.
        // "DISABLED" logs should not be displayed.
        ImsLog.v(LOG_MESSAGE_DISABLED + ": v");
        ImsLog.v(SLOT0, LOG_MESSAGE_DISABLED + ": v");
        ImsLog.d(LOG_MESSAGE_DISABLED + ": d");
        ImsLog.d(SLOT0, LOG_MESSAGE_DISABLED + ": d");
        ImsLog.i(LOG_MESSAGE_DISABLED + ": i");
        ImsLog.i(SLOT0, LOG_MESSAGE_DISABLED + ": i");
        ImsLog.w(LOG_MESSAGE_DISABLED + ": w");
        ImsLog.w(SLOT0, LOG_MESSAGE_DISABLED + ": w");
        ImsLog.e(LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(SLOT0, LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);
        ImsLog.e(SLOT0, LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);

        ImsLog.v(this, LOG_MESSAGE_DISABLED + ": v");
        ImsLog.v(this, SLOT0, LOG_MESSAGE_DISABLED + ": v");
        ImsLog.d(this, LOG_MESSAGE_DISABLED + ": d");
        ImsLog.d(this, SLOT0, LOG_MESSAGE_DISABLED + ": d");
        ImsLog.i(this, LOG_MESSAGE_DISABLED + ": i");
        ImsLog.i(this, SLOT0, LOG_MESSAGE_DISABLED + ": i");
        ImsLog.w(this, LOG_MESSAGE_DISABLED + ": w");
        ImsLog.w(this, SLOT0, LOG_MESSAGE_DISABLED + ": w");
        ImsLog.e(this, LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(this, SLOT0, LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(this, LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);
        ImsLog.e(this, SLOT0, LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);
    }

    @Test
    @SmallTest
    public void logOperations_optionDebugEnabled() throws Exception {
        setLogEnabledFor(Log.TRACE_OPTION_D);

        Exception throwable = null;

        try {
            throw new Exception();
        } catch (Exception e) {
            throwable = e;
        }

        // Checks the logs in the logcat.
        // "DISABLED" logs should not be displayed.
        ImsLog.v(LOG_MESSAGE + ": v");
        ImsLog.v(SLOT0, LOG_MESSAGE + ": v");
        ImsLog.d(LOG_MESSAGE + ": d");
        ImsLog.d(SLOT0, LOG_MESSAGE + ": d");
        ImsLog.i(LOG_MESSAGE_DISABLED + ": i");
        ImsLog.i(SLOT0, LOG_MESSAGE_DISABLED + ": i");
        ImsLog.w(LOG_MESSAGE_DISABLED + ": w");
        ImsLog.w(SLOT0, LOG_MESSAGE_DISABLED + ": w");
        ImsLog.e(LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(SLOT0, LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);
        ImsLog.e(SLOT0, LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);
    }

    @Test
    @SmallTest
    public void logOperations_optionInfoEnabled() throws Exception {
        setLogEnabledFor(Log.TRACE_OPTION_I);

        Exception throwable = null;

        try {
            throw new Exception();
        } catch (Exception e) {
            throwable = e;
        }

        // Checks the logs in the logcat.
        // "DISABLED" logs should not be displayed.
        ImsLog.v(LOG_MESSAGE_DISABLED + ": v");
        ImsLog.v(SLOT0, LOG_MESSAGE_DISABLED + ": v");
        ImsLog.d(LOG_MESSAGE_DISABLED + ": d");
        ImsLog.d(SLOT0, LOG_MESSAGE_DISABLED + ": d");
        ImsLog.i(LOG_MESSAGE + ": i");
        ImsLog.i(SLOT0, LOG_MESSAGE + ": i");
        ImsLog.w(LOG_MESSAGE_DISABLED + ": w");
        ImsLog.w(SLOT0, LOG_MESSAGE_DISABLED + ": w");
        ImsLog.e(LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(SLOT0, LOG_MESSAGE_DISABLED + ": e");
        ImsLog.e(LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);
        ImsLog.e(SLOT0, LOG_MESSAGE_DISABLED + ": e w/ exception", throwable);
    }

    @Test
    @SmallTest
    public void logOperations_optionErrorEnabled() throws Exception {
        setLogEnabledFor(Log.TRACE_OPTION_E);

        Exception throwable = null;

        try {
            throw new Exception();
        } catch (Exception e) {
            throwable = e;
        }

        // Checks the logs in the logcat.
        // "DISABLED" logs should not be displayed.
        ImsLog.v(LOG_MESSAGE_DISABLED + ": v");
        ImsLog.v(SLOT0, LOG_MESSAGE_DISABLED + ": v");
        ImsLog.d(LOG_MESSAGE_DISABLED + ": d");
        ImsLog.d(SLOT0, LOG_MESSAGE_DISABLED + ": d");
        ImsLog.i(LOG_MESSAGE_DISABLED + ": i");
        ImsLog.i(SLOT0, LOG_MESSAGE_DISABLED + ": i");
        ImsLog.w(LOG_MESSAGE + ": w");
        ImsLog.w(SLOT0, LOG_MESSAGE + ": w");
        ImsLog.e(LOG_MESSAGE + ": e");
        ImsLog.e(SLOT0, LOG_MESSAGE + ": e");
        ImsLog.e(LOG_MESSAGE + ": e w/ exception", throwable);
        ImsLog.e(SLOT0, LOG_MESSAGE + ": e w/ exception", throwable);
    }

    @Test
    @SmallTest
    public void init() throws Exception {
        setLogEnabledForAllLevels();

        assertTrue(ImsLog.isLogEnabled());
        assertTrue(ImsLog.isOptionEnabled(Log.TRACE_OPTION_D));
        assertTrue(ImsLog.isOptionEnabled(Log.TRACE_OPTION_I));
        assertTrue(ImsLog.isOptionEnabled(Log.TRACE_OPTION_E));
    }

    @Test
    @SmallTest
    public void setLogEnabled() throws Exception {
        ImsLog.setLogEnabled(true);
        assertTrue(ImsLog.isLogEnabled());

        ImsLog.setLogEnabled(false);
        assertFalse(ImsLog.isLogEnabled());
    }

    @Test
    @SmallTest
    public void setDebugOn() throws Exception {
        ImsLog.setDebugOn(true);
        assertTrue(ImsLog.isDebuggable());

        ImsLog.setDebugOn(false);
        assertFalse(ImsLog.isDebuggable());
    }

    @Test
    @SmallTest
    public void hiddenString() throws Exception {
        String refinedLogMessage;
        String nullString = null;

        refinedLogMessage = ImsLog.hiddenString(nullString);
        assertEquals(Log.NULL, refinedLogMessage);

        refinedLogMessage = ImsLog.hiddenString("");
        assertEquals(Log.EMPTY, refinedLogMessage);

        // When the debug is not enabled.
        ImsLog.setDebugOn(false);
        refinedLogMessage = ImsLog.hiddenString(LOG_MESSAGE);
        assertEquals(Log.HIDDEN, refinedLogMessage);

        // When the debug is enabled.
        ImsLog.setDebugOn(true);
        refinedLogMessage = ImsLog.hiddenString(LOG_MESSAGE);
        assertEquals(LOG_MESSAGE, refinedLogMessage);
    }

    @Test
    @SmallTest
    public void hiddenString_stringArray() throws Exception {
        String refinedLogMessage;
        String[] nullString = null;
        String[] testStrings = { TEST, LOG_MESSAGE };

        // When the debug is not enabled.
        ImsLog.setDebugOn(false);
        refinedLogMessage = ImsLog.hiddenString(nullString);
        assertEquals(Log.NULL, refinedLogMessage);

        refinedLogMessage = ImsLog.hiddenString(testStrings);
        assertEquals(String.valueOf(testStrings.length), refinedLogMessage);

        // When the debug is enabled.
        ImsLog.setDebugOn(true);
        refinedLogMessage = ImsLog.hiddenString(nullString);
        assertEquals(Arrays.toString(nullString), refinedLogMessage);

        refinedLogMessage = ImsLog.hiddenString(testStrings);
        assertEquals(Arrays.toString(testStrings), refinedLogMessage);
    }

    @Test
    @SmallTest
    public void lastSubString_delimiter() throws Exception {
        String token;

        token = ImsLog.lastSubString(null, DELIMITER);
        assertEquals(token, Log.NULL);

        token = ImsLog.lastSubString("", DELIMITER);
        assertEquals(token, Log.EMPTY);

        token = ImsLog.lastSubString(TEST_LOG_MESSAGE, DELIMITER);
        assertEquals(token, LOG_MESSAGE);

        token = ImsLog.lastSubString(TEST_LOG_MESSAGE, null);
        assertEquals(token, TEST_LOG_MESSAGE);

        token = ImsLog.lastSubString(TEST, DELIMITER);
        assertEquals(token, TEST);
    }

    @Test
    @SmallTest
    public void lastSubString_count() throws Exception {
        String token;

        token = ImsLog.lastSubString(null, 0);
        assertEquals(token, Log.NULL);

        token = ImsLog.lastSubString("", 0);
        assertEquals(token, Log.EMPTY);

        token = ImsLog.lastSubString(TEST, 0);
        assertEquals(token, "");

        token = ImsLog.lastSubString(TEST, 1);
        assertEquals(token, "t");

        token = ImsLog.lastSubString(TEST, 3);
        assertEquals(token, "est");

        token = ImsLog.lastSubString(TEST, 4);
        assertEquals(token, TEST);

        token = ImsLog.lastSubString(TEST, 5);
        assertEquals(token, TEST);
    }

    @Test
    @SmallTest
    public void subString() throws Exception {
        String token;

        token = ImsLog.subString(null, 0, 0);
        assertEquals(token, Log.NULL);

        token = ImsLog.subString("", 0, 0);
        assertEquals(token, Log.EMPTY);

        token = ImsLog.subString(TEST, 2, 1);
        assertEquals(token, Log.EMPTY);

        token = ImsLog.subString(TEST, 0, 1);
        assertEquals(token, "T");

        token = ImsLog.subString(TEST, 0, 2);
        assertEquals(token, "Te");

        token = ImsLog.subString(TEST, 1, TEST.length());
        assertEquals(token, "est");

        token = ImsLog.subString(TEST, 0, TEST.length());
        assertEquals(token, TEST);

        token = ImsLog.subString(TEST, -1, -1);
        assertEquals(token, TEST);

        token = ImsLog.subString(TEST, 0, TEST.length() + 1);
        assertEquals(token, TEST);
    }

    private void setLogDisabled() {
        Log.init(0, false);
        ImsLog.init();
    }

    private void setLogEnabledForAllLevels() {
        Log.init(ImsUtils.hexStringToInt(Log.DEFAULT_LOG_OPTIONS), false);
        ImsLog.init();
    }

    private void setLogEnabledFor(int traceOption) {
        Log.init(traceOption, false);
        ImsLog.init();
        ImsLog.setLogEnabled(true);
    }
}
