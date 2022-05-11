package com.android.imsstack.util;

public final class Log {
    public static final String TAG = "ImsStack";
    public static final int DEBUG = android.util.Log.DEBUG;

    private static int sDebug = -1;
    private static int sAdminDebug = -1;

    static {
        if (!"user".equals(android.os.Build.TYPE)) {
            sDebug = 1;
        }
    }

    public static boolean isDebuggable() {
        if (sDebug == 1 || sAdminDebug == 1) {
            return true;
        } else if (sDebug == -1) {
            return isLoggable(DEBUG);
        }

        return false;
    }

    public static boolean isLoggable(int level) {
        return android.util.Log.isLoggable(TAG, level);
    }

    public static void d(String tag, String msg) {
        android.util.Log.d(tag, msg);
    }

    public static void e(String tag, String msg) {
        android.util.Log.e(tag, msg);
    }

    public static void e(String tag, String msg, Throwable t) {
        android.util.Log.e(tag, msg, t);
    }

    public static void i(String tag, String msg) {
        android.util.Log.i(tag, msg);
    }

    public static void v(String tag, String msg) {
        android.util.Log.v(tag, msg);
    }

    public static void w(String tag, String msg) {
        android.util.Log.w(tag, msg);
    }

    // Conditional logging {
    public static void dc(String tag, String msg) {
        if (isDebuggable()) {
            d(tag, msg);
        }
    }

    public static void vc(String tag, String msg) {
        if (isDebuggable()) {
            v(tag, msg);
        }
    }
    // }

    // PII : Personally Identifiable Information
    public static String pii(String s) {
        if (s == null) {
            return "(null)";
        } else if (s.isEmpty()) {
            return "(empty)";
        } else if (isDebuggable()) {
            return s;
        }

        return "****";
    }

    public static void setDebuggable() {
        sDebug = isLoggable(DEBUG) ? 1 : 0;
    }

    public static void setAdminDebug(boolean adminDebug) {
        sAdminDebug = adminDebug ? 1 : 0;
    }

    public static String firstSubString(String s, String delimiter, boolean leftToken) {
        if (s == null) {
            return "(null)";
        } else if (s.isEmpty()) {
            return "(empty)";
        } else {
            int index = (delimiter == null) ? (-1) : s.indexOf(delimiter);
            if (index < 0) {
                return s;
            }
            return leftToken ? s.substring(0, index) : s.substring(index + 1, s.length());
        }
    }

    public static String lastSubString(String s, int count) {
        if (s == null) {
            return "(null)";
        } else if (s.isEmpty()) {
            return "(empty)";
        } else {
            if (s.length() < count) {
                return s;
            }

            return subString(s, s.length() - count, s.length());
        }
    }

    public static String lastSubString(String s, String delimiter) {
        if (s == null) {
            return "(null)";
        } else if (s.isEmpty()) {
            return "(empty)";
        } else {
            int index = (delimiter == null) ? (-1) : s.lastIndexOf(delimiter);
            return (index < 0) ? s : s.substring(index + 1, s.length());
        }
    }

    public static String subString(String s, int start, int end) {
        if (s == null) {
            return "(null)";
        } else if (s.isEmpty()) {
            return "(empty)";
        } else {
            start = ((start < 0) || (start > s.length())) ? 0 : start;
            end = ((end < 0) || (end > s.length())) ? s.length() : end;

            if (start > end) {
                return "(empty)";
            }

            return s.substring(start, end);
        }
    }
}
