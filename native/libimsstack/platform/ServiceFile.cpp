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
#include "ImsFile.h"
#include "PlatformFactory.h"
#include "ServiceFile.h"
#include "ServiceMemory.h"

class FileServicePrivate
{
public:
    inline FileServicePrivate()
        : m_piFileUtil(IMS_NULL)
    {}
    inline ~FileServicePrivate()
    {
        PlatformFactory::DestroyFileUtil(m_piFileUtil);
    }

    FileServicePrivate(IN const FileServicePrivate&) = delete;
    FileServicePrivate& operator=(IN const FileServicePrivate&) = delete;

public:
    inline IFileUtil* GetFileUtil()
    {
        if (m_piFileUtil == IMS_NULL)
        {
            m_piFileUtil = PlatformFactory::CreateFileUtil();
        }

        return m_piFileUtil;
    }

private:
    IFileUtil* m_piFileUtil;
};



PRIVATE
FileService::FileService()
    : m_pPrivate(new FileServicePrivate())
{
}

PRIVATE
FileService::~FileService()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
IFile* FileService::CreateFile()
{
    ImsFile* pFile = PlatformFactory::CreateFile();

    IMS_ASSERT(pFile != IMS_NULL);

    return pFile;
}

PUBLIC
void FileService::DestroyFile(IN IFile*& piFile)
{
    ImsFile* pFile = DYNAMIC_CAST(ImsFile*, piFile);

    if (pFile != IMS_NULL)
    {
        delete pFile;
        piFile = IMS_NULL;
    }
}

PUBLIC
IFileUtil* FileService::GetFileUtil()
{
    return m_pPrivate->GetFileUtil();
}

PUBLIC GLOBAL
FileService* FileService::GetFileService()
{
    static FileService* s_pFileService = IMS_NULL;

    if (s_pFileService == IMS_NULL)
    {
        s_pFileService = new FileService();
    }

    return s_pFileService;
}
