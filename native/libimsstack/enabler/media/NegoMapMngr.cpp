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

// == INCLUDES =========================================================
#include "ServiceTrace.h"

#include "NegoMapMngr.h"

// == DEFINES =========================================================

__IMS_TRACE_TAG_USER_DECL__("MED.SP");

// == STATIC VARIABLE ===================================================

// == Constructor, Destructor, Operator Overloading ========================================

// == PUBLIC METHOD ==============================================================
PROTECTED
IMS_BOOL NegoMapMngr::SetNegoMap(IN IMS_UINT32 nMediaNegoID, IN IMS_BOOL bRunning)
{
    IMS_BOOL bTempRunning = IMS_FALSE;

    if (nMediaNegoID == 0)
    {
        return IMS_FALSE;
    }

    // Only nMediaNegoID value will be TRUE
    SetAllNegoMap(IMS_FALSE);

    if (GetNegoMap(nMediaNegoID, bTempRunning) == IMS_TRUE)
    {
        m_objNegoMap.SetValue(nMediaNegoID, bRunning);
        return IMS_TRUE;
    }

    m_objNegoMap.Add(nMediaNegoID, bRunning);
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL NegoMapMngr::GetNegoMap(IN IMS_UINT32 nMediaNegoID, OUT IMS_BOOL& bRunning)
{
    IMS_SLONG nIndex = m_objNegoMap.GetIndexOfKey(nMediaNegoID);
    if (nIndex < 0)
    {
        bRunning = IMS_FALSE;
        return IMS_FALSE;
    }

    bRunning = m_objNegoMap.GetValueAt(nIndex);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL NegoMapMngr::SetAllNegoMap(IN IMS_BOOL bRunning)
{
    for (IMS_UINT32 n = 0; n < m_objNegoMap.GetSize(); n++)
    {
        m_objNegoMap.SetValueAt(n, bRunning);
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL NegoMapMngr::ClearNegoMap()
{
    IMS_TRACE_D("NegoMapMngr::ClearNegoMap", 0, 0, 0);
    PrintCurrentRunningNego();

    while (m_objNegoMap.GetSize() != 0)
    {
        m_objNegoMap.RemoveAt(0);
    }

    return IMS_TRUE;
}

PROTECTED
void NegoMapMngr::PrintCurrentRunningNego()
{
    for (IMS_UINT32 n = 0; n < m_objNegoMap.GetSize(); n++)
    {
        if (m_objNegoMap.GetValueAt(n) == IMS_TRUE)
        {
            IMS_TRACE_D("NegoMapMngr::PrintCurrentRunningNego - nMediaNegoID[%x]",
                    m_objNegoMap.GetKeyAt(n), 0, 0);
            return;
        }
    }

    IMS_TRACE_D("NegoMapMngr::PrintCurrentRunningNego - Not Running", 0, 0, 0);
    return;
}
