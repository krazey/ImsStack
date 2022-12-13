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

#ifndef MTS_DYNAMIC_LOADER_
#define MTS_DYNAMIC_LOADER_

#include "utility/MtsSipFormUtils.h"
#include "utility/MtsSmUtils.h"

class MtsDynamicLoader final
{
public:
    explicit MtsDynamicLoader(IN IMS_SINT32 nSlotId);
    ~MtsDynamicLoader();

    void Initialize();

    inline MtsSipFormUtils* GetMtsSipFormUtils() { return m_pMtsSipFormUtils; }
    inline MtsSmUtils* GetMtsSmUtils() { return m_pMtsSmUtils; }

private:
    void DestroyAll();

private:
    IMS_SINT32 m_nSlotId;
    MtsSipFormUtils* m_pMtsSipFormUtils;
    MtsSmUtils* m_pMtsSmUtils;
};

#endif
