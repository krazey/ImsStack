/**
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

#include "MediaBaseProfile.h"
#include "MediaProfileFactory.h"

PUBLIC void MediaBaseProfile::DeletePayloads()
{
    while (lstPayload.GetSize() > 0)
    {
        BasePayload* pPayload = lstPayload.GetAt(0);

        delete pPayload;
        lstPayload.RemoveAt(0);
    }
}

PUBLIC void MediaBaseProfile::CopyPayloads(IN ImsList<BasePayload*> payloadList)
{
    for (IMS_UINT32 i = 0; i < payloadList.GetSize(); i++)
    {
        BasePayload* pNewPayload =
                MediaProfileFactory::GetInstance()->CreatePayload(payloadList.GetAt(i));
        lstPayload.Append(pNewPayload);
    }
}
