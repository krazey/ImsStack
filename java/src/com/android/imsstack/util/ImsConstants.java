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
 * This class provides constant values for IMS.
 */
public final class ImsConstants {
    public static final boolean DBG = !android.os.Build.TYPE.equals("user");
    public static final String PACKAGE_NAME = "com.android.imsstack";
    /**
     * N : /data/data/com.android.imsstack
     * N-MR1 (Device Encrypted storage) : /data/user_de/0/com.android.imsstack
     */
    public static final String IMS_STORAGE_ROOT_DIR = "/data/user_de/0/com.android.imsstack";

    public static final boolean USE_GOOGLE_NATIVE_APPS = true;

    public static final boolean USE_CARRIER_CONFIG = true;
}
