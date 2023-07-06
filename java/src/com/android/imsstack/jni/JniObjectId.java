/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.jni;

/**
 * An interface for providing the object identifiers for JNI interworking.
 */
public interface JniObjectId {
    /** Indicates the identifier of System interface. */
    int SYSTEM = 1;

    /** Indicates the identifier of AoS (Always On Service) enabler. */
    int AOS = 11;

    /** Indicates the identifier of MTC (MmTel Call) enabler. */
    int MTC = 12;
    /** Indicates the identifier of MTS (MmTel SMS) enabler. */
    int MTS = 13;
    /** Indicates the identifier of UCE (User Capability Exchange) enabler. */
    int UCE = 14;
    /** Indicates the identifier of SIP delegate (for RCS single registration) enabler. */
    int SIP_DELEGATE = 15;

    /** Indicates the identifier of the MTC enabler's call. */
    int MTC_CALL = 21;
}
