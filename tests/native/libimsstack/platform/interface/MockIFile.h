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
#ifndef MOCK_I_FILE_H_
#define MOCK_I_FILE_H_

#include <gmock/gmock.h>

#include "IFile.h"

class MockIFile : public IFile
{
public:
    MockIFile() = default;
    ~MockIFile() override = default;

    MOCK_METHOD(IMS_BOOL, Open, (IN const AString& strName, IN FILE_OPEN_ENTYPE eMode), (override));
    MOCK_METHOD(void, Close, (), (override));
    MOCK_METHOD(IMS_UINT32, Seek, (IN IMS_UINT32 nOffset, IN FILE_SEEK_ENTYPE eFrom),
            (const, override));
    MOCK_METHOD(
            IMS_UINT32, Read, (OUT void* pBuffer, IN IMS_UINT32 nNumberOfBytesToRead), (override));
    MOCK_METHOD(
            IMS_UINT32, Write, (IN void* pBuffer, IN IMS_UINT32 nNumberOfBytesToWrite), (override));
    MOCK_METHOD(IMS_UINT32, GetSize, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetPos, (), (const, override));
};

class MockIFileUtil : public IFileUtil
{
public:
    MockIFileUtil() = default;
    ~MockIFileUtil() override = default;

    MOCK_METHOD(IMS_BOOL, ChangeMode, (IN const AString& strFileName, IN IMS_SINT32 nMode),
            (const, override));
    MOCK_METHOD(IMS_BOOL, ChangeOwner,
            (IN const AString& strFileName, IN IMS_SINT32 nUid, IN IMS_SINT32 nGid),
            (const, override));
    MOCK_METHOD(IMS_BOOL, Exist, (IN const AString& strFileName), (const, override));
    MOCK_METHOD(IMS_BOOL, Rename, (IN const AString& strOldName, IN const AString& strNewName),
            (const, override));
    MOCK_METHOD(IMS_BOOL, Delete, (IN const AString& strFileName), (const, override));
    MOCK_METHOD(AString, GetName, (IN const AString& strFilePath), (const, override));
    MOCK_METHOD(AString, GetExtension, (IN const AString& strFileName), (const, override));
    MOCK_METHOD(const IMS_CHAR*, GetFileSeparator, (), (const, override));
    MOCK_METHOD(IMS_BOOL, MakeDir, (IN const AString& strPathName, IN IMS_SINT32 nMode),
            (const, override));
    MOCK_METHOD(IMS_BOOL, MakeDirs, (IN const AString& strPathName, IN IMS_SINT32 nMode),
            (const, override));
    MOCK_METHOD(void, RemoveDir, (IN const AString& strPathName), (override));
    MOCK_METHOD(IMS_BOOL, ExistDir, (IN const AString& strPathName), (const, override));
    MOCK_METHOD(AString, GetExternalStoragePath, (), (const, override));
    ;
    MOCK_METHOD(IMS_BOOL, DeleteAllFiles,
            (IN const AString& strPathName, IN const AString& strFileType), (override));
    MOCK_METHOD(ImsList<AString>, GetAllFiles,
            (IN const AString& strPathName, IN const AString& strFileType), (override));
    MOCK_METHOD(IMS_SLONG, GetLastModifiedTime, (IN const AString& strPathName), (override));
};

#endif
