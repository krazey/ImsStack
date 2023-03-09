/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include <gtest/gtest.h>

#include "ImsFile.h"
#include "MockIOsFactory.h"
#include "OsFile.h"
#include "PlatformContext.h"
#include "ServiceFile.h"

using ::testing::Return;

namespace android
{

class FileServiceTest : public ::testing::Test
{
public:
    inline FileServiceTest() :
            m_pFileService(IMS_NULL),
            m_piOldOsFactory(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_piOldOsFactory = PlatformContext::GetInstance()->SetOsFactory(&m_objOsFactory);
        m_pFileService = FileService::GetFileService();
        ASSERT_TRUE(m_pFileService != nullptr);
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetOsFactory(m_piOldOsFactory);
    }

public:
    FileService* m_pFileService;
    MockIOsFactory m_objOsFactory;
    IOsFactory* m_piOldOsFactory;
};

TEST_F(FileServiceTest, CreateFile)
{
    OsFile objOsFile;

    EXPECT_CALL(m_objOsFactory, CreateFile).Times(1).WillOnce(Return(&objOsFile));

    EXPECT_TRUE(m_pFileService->CreateFile() == &objOsFile);
}

TEST_F(FileServiceTest, DestroyFile)
{
    IFile* pIFile = new OsFile();

    m_pFileService->DestroyFile(pIFile);

    EXPECT_TRUE(pIFile == nullptr);
}

TEST_F(FileServiceTest, GetFileUtil)
{
    OsFileUtil objOsFileUtil;

    EXPECT_CALL(m_objOsFactory, CreateFileUtil).Times(1).WillOnce(Return(&objOsFileUtil));

    EXPECT_TRUE(m_pFileService->GetFileUtil() == &objOsFileUtil);
}

}  // namespace android
