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
#ifndef OS_FILE_H_
#define OS_FILE_H_

#include "ImsFile.h"

class OsFilePrivate;

class OsFile final : public ImsFile
{
public:
    OsFile();
    virtual ~OsFile();

    OsFile(IN const OsFile&) = delete;
    OsFile& operator=(IN const OsFile&) = delete;

public:
    IMS_BOOL Open(IN const AString& strName, IN FILE_OPEN_ENTYPE eMode) override;
    void Close() override;
    IMS_UINT32 Seek(IN IMS_UINT32 nOffset, IN FILE_SEEK_ENTYPE eFrom) const override;
    IMS_UINT32 Read(OUT void* pBuffer, IN IMS_UINT32 nNumberOfBytesToRead) override;
    IMS_UINT32 Write(IN void* pBuffer, IN IMS_UINT32 nNumberOfBytesToWrite) override;
    IMS_UINT32 GetSize() const override;
    IMS_UINT32 GetPos() const override;

private:
    OsFilePrivate* m_pFileP;
};

class OsFileUtil : public IFileUtil
{
public:
    OsFileUtil() {}
    virtual ~OsFileUtil() {}

public:
    IMS_BOOL ChangeMode(IN const AString& strFileName, IN IMS_SINT32 nMode) const override;
    IMS_BOOL ChangeOwner(
            IN const AString& strFileName, IN IMS_SINT32 nUid, IN IMS_SINT32 nGid) const override;

    IMS_BOOL Exist(IN const AString& strFileName) const override;

    IMS_BOOL Rename(IN const AString& strOldName, IN const AString& strNewName) const override;

    IMS_BOOL Delete(IN const AString& strFileName) const override;

    AString GetName(IN const AString& strFilePath) const override;

    AString GetExtension(IN const AString& strFileName) const override;

    const IMS_CHAR* GetFileSeparator() const override;

    IMS_BOOL MakeDir(IN const AString& strPathName, IN IMS_SINT32 nMode) const override;

    IMS_BOOL MakeDirs(IN const AString& strPathName, IN IMS_SINT32 nMode) const override;

    void RemoveDir(IN const AString& strPathName) override;

    IMS_BOOL ExistDir(IN const AString& strPathName) const override;

    AString GetExternalStoragePath() const override;

    IMS_BOOL DeleteAllFiles(IN const AString& strPathName,
            IN const AString& strFileType = AString::ConstNull()) override;

    ImsList<AString> GetAllFiles(IN const AString& strPathName,
            IN const AString& strFileType = AString::ConstNull()) override;

    IMS_SLONG GetLastModifiedTime(IN const AString& strPathName) override;

private:
    static const IMS_CHAR FILE_SEPARATOR[];
};

#endif
