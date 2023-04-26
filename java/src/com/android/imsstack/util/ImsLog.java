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

import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;

/**
 * This class provides an interface to output the logs.
 */
public final class ImsLog {
    public static final boolean DBG = !"user".equals(android.os.Build.TYPE);

    private static final int OPT_MEDIUM_SERIAL = 0x00010000;
    private static final int OPT_HIDE_PRIVACY = 0x00000100;
    private static final int OPT_CONFIGURABLE_ALL = 0x000FFFFF;
    private static final int OPT_DEBUG_ON = 0x10000000;

    private static int sOption = 0;

    /** Sends a {@link Log#VERBOSE} log message. */
    public static final void v(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.v(Log.TAG, getMessage(log));
        }
    }

    /** Sends a {@link Log#VERBOSE} log message with a slot id. */
    public static final void v(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.v(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#DEBUG} log message. */
    public static final void d(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.d(Log.TAG, getMessage(log));
        }
    }

    /** Sends a {@link Log#DEBUG} log message with a slot id. */
    public static final void d(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.d(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#INFO} log message. */
    public static final void i(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_I)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.i(Log.TAG, getMessage(log));
        }
    }

    /** Sends a {@link Log#INFO} log message with a slot id. */
    public static final void i(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_I)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.i(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#WARN} log message. */
    public static final void w(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.w(Log.TAG, getMessage(log));
        }
    }

    /** Sends a {@link Log#WARN} log message with a slot id. */
    public static final void w(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.w(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#ERROR} log message. */
    public static final void e(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage(log));
        }
    }

    /** Sends a {@link Log#ERROR} log message with a slot id. */
    public static final void e(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#ERROR} log message with an exception. */
    public static final void e(String log, Throwable t) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage(log), t);
        }
    }

    /** Sends a {@link Log#ERROR} log message with an exception and a slot id. */
    public static final void e(int slotId, String log, Throwable t) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage("[" + slotId + "] " + log), t);
        }
    }

    /** Checks if the debug mode is enabled. */
    public static boolean isDebuggable() {
        return isOptionEnabled(OPT_DEBUG_ON);
    }

    /** Checks if the log is enabled or not. */
    public static boolean isLogEnabled() {
        return (sOption & OPT_MEDIUM_SERIAL) != 0;
    }

    /** Check if the level of logging is enabled or not. */
    public static boolean isOptionEnabled(int option) {
        return (sOption & option) != 0;
    }

    /** Initializes the logging options from the configuration. */
    public static final void init() {
        int option = LogUtils.getLogOptions(0);
        boolean debugOn = LogUtils.isDebugOn(0);

        Log.d(Log.TAG, "ImsLog :: option=0x" + Integer.toHexString(option) + ", debug=" + debugOn);

        setLogOption(option);

        if (!debugOn) {
            debugOn = DBG;
        }

        setDebugOn(debugOn);
    }

    /** Sets the log enabled. */
    @VisibleForTesting
    public static void setLogEnabled(boolean enabled) {
        if (enabled) {
            setOption(OPT_MEDIUM_SERIAL);
        } else {
            clearOption(OPT_MEDIUM_SERIAL);
        }
    }

    /** Sets the debug mode. */
    public static void setDebugOn(boolean debugOn) {
        if (debugOn) {
            setOption(OPT_DEBUG_ON);
        } else {
            clearOption(OPT_DEBUG_ON);
        }

        Log.setImsDebug(debugOn);
    }

    /**
     * Returns a string that is a substring of this string.
     * The substring begins at the specified number {@code count} of characters
     * from the end and extends to the end of this string.
     *
     * @param s The string to be evaluated.
     * @param count The number of characters from the end.
     * @return A substring.
     */
    public static String lastSubString(String s, int count) {
        return Log.lastSubString(s, count);
    }

    /**
     * Returns a string that is a substring of this string.
     * The substring begins at the next position of the last occurrence of
     * the specified delimiter {@code delimiter} and extends to the end of this string.
     *
     * @param s The string to be evaluated.
     * @param delimiter The delimiter to find a start position.
     * @return A substring.
     */
    public static String lastSubString(String s, String delimiter) {
        return Log.lastSubString(s, delimiter);
    }

    /**
     * Returns a string that is a substring of this string.
     * The substring begins at the specified {@code start} and
     * extends to the character at index {@code end -1}.
     *
     * @param s The string to be evaluated.
     * @param start The start index, inclusive.
     * @param end The end index, exclusive.
     * @return A substring.
     */
    public static String subString(String s, int start, int end) {
        return Log.subString(s, start, end);
    }

    /**
     * Returns the log string based on the conditions:
     * If the input argument is
     *     - null: returns {@link Log#NULL}
     *     - empty: returns {@link Log#EMPTY}
     *     - non-debug mode: returns {@link Log#HIDDEN}
     *     - debug mode: returns the input argument
     */
    public static String hiddenString(String s) {
        if (s == null) {
            return Log.NULL;
        } else if (s.isEmpty()) {
            return Log.EMPTY;
        }

        return isDebuggable() ? s : Log.HIDDEN;
    }

    /**
     * Returns the log string based on the conditions:
     * If the input argument is
     *     - null: returns {@link Log#NULL}
     *     - non-debug mode: returns the length of this array
     *     - debug mode: returns the formatted string for this array
     */
    public static String hiddenString(String[] s) {
        return isDebuggable() ?
                Arrays.toString(s) : ((s != null) ? String.valueOf(s.length) : Log.NULL);
    }

    private static String getMessage(String log) {
        StackTraceElement[] elements = (new Throwable()).getStackTrace();
        StackTraceElement ste = elements[2];

        return "[" + lastSubString(ste.getClassName(), ".")
                + "::" + ste.getMethodName()
                + ":" + ste.getLineNumber()
                + "] " + log;
    }

    private static void clearOption(int option) {
        sOption &= (~option);
    }

    private static void setOption(int option) {
        sOption |= option;
    }

    private static void setLogOption(int option) {
        clearOption(OPT_CONFIGURABLE_ALL);
        setOption(option);

        Log.w(Log.TAG, "LOG is " + (isLogEnabled() ? "ON" : "OFF")
            + " :: D=" + isOptionEnabled(LogUtils.TRACE_OPTION_D)
            + ", E=" + isOptionEnabled(LogUtils.TRACE_OPTION_E)
            + ", I=" + isOptionEnabled(LogUtils.TRACE_OPTION_I)
            + ", HidePrivacy=" + isOptionEnabled(OPT_HIDE_PRIVACY));
    }

    private ImsLog() {}
}
