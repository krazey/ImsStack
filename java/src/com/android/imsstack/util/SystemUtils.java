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

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

/**
 * This class provides the wrapper APIs for Android Platform.
 */
public final class SystemUtils {
    public static final String UTC_TIME_FORMAT = "yyyy-MM-dd'T'HH:mm:ss";

    /**
     * Performs the message digest and converts it to the hexa-decimal string.
     *
     * @param algorithm The digest algorithm.
     * @param input The input string.
     * @return A hexa-decimal string as a result of message digest.
     */
    public static String calculateMessageDigest(String algorithm, String input) {
        MessageDigest md = null;
        byte[] digest = null;

        try {
            md = MessageDigest.getInstance(algorithm);
            digest = md.digest(input.getBytes());
        } catch (NoSuchAlgorithmException e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
        } catch (Exception e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
        }

        return (digest != null) ? toHexString(digest) : null;
    }

    /**
     * Generates the UTC time format string from the given milli-seconds.
     *
     * @param millis The time as milli-seconds to be converted.
     * @return A UTC time format string.
     */
    public static String getUtcTimeFormat(long millis) {
        SimpleDateFormat sdf = new SimpleDateFormat(UTC_TIME_FORMAT, Locale.US);
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        return sdf.format(new Date(millis)) + "Z";
    }

    /**
     * Converts the hex-decimal string to an integer value.
     *
     * @param value The hexa-decimal string (e.g. "0x0001000F")
     * @return The coverted integer value if the value is valid, -1 otherwise.
     */
    public static int hexStringToInt(String value) {
        if (value == null || value.isEmpty()) {
            return -1;
        }

        Integer intValue;

        try {
            intValue = Integer.decode(value);
        } catch (NumberFormatException e) {
            intValue = null;
        }

        if (intValue == null) {
            return -1;
        }

        long longValue = intValue.longValue();
        return (int) (longValue & 0xFFFFFFFF);
    }

    public static String toHexString(byte[] input) {
        if (input == null) {
            return null;
        }

        String output = "";
        String hex;

        for (int i = 0; i < input.length; ++i) {
            hex = Integer.toHexString(input[i] & 0xff);

            if (hex.length() == 1) {
                hex = "0".concat(hex);
            }

            output += hex.toLowerCase();
        }

        return output;
    }

    private SystemUtils() {}
}
