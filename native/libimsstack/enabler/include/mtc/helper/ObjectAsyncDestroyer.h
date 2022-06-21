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

#ifndef OBJECT_ASYNC_DESTROYER_H_
#define OBJECT_ASYNC_DESTROYER_H_

#include "IMSTypeDef.h"
#include "ImsActivity.h"
#include "ImsMessage.h"

template <typename MtcObject>
class ObjectAsyncDestroyer final : public ImsActivity
{
public:
    inline explicit ObjectAsyncDestroyer() {}
    inline virtual ~ObjectAsyncDestroyer() {}
    ObjectAsyncDestroyer(IN const ObjectAsyncDestroyer&) = delete;
    ObjectAsyncDestroyer& operator=(IN const ObjectAsyncDestroyer&) = delete;

    // ImsActivity implementation
    inline IImsActivityController* GetController() override { return IMS_NULL; }
    inline IMS_BOOL DispatchMessage(IN IMSMSG& objMsg) override;

    inline void Destroy(IN MtcObject* pObject);

private:
    static const IMS_UINT32 MSG_DESTROY_OBJECT = 0;
};

PUBLIC VIRTUAL template <typename MtcObject>
inline IMS_BOOL ObjectAsyncDestroyer<MtcObject>::DispatchMessage(IN IMSMSG& objMsg)
{
    switch (objMsg.GetName())
    {
        case MSG_DESTROY_OBJECT:
            delete reinterpret_cast<MtcObject*>(objMsg.nLparam);
            return IMS_TRUE;
        default:
            return IMS_FALSE;
    }
}

PUBLIC
template <typename MtcObject>
inline void ObjectAsyncDestroyer<MtcObject>::Destroy(IN MtcObject* pObject)
{
    PostMessage(MSG_DESTROY_OBJECT, 0, reinterpret_cast<IMS_UINTP>(pObject));
}

#endif
