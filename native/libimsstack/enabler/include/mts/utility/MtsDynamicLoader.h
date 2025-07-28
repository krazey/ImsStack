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

#include "utility/IMtsDynamicLoader.h"
#include "utility/MtsAosUtils.h"
#include "utility/MtsGeolocationUtils.h"
#include "utility/MtsSipFormUtils.h"
#include "utility/MtsSmUtils.h"

class IMtsContext;

class MtsDynamicLoader final : public IMtsDynamicLoader
{
public:
    explicit MtsDynamicLoader(IN IMtsContext& objContext);
    virtual ~MtsDynamicLoader();
    MtsDynamicLoader(IN const MtsDynamicLoader&) = delete;
    MtsDynamicLoader& operator=(IN const MtsDynamicLoader&) = delete;

    // IMtsDynamicLoader
    inline MtsSipFormUtils* GetMtsSipFormUtils() const override { return m_pMtsSipFormUtils; };
    inline MtsSmUtils* GetMtsSmUtils() const override { return m_pMtsSmUtils; };
    inline MtsGeolocationUtils* GetMtsGeolocationUtils() const override
    {
        return m_pMtsGeolocationUtils;
    };
    inline MtsAosUtils* GetMtsAosUtils() const override { return m_pMtsAosUtils; };

private:
    void Initialize();
    void DestroyAll();

private:
    IMtsContext& m_objContext;
    MtsSipFormUtils* m_pMtsSipFormUtils;
    MtsSmUtils* m_pMtsSmUtils;
    MtsGeolocationUtils* m_pMtsGeolocationUtils;
    MtsAosUtils* m_pMtsAosUtils;
};

#endif
