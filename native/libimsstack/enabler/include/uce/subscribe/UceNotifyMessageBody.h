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

#ifndef UCE_NOTIFY_MESSAGE_BODY_H_
#define UCE_NOTIFY_MESSAGE_BODY_H_

#include "UceNotifyBodyPartData.h"

class UceNotifyMessageBody
{
public:
    explicit UceNotifyMessageBody() :
            m_strContentType(AString::ConstNull()){};
    virtual ~UceNotifyMessageBody()
    {
        for (IMS_UINT32 i = 0; i < m_objNotifyBodyPartDatas.GetSize(); ++i)
        {
            UceNotifyBodyPartData* pData = m_objNotifyBodyPartDatas.GetAt(i);
            if (pData != IMS_NULL)
            {
                delete pData;
            }
        }
        m_objNotifyBodyPartDatas.Clear();
    }

    void SetContentType(IN const AString& strContentType) { m_strContentType = strContentType; }
    void SetNotifyBodyPartData(IN UceNotifyBodyPartData* pData)
    {
        m_objNotifyBodyPartDatas.Append(pData);
    }
    AString& GetContentType() { return m_strContentType; }
    ImsList<UceNotifyBodyPartData*> GetNotifyBodyPartDatas() const
    {
        return m_objNotifyBodyPartDatas;
    }

private:
    AString m_strContentType;
    ImsList<UceNotifyBodyPartData*> m_objNotifyBodyPartDatas;
};
#endif /* UCE_NOTIFY_MESSAGE_BODY_H_ */
