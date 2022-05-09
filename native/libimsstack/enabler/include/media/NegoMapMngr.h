/**
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

#include "IMSMap.h"

#ifndef _NEGO_MAP_MNGR_H_
#define _NEGO_MAP_MNGR_H_

// == INCLUDES =========================================================
class NegoMapMngr
{
protected:
    IMS_BOOL SetNegoMap(IN IMS_UINT32 nMediaNegoID, IN IMS_BOOL bRunning);
    IMS_BOOL GetNegoMap(IN IMS_UINT32 nMediaNegoID, OUT IMS_BOOL& bRunning);
    IMS_BOOL SetAllNegoMap(IN IMS_BOOL bRunning);
    IMS_BOOL ClearNegoMap();
    void PrintCurrentRunningNego();

protected:
    IMSMap<IMS_UINT32, IMS_BOOL> m_objNegoMap;
};
#endif /* _NEGO_MAP_MNGR_H_ */
