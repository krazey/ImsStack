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
#ifndef TEST_FILE_SERVICE_H_
#define TEST_FILE_SERVICE_H_

#include "MockIFile.h"
#include "ServiceFile.h"

class TestFileService : public FileService
{
public:
    inline IFile* CreateFile() override { return &m_objFile; }
    inline void DestroyFile(IN IFile*& /*piFile*/) override {}
    inline IFileUtil* GetFileUtil() override { return &m_objFileUtil; }

    inline MockIFile& GetMockFile() { return m_objFile; }
    inline MockIFileUtil& GetMockFileUtil() { return m_objFileUtil; }

private:
    MockIFile m_objFile;
    MockIFileUtil m_objFileUtil;
};

#endif
