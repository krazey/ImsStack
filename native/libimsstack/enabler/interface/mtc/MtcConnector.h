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

#ifndef MTC_CONNECTOR_H_
#define MTC_CONNECTOR_H_

#include "IMSTypeDef.h"

class IMtcCallStateListener;

class MtcConnector
{
public:
    static void AddCallStateListener(IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener);
    static void RemoveCallStateListener(IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener);
};

#endif
