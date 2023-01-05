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
#ifndef CONST_3GPP_H_
#define CONST_3GPP_H_

#include "ImsTypeDef.h"

/**
 * @brief This class defines the constant values for 3GPP specifications.
 */
class Const3GPP
{
public:
    //// FEATURE TAG
    // SMS over IMS
    static const IMS_CHAR FEATURE_TAG_SMSIP[];

    //// IARI

    //// ICSI
    // Multimedia Telephony
    static const IMS_CHAR ICSI_MMTEL[];
};

#endif
