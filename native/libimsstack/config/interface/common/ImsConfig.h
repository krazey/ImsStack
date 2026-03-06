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
#ifndef IMS_CONFIG_H_
#define IMS_CONFIG_H_

class ImsConfig
{
public:
    /// Flags to identify the configuration repository
    /// It will be used in lParam or LOWORD(wParam) of IMS_EVENT_CONFIG_UPDATE
    enum
    {
        FLAG_IMS_NONE = 0x0000,

        FLAG_IMS_SUBSCRIBER = 0x0001,
        FLAG_IMS_ENGINE = 0x0002,
        FLAG_IMS_SIP = 0x0004,
        FLAG_IMS_AOS_CONNECTION = 0x0010,
        FLAG_IMS_AOS_REG = 0x0020,
        FLAG_IMS_COM_SIP = 0x0100,
        FLAG_IMS_COM_MEDIA = 0x0200,

        // Service specific
        FLAG_IMS_COM_SERVICE_SMS = 0x1000,

        F_IMS_ALL = 0xFFFF,
    };
};

#endif
