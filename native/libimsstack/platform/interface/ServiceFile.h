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
#ifndef SERVICE_FILE_H_
#define SERVICE_FILE_H_

#include "IFile.h"

class FileServicePrivate;

class FileService
{
private:
    FileService();
    ~FileService();

public:
    FileService(IN const FileService&) = delete;
    FileService& operator=(IN const FileService&) = delete;

public:
    IFile* CreateFile();
    void DestroyFile(IN IFile*& piFile);
    IFileUtil* GetFileUtil();

    static FileService* GetFileService();

private:
    FileServicePrivate* m_pPrivate;
};

#define IMS_FILE_Create() \
        FileService::GetFileService()->CreateFile()

#define IMS_FILE_Destroy(piFile) \
        FileService::GetFileService()->DestroyFile(piFile)

#define IMS_FILE_GetName(path) \
        FileService::GetFileService()->GetFileUtil()->GetName(path)

#define IMS_FILE_GetExtension(path) \
        FileService::GetFileService()->GetFileUtil()->GetExtension(path)

#define IMS_FILE_Exist(path) \
        FileService::GetFileService()->GetFileUtil()->Exist(path)

#define IMS_FILE_Rename(oldPath, newPath) \
        FileService::GetFileService()->GetFileUtil()->Rename(oldPath, newPath)

#define IMS_FILE_Delete(path) \
        FileService::GetFileService()->GetFileUtil()->Delete(path)

#define IMS_FILE_GetSeparator() \
        FileService::GetFileService()->GetFileUtil()->GetFileSeparator()

#endif
