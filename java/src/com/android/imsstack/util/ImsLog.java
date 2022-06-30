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

import android.text.TextUtils;

import java.util.Arrays;

public final class ImsLog {
    public static final boolean DBG = !"user".equals(android.os.Build.TYPE);

    private static final int OPT_MEDIUM_SERIAL = 0x00010000;
    private static final int OPT_HIDE_PRIVACY = 0x00000100;
    private static final int OPT_DEFAULT = 0x0001000F;
    private static final int OPT_CONFIGURABLE_ALL = 0x000FFFFF;
    private static final int OPT_DEBUG_ON = 0x10000000;

    private static int sOption = 0;

    public static final void v(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.v(Log.TAG, getMessage(log));
        }
    }

    public static final void v(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.v(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    public static final void d(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.d(Log.TAG, getMessage(log));
        }
    }

    public static final void d(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_D)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.d(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    public static final void i(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_I)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.i(Log.TAG, getMessage(log));
        }
    }

    public static final void i(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_I)
                && isOptionEnabled(OPT_HIDE_PRIVACY) == false) {
            Log.i(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    public static final void w(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.w(Log.TAG, getMessage(log));
        }
    }

    public static final void w(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.w(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    public static final void e(String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage(log));
        }
    }

    public static final void e(int slotId, String log) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage("[" + slotId + "] " + log));
        }
    }

    public static final void e(String log, Throwable t) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage(log), t);
        }
    }

    public static final void e(int slotId, String log, Throwable t) {
        if (isLogEnabled() && isOptionEnabled(LogUtils.TRACE_OPTION_E)) {
            Log.e(Log.TAG, getMessage("[" + slotId + "] " + log), t);
        }
    }

    public static final void p(String format, String ... args) {
        if (args == null) {
            return;
        }

        if (args.length <= 0) {
            return;
        }

        if (isLogEnabled() == false
                || isOptionEnabled(LogUtils.TRACE_OPTION_D) == false
                || isOptionEnabled(OPT_HIDE_PRIVACY) == true) {
            return;
        }

        for (int i = 0 ; i < args.length ; i++) {

            if (TextUtils.isEmpty(args[i]) == true) {
                continue;
            }

            format = format.replaceFirst("%[sS]", args[i]);
        }
        Log.d(Log.TAG, getMessage(format));
    }

    public static boolean isDebuggable() {
        return DBG || isOptionEnabled(OPT_DEBUG_ON);
    }

    public static boolean isLogEnabled() {
        return (sOption & OPT_MEDIUM_SERIAL) != 0;
    }

    public static boolean isOptionEnabled(int option) {
        return (sOption & option) != 0;
    }

    // Initialize the trace option from the configuration
    public static final void init() {
        int option = LogUtils.getLogOptions(0);
        boolean debugOn = LogUtils.isDebugOn(0);

        Log.d(Log.TAG, "ImsLog :: option=0x" + Integer.toHexString(option) + ", debug=" + debugOn);

        setLogOption(option);
        setDebugOn(debugOn);

        Log.setImsDebug(debugOn);
    }

    public static void setDebugOn(boolean debugOn) {
        if (debugOn) {
            setOption(OPT_DEBUG_ON);
        } else {
            clearOption(OPT_DEBUG_ON);
        }
    }

    public static String lastSubString(String s, int count) {
        return Log.lastSubString(s, count);
    }

    public static String lastSubString(String s, String delimiter) {
        return Log.lastSubString(s, delimiter);
    }

    public static String subString(String s, int start, int end) {
        return Log.subString(s, start, end);
    }

    // If string is hidden, then it returns "xxx"
    public static String hiddenString(String s) {
        if (s == null) {
            return "(null)";
        } else if (s.isEmpty()) {
            return "(empty)";
        }

        return isDebuggable() ? s : "xxx";
    }

    public static String hiddenString(String[] s) {
        return isDebuggable() ?
                Arrays.toString(s) : ((s != null) ? String.valueOf(s.length) : "(null)");
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

    /**
     * Ims privileged logging utilities
     */
    public static class Priv {
        public static final void v(String log) {
            Log.v(Log.TAG, getPrefix(log));
        }

        public static final void d(String log) {
            Log.d(Log.TAG, getPrefix(log));
        }

        public static final void i(String log) {
            Log.i(Log.TAG, getPrefix(log));
        }

        public static final void w(String log) {
            Log.w(Log.TAG, getPrefix(log));
        }

        public static final void e(String log) {
            Log.e(Log.TAG, getPrefix(log));
        }

        public static final void e(String log, Throwable t) {
            Log.e(Log.TAG, getPrefix(log), t);
        }

        private static String getPrefix(String log) {
            StackTraceElement[] elements = (new Throwable()).getStackTrace();
            StackTraceElement ste = elements[2];

            return "[" + lastSubString(ste.getClassName(), ".")
                    + "::" + ste.getMethodName()
                    + ":" + ste.getLineNumber()
                    + "] " + log;
        }
    }
}
