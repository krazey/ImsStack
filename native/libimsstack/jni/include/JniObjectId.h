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
#ifndef JNI_OBJECT_ID_H_
#define JNI_OBJECT_ID_H_

#include "ImsTypeDef.h"

/**
 * An interface for providing the object identifiers for JNI interworking.
 */
class JniObjectId
{
public:
    JniObjectId() = delete;

public:
    /** Indicates the identifier of System interface. */
    static const IMS_SINT32 SYSTEM = 1;

    /** Indicates the identifier of AoS (Always On Service) enabler. */
    static const IMS_SINT32 AOS = 11;

    /** Indicates the identifier of MTC (MmTel Call) enabler. */
    static const IMS_SINT32 MTC = 12;
    /** Indicates the identifier of MTS (MmTel SMS) enabler. */
    static const IMS_SINT32 MTS = 13;
    /** Indicates the identifier of UCE (User Capability Exchange) enabler. */
    static const IMS_SINT32 UCE = 14;
    /** Indicates the identifier of SIP delegate (for RCS single registration) enabler. */
    static const IMS_SINT32 SIP_DELEGATE = 15;

    /** Indicates the identifier of the MTC enabler's call. */
    static const IMS_SINT32 MTC_CALL = 21;
};

#endif
