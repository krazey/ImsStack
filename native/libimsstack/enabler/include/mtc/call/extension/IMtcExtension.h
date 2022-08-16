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

#ifndef INTERFACE_MTC_EXTENSION_H_
#define INTERFACE_MTC_EXTENSION_H_

#include "IMSTypeDef.h"
#include "call/message/IMtcMessageHandler.h"

class AString;

/**
 * This class represents an extension for mmtel call. It stores whether the extension is available
 * in the call. And adds corresponding option tag to the message to be sent.
 */
class IMtcExtension : public IMtcMessageFormatter, public IMtcMessageHandler
{
public:
    virtual ~IMtcExtension() {}

    /**
     * Make a clone of this instance.
     *
     * @return Copy of this instance.
     */
    virtual IMtcExtension* Clone() const;

    /**
     * Returns whether the extension is available on the remote.
     *
     * @return True if available on the remote.
     */
    virtual IMS_BOOL IsAvailableOnRemote() const;

    /**
     * Returns whether the extension is required on the remote.
     *
     * @return True if required on the remote.
     */
    virtual IMS_BOOL IsRequiredOnRemote() const;

    /**
     * Returns the extension's option tag
     *
     * @return Option tag string.
     */
    virtual const AString& GetOptionTag() const;
};

#endif
