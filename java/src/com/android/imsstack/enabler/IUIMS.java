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
package com.android.imsstack.enabler;

public class IUIMS
{
    public static final int APP_BASE = 0;
    public static final int APP_MTC = 1;
    public static final int APP_MTS = 2;
    public static final int APP_UCE = 3;
    public static final int APP_SIP_DELEGATE = 4;
    public static final int APP_MAX = 5;
    public static final int APP_UNKNOWN = APP_MAX + 1;

    // System interface
    public static final int SYSTEM_INTERFACE = 51;

    // AOS
    public static final int AOS_SERVICE = 61;

    // MTC
    public static final int MTC_CALL = 91;

    public static final int MTS_BASE = 400;
    public static final int MTS_EMERGENCY_SERVICE = (MTS_BASE + 1);

    //// Bitmask to composite each services
    public static final int M_APP_UC = 0x00000001;
    public static final int M_APP_SMS = 0x00000002;
    public static final int M_APP_VT = 0x00000004;
    public static final int M_SERVICE_ALL = 0xFFFFFFFF;
}
