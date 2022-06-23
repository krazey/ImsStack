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

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "ImsMap.h"
#include "ImsList.h"
#include "SurfaceManager.h"

__IMS_TRACE_TAG_USER_DECL__("MED.VS");

extern void IMSInterface_ReleaseSurface(long nSurface);

class SurfaceManagerPrivate
{
public:
    inline SurfaceManagerPrivate() :
            piLock(IMS_NULL),
            objSurfaceMaps(IMSMap<IMS_UINTP, IMSList<IMS_UINTP>>()),
            objSurfaceMapsForRemoval(IMSMap<IMS_UINTP, IMSList<IMS_UINTP>>())
    {
        piLock = MutexService::GetMutexService()->CreateMutex();
    }
    inline ~SurfaceManagerPrivate() { MutexService::GetMutexService()->DestroyMutex(piLock); }

private:
    SurfaceManagerPrivate(IN const SurfaceManagerPrivate& objRHS);
    SurfaceManagerPrivate& operator=(IN const SurfaceManagerPrivate& objRHS);

public:
    void AddSurface(IN IMS_UINTP nSession, IN IMS_UINTP nSurface);
    void RemoveSurface(IN IMS_UINTP nSession, IN IMS_UINTP nSurface);
    void RemoveAll(IN IMS_UINTP nSession);
    void RemoveSurfaceDelayed(IN IMS_UINTP nSession, IN IMS_UINTP nSurface);
    void RemovePendingSurfaces(IN IMS_UINTP nSession);

private:
    IMutex* piLock;
    IMSMap<IMS_UINTP, IMSList<IMS_UINTP>> objSurfaceMaps;
    IMSMap<IMS_UINTP, IMSList<IMS_UINTP>> objSurfaceMapsForRemoval;
};

PUBLIC
void SurfaceManagerPrivate::AddSurface(IN IMS_UINTP nSession, IN IMS_UINTP nSurface)
{
    if (nSession == 0 || nSurface == 0)
    {
        return;
    }

    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objSurfaceMaps.GetIndexOfKey(nSession);

    if (nIndex < 0)
    {
        IMSList<IMS_UINTP> objSurfaces;
        objSurfaces.Append(nSurface);

        objSurfaceMaps.Add(nSession, objSurfaces);

        IMS_TRACE_D("AddSurface - [%" PFLS_x "]", nSurface, 0, 0);
        return;
    }

    IMSList<IMS_UINTP>& objSurfaces = objSurfaceMaps.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSurfaces.GetSize(); i++)
    {
        IMS_UINTP nTmpSurface = objSurfaces.GetAt(i);

        if (nTmpSurface == nSurface)
        {
            return;
        }
    }

    objSurfaces.Append(nSurface);

    IMS_TRACE_D("AddSurface[%d], [%" PFLS_x "]", objSurfaces.GetSize(), nSurface, 0);
}

PUBLIC
void SurfaceManagerPrivate::RemoveSurface(IN IMS_UINTP nSession, IN IMS_UINTP nSurface)
{
    if (nSession == 0 || nSurface == 0)
    {
        return;
    }

    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objSurfaceMaps.GetIndexOfKey(nSession);

    if (nIndex < 0)
    {
        return;
    }

    IMSList<IMS_UINTP>& objSurfaces = objSurfaceMaps.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSurfaces.GetSize(); i++)
    {
        IMS_UINTP nTmpSurface = objSurfaces.GetAt(i);

        if (nTmpSurface == nSurface)
        {
            IMSInterface_ReleaseSurface(nSurface);
            objSurfaces.RemoveAt(i);

            IMS_TRACE_D("SurfaceManager :: RemoveSurface[%d], [%" PFLS_x "]", objSurfaces.GetSize(),
                    nSurface, 0);

            return;
        }
    }
}

