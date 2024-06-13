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

/**
 * This class provides the logging interface and utility methods
 * that will be helpful when adding the log statement.
 */
public final class Log {
    public static final String TAG = "ImsStack";
    public static final int DEBUG = android.util.Log.DEBUG;
    public static final String HIDDEN = "****";
    public static final String EMPTY = "(empty)";
    public static final String NULL = "(null)";
    /**
     * Logging options.
     */
    public static final int TRACE_OPTION_D = 0x00000001;
    public static final int TRACE_OPTION_E = 0x00000002;
    public static final int TRACE_OPTION_I = 0x00000004;
    // logcat + all levels(D / E / I / TEXT)
    public static final String DEFAULT_LOG_OPTIONS = "0x0001000F";

    private static int sDebug = -1;
    private static int sImsDebug = -1;
    private static int sLogOptions = -1;

    static {
        if (!ImsUtils.IS_USER) {
            sDebug = 1;
        }
    }

    /**
     * Initializes the logging configuration.
     *
     * @param logOptions A logging options.
     * @param debugEnabled Flag specifying whether the debug option is enabled or not.
     */
    public static void init(int logOptions, boolean debugEnabled) {
        setLogOptions(logOptions);
        setImsDebug(debugEnabled);
    }

    /**
     * Checks whether the debug mode is enabled or not.
     */
    public static boolean isDebuggable() {
        if (sDebug == 1 || sImsDebug == 1) {
            return true;
        } else if (sDebug == -1) {
            return isLoggable(DEBUG);
        }

        return false;
    }

    /**
     * Checks whether the logging is enabled for the given log level.
     */
    public static boolean isLoggable(int level) {
        return android.util.Log.isLoggable(TAG, level);
    }

    /**
     * Prints the debug level log.
     */
    public static void d(String tag, String msg) {
        if (isLogEnabled(TRACE_OPTION_D)) {
            android.util.Log.d(tag, msg);
        }
    }

    /**
     * Prints the error level log.
     */
    public static void e(String tag, String msg) {
        if (isLogEnabled(TRACE_OPTION_E)) {
            android.util.Log.e(tag, msg);
        }
    }

    /**
     * Prints the error level log.
     */
    public static void e(String tag, String msg, Throwable t) {
        if (isLogEnabled(TRACE_OPTION_E)) {
            android.util.Log.e(tag, msg, t);
        }
    }

    /**
     * Prints the information level log.
     */
    public static void i(String tag, String msg) {
        if (isLogEnabled(TRACE_OPTION_I)) {
            android.util.Log.i(tag, msg);
        }
    }

    /**
     * Prints the verbose level log.
     */
    public static void v(String tag, String msg) {
        if (isLogEnabled(TRACE_OPTION_D)) {
            android.util.Log.v(tag, msg);
        }
    }

    /**
     * Prints the warning level log.
     */
    public static void w(String tag, String msg) {
        if (isLogEnabled(TRACE_OPTION_I)) {
            android.util.Log.w(tag, msg);
        }
    }

    /**
     * Prints the debug level log when the debug mode is enabled.
     */
    public static void dc(String tag, String msg) {
        if (isDebuggable()) {
            d(tag, msg);
        }
    }

    /**
     * Prints the verbose level log when the debug mode is enabled.
     */
    public static void vc(String tag, String msg) {
        if (isDebuggable()) {
            v(tag, msg);
        }
    }

    /**
     * Returns a logging string depending on whether it's debuggable or not.
     *
     * PII : Personally Identifiable Information
     */
    public static String pii(String s) {
        if (s == null) {
            return NULL;
        } else if (s.isEmpty()) {
            return EMPTY;
        } else if (isDebuggable()) {
            return s;
        }

        return HIDDEN;
    }

    /** Sets the debuggable value after checking this tag is enabled in the debug mode. */
    public static void setDebuggable() {
        sDebug = isLoggable(DEBUG) ? 1 : 0;
    }

    /** Checks whether the IMS debug is enabled or not. */
    /* package */ static boolean isImsDebugEnabled() {
        return sImsDebug == 1;
    }

    /** Sets the debuggable value from the specified flag. */
    public static void setImsDebug(boolean imsDebug) {
        sImsDebug = imsDebug ? 1 : 0;
    }

    /** Returns the log options. */
    /* package */ static int getLogOptions() {
        return sLogOptions;
    }

    /** Sets the log options. */
    public static void setLogOptions(int logOptions) {
        sLogOptions = logOptions;
    }

    private static boolean isLogEnabled(int option) {
        if (sLogOptions == -1) {
            // All logs are available as default if it's not initialized.
            return true;
        }
        return (sLogOptions & option) == option;
    }

    /**
     * Returns a string that is a substring of this string.
     * The substring begins at the start of this string and
     * extends to the previous position of specified delimiter. Or,
     * the substring begins at the next poistion of the specified delimiter and
     * extends to the end of this string.
     *
     * @param s The string to be evaluated.
     * @param delimiter The delimiter to find a start position.
     * @param leftToken The flag specifying which token will be taken.
     *                  Returns the string on the left by delimiter if true,
     *                  otherwise the string on the right.
     * @return A substring.
     */
    public static String firstSubString(String s, String delimiter, boolean leftToken) {
        if (s == null) {
            return NULL;
        } else if (s.isEmpty()) {
            return EMPTY;
        } else {
            int index = (delimiter == null) ? (-1) : s.indexOf(delimiter);
            if (index < 0) {
                return s;
            }
            return leftToken ? s.substring(0, index) : s.substring(index + 1, s.length());
        }
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
        if (s == null) {
            return NULL;
        } else if (s.isEmpty()) {
            return EMPTY;
        } else {
            if (s.length() < count) {
                return s;
            }

            return subString(s, s.length() - count, s.length());
        }
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
        if (s == null) {
            return NULL;
        } else if (s.isEmpty()) {
            return EMPTY;
        } else {
            int index = (delimiter == null) ? (-1) : s.lastIndexOf(delimiter);
            return (index < 0) ? s : s.substring(index + 1, s.length());
        }
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
        if (s == null) {
            return NULL;
        } else if (s.isEmpty()) {
            return EMPTY;
        } else {
            start = ((start < 0) || (start > s.length())) ? 0 : start;
            end = ((end < 0) || (end > s.length())) ? s.length() : end;

            if (start > end) {
                return EMPTY;
            }

            return s.substring(start, end);
        }
    }

    private Log() {}
}
