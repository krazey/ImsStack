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
#ifndef INTERFACE_AOS_BLOCK_LISTENER_H_
#define INTERFACE_AOS_BLOCK_LISTENER_H_

#include "ImsTypeDef.h"

class IAosBlockListener
{
public:
    virtual ~IAosBlockListener(){};

    /**
     * @brief Called when a block reason is changed.
     *
     * This method is invoked for block reason changes.
     *
     * @param nType The type of the block.
     * @param nParam The state of the block.
     */
    virtual void Block_Changed(IN IMS_UINT32 nType = 0, IN IMS_UINT32 nParam = 0) = 0;
};
#endif  // INTERFACE_AOS_BLOCK_LISTENER_H_
