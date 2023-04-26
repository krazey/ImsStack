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

import android.annotation.NonNull;
import android.telephony.PhoneNumberUtils;

/**
 * This class provides the helper APIs for SIM interworking.
 */
public final class SimUtils {
    private static final char[] HEX_CHARS = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    /**
     * BCD values 'C', 'D' and 'E' are never sent across the radio interface.
     *     - PAUSE(‘,’), WILD(‘N’), WAIT(‘;’)
     */
    private static final char PAUSE = ',';
    private static final char WILD = 'N';
    private static final char WAIT = ';';

    /**
     * Converts a byte array into a hexadecimal string.
     *
     * @param bytes The byte array.
     * @return A hexadecimal string format of the specified byte array.
     */
    public static String bytesToHexString(@NonNull byte[] bytes) {
        StringBuilder hexString = new StringBuilder(2 * bytes.length);

        for (int i = 0; i < bytes.length; i++) {
            int b;
            b = 0x0f & (bytes[i] >> 4);
            hexString.append(HEX_CHARS[b]);
            b = 0x0f & bytes[i];
            hexString.append(HEX_CHARS[b]);
        }

        return hexString.toString();
    }

    /**
     * Converts a hexadecimal string into a byte array.
     *
     * @param s The hexadecimal string.
     * @return A byte array of the specified string.
     */
    public static byte[] hexStringToBytes(String s) {
        if (s == null) {
            return null;
        }

        int size = s.length();
        byte[] bytes = new byte[size / 2];

        for (int i = 0; i < size; i += 2) {
            bytes[i / 2] = (byte) ((hexCharToInt(s.charAt(i)) << 4)
                    | hexCharToInt(s.charAt(i + 1)));
        }

        return bytes;
    }

    /**
     * Converts a hexadecimal character value to an integer value.
     *
     * @param c The hexadecimal character.
     * @return An integer value of the specified character.
     */
    public static int hexCharToInt(char c) {
        if (c >= '0' && c <= '9') {
            return (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            return (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            return (c - 'a' + 10);
        }

        throw new RuntimeException("invalid hex char '" + c + "'");
    }

    /**
     * Converts a normal string to a BCD based string.
     * It just changes the order of each two characters.
     * And, if the string length is odd, it adds 'F' character in the end of the string.
     *
     * @param s The string value.
     * @return A BCD string.
     */
    public static String stringToBcdString(String s) {
        if (s == null) {
            return null;
        } else if (s.length() == 0) {
            return "";
        }

        if (s.length() % 2 != 0) {
            s += "F";
        }

        StringBuilder sb = new StringBuilder(s);

        for (int i = 0; i < sb.length(); i += 2) {
            char c = sb.charAt(i);
            sb.setCharAt(i, sb.charAt(i + 1));
            sb.setCharAt(i + 1, c);
        }

        return sb.toString();
    }

    /**
     * Checks if the specified string contains the wild values or not.
     *
     * @param s The string to be evaluated.
     * @return true if it contains wild values, false otherwise.
     */
    public static boolean containsWildValue(String s) {
        if (s == null || s.isEmpty()) {
            return false;
        }

        for (int i = 0; i < s.length(); ++i) {
            char c = s.charAt(i);

            if (c == PAUSE || c == WILD || c == WAIT) {
                return true;
            }
        }

        return false;
    }

    /**
     * Checks if the specified string only consists of ISO-LATIN characters 0-9, *, #.
     *
     * @param s The string to be evaluated.
     * @return true if it only contains 12 keys, false otherwise.
     */
    public static boolean contains12KeysOnly(String s) {
        if (s == null || s.isEmpty()) {
            return false;
        }

        for (int i = 0; i < s.length(); ++i) {
            if (!PhoneNumberUtils.is12Key(s.charAt(i))) {
                return false;
            }
        }

        return true;
    }

    private SimUtils() {}
}
