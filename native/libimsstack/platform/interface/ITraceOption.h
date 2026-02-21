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
#ifndef INTERFACE_TRACE_OPTION_H_
#define INTERFACE_TRACE_OPTION_H_

class ITraceOption
{
public:
    enum
    {
        /// Option - filtering
        OPT_CAT_NONE = 0x00000000,
        OPT_CAT_D = 0x00000001,
        OPT_CAT_E = 0x00000002,
        OPT_CAT_I = 0x00000004,
        OPT_CAT_TEXT = 0x00000008,
        OPT_CAT_ALL = (OPT_CAT_D | OPT_CAT_E | OPT_CAT_I | OPT_CAT_TEXT),

        OPT_HIDE_PRIVACY = 0x00000100,

        /// Option - logging medium
        OPT_MEDIUM_SERIAL = 0x00010000,

        /// Default trace options
        OPT_DEFAULT = (OPT_CAT_ALL | OPT_MEDIUM_SERIAL),
        /// Release trace options
        RELEASE_OPT_DEFAULT = (OPT_CAT_E | OPT_CAT_I | OPT_CAT_TEXT | OPT_MEDIUM_SERIAL),

        OPT_MAX = 0x7FFFFFFF
    };
};

#endif
