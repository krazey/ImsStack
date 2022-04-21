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
#ifndef SERVICE_VONR_H_
#define SERVICE_VONR_H_

#include "IMSMSG.h"

class IVoNR;
class VoNRServicePrivate;

class VoNRService
{
private:
    VoNRService();
    ~VoNRService();

private:
    VoNRService(IN CONST VoNRService &objRHS);
    VoNRService& operator=(IN CONST VoNRService &objRHS);

public:
    IVoNR* GetVoNR(IN IMS_SINT32 nSlotId);

    void DispatchServiceMessage(IN IMSMSG &objMSG);

    static VoNRService* GetVoNRService();

private:
    VoNRServicePrivate *pPrivate;
};

#endif // SERVICE_VONR_H_
