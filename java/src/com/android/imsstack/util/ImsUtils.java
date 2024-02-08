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

import androidx.annotation.NonNull;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.time.Instant;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Locale;

/**
 * This class provides the utility and helper APIs for ImsStack application.
 */
public final class ImsUtils {
    private static final char[] HEX_DIGITS = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    private static final char[] HEX_LOWERCASE_DIGITS = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    /**
     * Closes the {@link AutoCloseable} object silently.
     *
     * @param closeable The {@link AutoCloseable} object to be closed.
     */
    public static void closeQuietly(AutoCloseable closeable) {
        if (closeable != null) {
            try {
                closeable.close();
            } catch (RuntimeException rethrown) {
                throw rethrown;
            } catch (Exception e) {
                Log.e(Log.TAG, "close: " + e);
            }
        }
    }

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

        return (digest != null) ? bytesToHexString(digest, false) : null;
    }

    /**
     * Generates the UTC time format string from the given milli-seconds.
     *
     * @param millis The time as milli-seconds to be converted.
     * @return A UTC time format string.
     */
    public static String getUtcTimeFormat(long millis) {
        DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        ZonedDateTime dateTime = Instant.ofEpochMilli(millis).atZone(ZoneId.of("UTC"));
        return dateTime.format(dtf);
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

    /**
     * Converts a byte array into a hexadecimal string.
     * It will be converted to capital letters by default.
     *
     * @param bytes The byte array.
     * @return A hexadecimal string format of the specified byte array.
     */
    public static String bytesToHexString(@NonNull byte[] bytes) {
        return bytesToHexString(bytes, true);
    }

    /**
     * Converts a byte array into a hexadecimal string.
     *
     * @param bytes The byte array.
     * @param uppercase The flag specifying whether the converted hex string should be uppercase
     *                  or not.
     * @return A hexadecimal string format of the specified byte array.
     */
    public static String bytesToHexString(@NonNull byte[] bytes, boolean uppercase) {
        if (bytes == null) {
            return null;
        }
        final char[] hexDigits = uppercase ? HEX_DIGITS : HEX_LOWERCASE_DIGITS;
        StringBuilder hexString = new StringBuilder(2 * bytes.length);

        for (int i = 0; i < bytes.length; i++) {
            byte b = bytes[i];
            hexString.append(hexDigits[(b >> 4) & 0x0f]);
            hexString.append(hexDigits[b & 0x0f]);
        }

        return hexString.toString();
    }

    private ImsUtils() {}
}
