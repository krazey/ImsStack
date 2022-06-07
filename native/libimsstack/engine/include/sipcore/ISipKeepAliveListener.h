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
#ifndef INTERFACE_SIP_KEEP_ALIVE_LISTENER_H_
#define INTERFACE_SIP_KEEP_ALIVE_LISTENER_H_

/**
 * @brief This class defines a listener interface for the application level keep-alive mechanism.
 */
class ISipKeepAliveListener
{
public:
    virtual void KeepAlive_PongReceived() = 0;
};

#endif