PUBLIC
void SurfaceManagerPrivate::RemoveAll(IN IMS_UINTP nSession)
{
    if (nSession == 0)
    {
        return;
    }

    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objSurfaceMaps.GetIndexOfKey(nSession);

    if (nIndex < 0)
    {
        nIndex = objSurfaceMapsForRemoval.GetIndexOfKey(nSession);

        if (nIndex >= 0)
        {
            objSurfaceMapsForRemoval.RemoveAt(nIndex);
        }

        return;
    }

    IMSList<IMS_UINTP>& objSurfaces = objSurfaceMaps.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSurfaces.GetSize(); i++)
    {
        IMS_UINTP nSurface = objSurfaces.GetAt(i);
        IMSInterface_ReleaseSurface(nSurface);
    }

    objSurfaceMaps.RemoveAt(nIndex);

    nIndex = objSurfaceMapsForRemoval.GetIndexOfKey(nSession);

    if (nIndex >= 0)
    {
        objSurfaceMapsForRemoval.RemoveAt(nIndex);
    }

    IMS_TRACE_D("SurfaceManager :: RemoveAll - [%d][%d]", objSurfaceMaps.GetSize(),
            objSurfaceMapsForRemoval.GetSize(), 0);
}

PUBLIC
void SurfaceManagerPrivate::RemoveSurfaceDelayed(IN IMS_UINTP nSession, IN IMS_UINTP nSurface)
{
    if (nSession == 0 || nSurface == 0)
    {
        return;
    }

    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objSurfaceMapsForRemoval.GetIndexOfKey(nSession);

    if (nIndex < 0)
    {
        IMSList<IMS_UINTP> objSurfaces;
        objSurfaces.Append(nSurface);

        objSurfaceMapsForRemoval.Add(nSession, objSurfaces);
        IMS_TRACE_D("RemoveSurfaceDelayed - [%" PFLS_x "]", nSurface, 0, 0);
        return;
    }

    IMSList<IMS_UINTP>& objSurfaces = objSurfaceMapsForRemoval.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSurfaces.GetSize(); i++)
    {
        IMS_UINTP nTmpSurface = objSurfaces.GetAt(i);

        if (nTmpSurface == nSurface)
        {
            return;
        }
    }

    objSurfaces.Append(nSurface);

    IMS_TRACE_D("RemoveSurfaceDelayed[%d], [%" PFLS_x "]", objSurfaces.GetSize(), nSurface, 0);
}

PUBLIC
void SurfaceManagerPrivate::RemovePendingSurfaces(IN IMS_UINTP nSession)
{
    if (nSession == 0)
    {
        return;
    }

    IMSList<IMS_UINTP> objSurfaces;
    IMS_SLONG nIndex = -1;

    {
        LockGuard objLock(piLock);
        nIndex = objSurfaceMapsForRemoval.GetIndexOfKey(nSession);

        if (nIndex < 0)
        {
            return;
        }

        objSurfaces = objSurfaceMapsForRemoval.GetValueAt(nIndex);
        objSurfaceMapsForRemoval.RemoveAt(nIndex);
    }

    for (IMS_UINT32 i = 0; i < objSurfaces.GetSize(); i++)
    {
        RemoveSurface(nSession, objSurfaces.GetAt(i));
    }

    IMS_TRACE_D("RemovePendingSurfaces[%d], removedCount[%d]", objSurfaceMapsForRemoval.GetSize(),
            objSurfaces.GetSize(), 0);
}

PRIVATE
SurfaceManager::SurfaceManager() :
        pPrivate(new SurfaceManagerPrivate())
{
}

PRIVATE
SurfaceManager::~SurfaceManager()
{
    delete pPrivate;
}

PUBLIC GLOBAL SurfaceManager* SurfaceManager::GetInstance()
{
    static SurfaceManager* pSurfaceManager = IMS_NULL;

    if (pSurfaceManager == IMS_NULL)
    {
        pSurfaceManager = new SurfaceManager();
    }

    return pSurfaceManager;
}

PUBLIC
void SurfaceManager::AddSurface(IN IMS_UINTP nSession, IN IMS_UINTP nSurface)
{
    pPrivate->AddSurface(nSession, nSurface);
}

PUBLIC
void SurfaceManager::RemoveSurface(IN IMS_UINTP nSession, IN IMS_UINTP nSurface)
{
    pPrivate->RemoveSurface(nSession, nSurface);
}

PUBLIC
void SurfaceManager::RemoveAll(IN IMS_UINTP nSession)
{
    pPrivate->RemoveAll(nSession);
}

PUBLIC
void SurfaceManager::RemoveSurfaceDelayed(IN IMS_UINTP nSession, IN IMS_UINTP nSurface)
{
    pPrivate->RemoveSurfaceDelayed(nSession, nSurface);
}

PUBLIC
void SurfaceManager::RemovePendingSurfaces(IN IMS_UINTP nSession)
{
    pPrivate->RemovePendingSurfaces(nSession);
}
