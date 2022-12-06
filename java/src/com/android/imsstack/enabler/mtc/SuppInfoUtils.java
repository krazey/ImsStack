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

package com.android.imsstack.enabler.mtc;

import java.util.LinkedHashMap;

/**
 * This class provides the utility methods to control items of SuppInfo.
 */
public final class SuppInfoUtils {
    public static final int TYPE_BOOLEAN = 1;
    public static final int TYPE_INT = 2;
    public static final int TYPE_STRING = 3;

    private static final LinkedHashMap<Integer, String> sTypeToKey =
            new LinkedHashMap<Integer, String>();
    private static final LinkedHashMap<Integer, Integer> sTypeToValueType =
            new LinkedHashMap<Integer, Integer>();

    public static void addKey(int type, String key) {
        sTypeToKey.put(type, key);
    }

    public static void removeKey(int type) {
        sTypeToKey.remove(type);
    }

    public static void addValueType(int type, int valueType) {
        sTypeToValueType.put(type, valueType);
    }

    public static void removeValueType(int type) {
        sTypeToValueType.remove(type);
    }

    public static String getKey(int type) {
        return sTypeToKey.get(type);
    }

    public static boolean isValueBoolean(int type) {
        Integer valueType = sTypeToValueType.get(type);
        return (valueType != null) ? (valueType.intValue() == TYPE_BOOLEAN) : false;
    }

    public static boolean isValueInt(int type) {
        Integer valueType = sTypeToValueType.get(type);
        return (valueType != null) ? (valueType.intValue() == TYPE_INT) : false;
    }

    public static boolean isValueString(int type) {
        Integer valueType = sTypeToValueType.get(type);
        return (valueType != null) ? (valueType.intValue() == TYPE_STRING) : false;
    }
}
