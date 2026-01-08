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
    public static final boolean DBG = !ImsUtils.IS_USER;

    private static final int OPT_MEDIUM_SERIAL = 0x00010000;
    private static final int OPT_HIDE_PRIVACY = 0x00000100;
    private static final int OPT_CONFIGURABLE_ALL = 0x000FFFFF;
    private static final int OPT_DEBUG_ON = 0x10000000;
    private static final String CLASS_PRIVATE_LOG_PREFIX = "log";

    private static int sOption = 0;

    /** Sends a {@link Log#VERBOSE} log message with a specified object. */
    public static void v(Object o, String log) {
        if (isLoggable(Log.TRACE_OPTION_D)) {
            Log.v(null, getMessage(o, log));
        }
    }

    /** Sends a {@link Log#VERBOSE} log message with a specified object and a slot id. */
    public static void v(Object o, int slotId, String log) {
        if (isLoggable(Log.TRACE_OPTION_D)) {
            Log.v(null, getMessage(o, "[s" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#DEBUG} log message with a specified object. */
    public static void d(Object o, String log) {
        if (isLoggable(Log.TRACE_OPTION_D)) {
            Log.d(null, getMessage(o, log));
        }
    }

    /** Sends a {@link Log#DEBUG} log message with a specified object and a slot id. */
    public static void d(Object o, int slotId, String log) {
        if (isLoggable(Log.TRACE_OPTION_D)) {
            Log.d(null, getMessage(o, "[s" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#INFO} log message with a specified object. */
    public static void i(Object o, String log) {
        if (isLoggable(Log.TRACE_OPTION_I)) {
            Log.i(null, getMessage(o, log));
        }
    }

    /** Sends a {@link Log#INFO} log message with a slot id. */
    public static void i(Object o, int slotId, String log) {
        if (isLoggable(Log.TRACE_OPTION_I)) {
            Log.i(null, getMessage(o, "[s" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#WARN} log message with a specified object. */
    public static void w(Object o, String log) {
        if (isLoggable(Log.TRACE_OPTION_E)) {
            Log.w(null, getMessage(o, log));
        }
    }

    /** Sends a {@link Log#WARN} log message with a specified object and a slot id. */
    public static void w(Object o, int slotId, String log) {
        if (isLoggable(Log.TRACE_OPTION_E)) {
            Log.w(null, getMessage(o, "[s" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#ERROR} log message with a specified object. */
    public static void e(Object o, String log) {
        if (isLoggable(Log.TRACE_OPTION_E)) {
            Log.e(null, getMessage(o, log));
        }
    }

    /** Sends a {@link Log#ERROR} log message with a specified object and slot id. */
    public static void e(Object o, int slotId, String log) {
        if (isLoggable(Log.TRACE_OPTION_E)) {
            Log.e(null, getMessage(o, "[s" + slotId + "] " + log));
        }
    }

    /** Sends a {@link Log#ERROR} log message with a specified object and an exception. */
    public static void e(Object o, String log, Throwable t) {
        if (isLoggable(Log.TRACE_OPTION_E)) {
            Log.e(null, getMessage(o, log), t);
        }
    }

    /**
     * Sends a {@link Log#ERROR} log message with a specified object, an exception
     * and a slot id.
     */
    public static void e(Object o, int slotId, String log, Throwable t) {
        if (isLoggable(Log.TRACE_OPTION_E)) {
            Log.e(null, getMessage(o, "[s" + slotId + "] " + log), t);
        }
    }

    /** Sends a {@link Log#VERBOSE} log message. */
    public static void v(String log) {
        v(null, log);
    }

    /** Sends a {@link Log#VERBOSE} log message with a slot id. */
    public static void v(int slotId, String log) {
        v(null, slotId, log);
    }

    /** Sends a {@link Log#DEBUG} log message. */
    public static void d(String log) {
        d(null, log);
    }

    /** Sends a {@link Log#DEBUG} log message with a slot id. */
    public static void d(int slotId, String log) {
        d(null, slotId, log);
    }

    /** Sends a {@link Log#INFO} log message. */
    public static void i(String log) {
        i(null, log);
    }

    /** Sends a {@link Log#INFO} log message with a slot id. */
    public static void i(int slotId, String log) {
        i(null, slotId, log);
    }

    /** Sends a {@link Log#WARN} log message. */
    public static void w(String log) {
        w(null, log);
    }

    /** Sends a {@link Log#WARN} log message with a slot id. */
    public static void w(int slotId, String log) {
        w(null, slotId, log);
    }

    /** Sends a {@link Log#ERROR} log message. */
    public static void e(String log) {
        e(null, log);
    }

    /** Sends a {@link Log#ERROR} log message with a slot id. */
    public static void e(int slotId, String log) {
        e(null, slotId, log);
    }

    /** Sends a {@link Log#ERROR} log message with an exception. */
    public static void e(String log, Throwable t) {
        e(null, log, t);
    }

    /** Sends a {@link Log#ERROR} log message with an exception and a slot id. */
    public static void e(int slotId, String log, Throwable t) {
        e(null, slotId, log, t);
    }

    /** Checks if the debug mode is enabled. */
    public static boolean isDebuggable() {
        return isOptionEnabled(OPT_DEBUG_ON);
    }

    /** Checks if the log is enabled or not. */
    public static boolean isLogEnabled() {
        return (sOption & OPT_MEDIUM_SERIAL) != 0;
    }

    /** Checks if the logging is enabled for the given log option. */
    public static boolean isLoggable(int option) {
        boolean loggable = true;
        if (option == Log.TRACE_OPTION_D
                || option == Log.TRACE_OPTION_I) {
            loggable = !isOptionEnabled(OPT_HIDE_PRIVACY);
        }
        return isLogEnabled() && isOptionEnabled(option) && loggable;
    }

    /** Check if the level of logging is enabled or not. */
    public static boolean isOptionEnabled(int option) {
        return (sOption & option) != 0;
    }

    /** Initializes the logging options from the configuration. */
    public static void init() {
        int option = Log.getLogOptions();
        boolean debugOn = Log.isImsDebugEnabled();

        Log.d(ImsLog.class, "ImsLog: option=0x" + Integer.toHexString(option)
                + ", debug=" + debugOn);

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
        return isDebuggable()
                ? Arrays.toString(s) : ((s != null) ? String.valueOf(s.length) : Log.NULL);
    }

    private static String getMessage(String log) {
        StackTraceElement[] elements = (new Throwable()).getStackTrace();
        StackTraceElement ste = Arrays.stream(elements)
                .filter(e -> !e.getClassName().endsWith(ImsLog.class.getSimpleName())
                        && !e.getMethodName().startsWith(CLASS_PRIVATE_LOG_PREFIX))
                .findFirst()
                .orElse(elements[4]);
        return log + " [@" + lastSubString(ste.getClassName(), ".")
                + "." + ste.getMethodName()
                + ":" + ste.getLineNumber();
    }

    private static String getMessage(Object o, String log) {
        if (o == null) {
            return getMessage(log);
        }
        StackTraceElement ste = Arrays.stream((new Throwable()).getStackTrace())
                .filter(e -> e.getClassName().endsWith(o.getClass().getSimpleName())
                        && !e.getMethodName().startsWith(CLASS_PRIVATE_LOG_PREFIX))
                .findFirst()
                .orElse(null);
        return ste == null
                ? log + " [@" + o.getClass().getSimpleName()
                : log + " [@" + lastSubString(ste.getClassName(), ".")
                        + "." + ste.getMethodName()
                        + ":" + ste.getLineNumber();
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

        Log.w(ImsLog.class, "LOG is " + (isLogEnabled() ? "ON" : "OFF")
                + "; D=" + isOptionEnabled(Log.TRACE_OPTION_D)
                + ", E=" + isOptionEnabled(Log.TRACE_OPTION_E)
                + ", I=" + isOptionEnabled(Log.TRACE_OPTION_I)
                + ", HidePrivacy=" + isOptionEnabled(OPT_HIDE_PRIVACY));
    }

    private ImsLog() {}
}
