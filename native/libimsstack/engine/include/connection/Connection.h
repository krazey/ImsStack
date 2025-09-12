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
#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "EngineActivity.h"

class IConnection;

class Connection : public EngineActivity
{
public:
    Connection();
    ~Connection() override = default;

    Connection(IN const Connection&) = delete;
    Connection& operator=(IN const Connection&) = delete;

public:
    // IConnection interface
    virtual void Close();

    inline IConnection* GetOwner() const { return m_piOwner; }
    inline void SetOwner(IN IConnection* piOwner) { m_piOwner = piOwner; }

private:
    IConnection* m_piOwner;
};

#endif
