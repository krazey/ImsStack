/*
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

#ifndef INTERFACE_MTS_CONTEXT_H_
#define INTERFACE_MTS_CONTEXT_H_

#include "ImsTypeDef.h"

class IMtsDynamicLoader;
class IMtsMessageController;
class IMtsService;

class IMtsContext
{
public:
    virtual ~IMtsContext() = default;

    /**
     * @brief Gets current slot ID.
     *
     * @return Slot ID.
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

    /**
     * @brief Gets the IMtsService instance associated with this context.
     *
     * @return Reference to the IMtsService instance.
     */
    virtual IMtsService& GetService() = 0;

    /**
     * @brief Gets the IMtsMessageController instance associated with this context.
     *
     * @return Reference to the IMtsMessageController instance.
     */
    virtual IMtsMessageController& GetMessageController() = 0;

    /**
     * @brief Gets the IMtsDynamicLoader instance associated with this context.
     *
     * @return Reference to the IMtsDynamicLoader instance.
     */
    virtual const IMtsDynamicLoader& GetDynamicLoader() const = 0;
};

#endif
