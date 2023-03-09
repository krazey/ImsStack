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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "IFile.h"
#include "MockISystem.h"
#include "OsFile.h"
#include "PlatformContext.h"

using ::testing::_;
using ::testing::Invoke;

namespace android
{

class OsFileTest : public ::testing::Test
{
public:
    MockISystem m_objMockSystem;

    OsFile* m_pOsFile;
    OsFileUtil* m_pOsFileUtil;

    ISystem* m_piDefaultSystem;

protected:
    virtual void SetUp() override
    {
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsFile = new OsFile();
        ASSERT_TRUE(m_pOsFile != nullptr);

        m_pOsFileUtil = new OsFileUtil();
        ASSERT_TRUE(m_pOsFileUtil != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pOsFile != IMS_NULL)
        {
            delete m_pOsFile;
            m_pOsFile = IMS_NULL;
        }
        if (m_pOsFileUtil != IMS_NULL)
        {
            delete m_pOsFileUtil;
            m_pOsFileUtil = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
    }
};

TEST_F(OsFileTest, FileOperation)
{
    AString strFileName;
    EXPECT_EQ(m_pOsFile->Open(strFileName, FILE_OPEN_READWRITE), IMS_FALSE);
    EXPECT_EQ(m_pOsFile->Seek(0, FILE_SEEK_END), IFile::INVALID_VALUE);

    strFileName = "/data/local/tmp/test.txt";
    EXPECT_EQ(m_pOsFile->Open(strFileName, FILE_OPEN_WRITEONLY), IMS_TRUE);
    AString strBuffer("Hello ");
    EXPECT_EQ(m_pOsFile->Write(reinterpret_cast<void*>(strBuffer.GetStr()), strBuffer.GetLength()),
            strBuffer.GetLength());
    m_pOsFile->Close();

    strBuffer = "This is test file";
    EXPECT_EQ(m_pOsFile->Open(strFileName, FILE_OPEN_READWRITE), IMS_TRUE);
    EXPECT_EQ(m_pOsFile->Seek(0, FILE_SEEK_END), 0);
    EXPECT_EQ(m_pOsFile->Write(reinterpret_cast<void*>(strBuffer.GetStr()), strBuffer.GetLength()),
            strBuffer.GetLength());
    m_pOsFile->Close();

    EXPECT_EQ(m_pOsFile->Open(strFileName, FILE_OPEN_READONLY), IMS_TRUE);
    IMS_UINT32 nSize = m_pOsFile->GetSize();
    EXPECT_GT(nSize, 0);

    IMS_CHAR* pcBuffer = new IMS_CHAR[nSize + 1];
    memset(pcBuffer, 0x0, nSize + 1);

    EXPECT_GT(m_pOsFile->Read(reinterpret_cast<void*>(pcBuffer), nSize), 0);
    EXPECT_STREQ(pcBuffer, "Hello This is test file");

    memset(pcBuffer, 0x0, nSize + 1);
    EXPECT_GT(m_pOsFile->GetPos(), 0);
    EXPECT_EQ(m_pOsFile->Read(reinterpret_cast<void*>(pcBuffer), nSize), 0);

    memset(pcBuffer, 0x0, nSize + 1);
    EXPECT_GT(m_pOsFile->GetPos(), 0);
    EXPECT_EQ(m_pOsFile->Seek(6, FILE_SEEK_BEGIN), 0);
    EXPECT_GT(m_pOsFile->Read(reinterpret_cast<void*>(pcBuffer), nSize), 0);
    EXPECT_STREQ(pcBuffer, strBuffer.GetStr());

    delete[] pcBuffer;
    m_pOsFile->Close();

    EXPECT_EQ(m_pOsFileUtil->Delete(strFileName), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->Delete(strFileName), IMS_FALSE);
}

TEST_F(OsFileTest, FileUtil)
{
    EXPECT_STREQ(m_pOsFileUtil->GetFileSeparator(), "/");

    AString strFileName("/data/local/tmp/test_2.txt");
    AString strNewFileName("/data/local/tmp/Test_New.txt");
    EXPECT_EQ(m_pOsFile->Open(strFileName, FILE_OPEN_WRITEONLY), IMS_TRUE);
    m_pOsFile->Close();

    EXPECT_STREQ((m_pOsFileUtil->GetName(strFileName)).GetStr(), "test_2");
    EXPECT_STREQ((m_pOsFileUtil->GetExtension(strFileName)).GetStr(), "txt");
    EXPECT_EQ(m_pOsFileUtil->Exist(strFileName), IMS_TRUE);

    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_OTHER_XO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_OTHER_WO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_OTHER_RO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_GROUP_XO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_GROUP_WO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_GROUP_RO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_USER_XO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_USER_WO), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ChangeMode(strFileName, IFileUtil::MODE_USER_RO), IMS_TRUE);

    EXPECT_EQ(m_pOsFileUtil->Rename(strFileName, strNewFileName), IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->Rename(strFileName, strNewFileName), IMS_FALSE);
    EXPECT_EQ(m_pOsFileUtil->Exist(strFileName), IMS_FALSE);
    EXPECT_EQ(m_pOsFileUtil->Delete(strNewFileName), IMS_TRUE);
}

TEST_F(OsFileTest, Directory)
{
    AString strRootstrDirName;
    EXPECT_EQ(m_pOsFileUtil->MakeDirs(strRootstrDirName, IFileUtil::MODE_USER_RO), IMS_FALSE);

    strRootstrDirName = "/data/local/tmp/ImsUT/PlatformUT";
    EXPECT_EQ(m_pOsFileUtil->MakeDirs(strRootstrDirName,
                      IFileUtil::MODE_USER_RO | IFileUtil::MODE_USER_WO | IFileUtil::MODE_USER_XO),
            IMS_TRUE);
    EXPECT_EQ(m_pOsFileUtil->ExistDir(strRootstrDirName), IMS_TRUE);

    AString strDirName = "/data/local/tmp/ImsUT/PlatformUT/temp1";
    EXPECT_EQ(
            m_pOsFileUtil->MakeDirs(strDirName,
                    IFileUtil::MODE_GROUP_RO | IFileUtil::MODE_GROUP_WO | IFileUtil::MODE_GROUP_XO),
            IMS_TRUE);

    strDirName = "/data/local/tmp/ImsUT/PlatformUT/temp2";
    EXPECT_EQ(
            m_pOsFileUtil->MakeDirs(strDirName,
                    IFileUtil::MODE_OTHER_RO | IFileUtil::MODE_OTHER_WO | IFileUtil::MODE_OTHER_XO),
            IMS_TRUE);

    AString strFileName1 = "/data/local/tmp/ImsUT/PlatformUT/test_1.txt";
    EXPECT_EQ(m_pOsFile->Open(strFileName1, FILE_OPEN_WRITEONLY), IMS_TRUE);
    m_pOsFile->Close();

    AString strFileName2 = "/data/local/tmp/ImsUT/PlatformUT/test_2.txt";
    EXPECT_EQ(m_pOsFile->Open(strFileName2, FILE_OPEN_WRITEONLY), IMS_TRUE);
    m_pOsFile->Close();

    IMSList<AString> objstrFileNameList = m_pOsFileUtil->GetAllFiles(strRootstrDirName);

    EXPECT_EQ(objstrFileNameList.GetSize(), 4);

    EXPECT_STREQ((objstrFileNameList.GetAt(0)).GetStr(), "temp1");
    EXPECT_STREQ((objstrFileNameList.GetAt(1)).GetStr(), "temp2");
    EXPECT_STREQ((objstrFileNameList.GetAt(2)).GetStr(), "test_1.txt");
    EXPECT_STREQ((objstrFileNameList.GetAt(3)).GetStr(), "test_2.txt");

    EXPECT_GT(m_pOsFileUtil->GetLastModifiedTime(strRootstrDirName), 0);

    strRootstrDirName = "/data/local/tmp/ImsUT/";
    EXPECT_EQ(m_pOsFileUtil->DeleteAllFiles(strRootstrDirName), IMS_TRUE);
    m_pOsFileUtil->RemoveDir(strRootstrDirName);

    objstrFileNameList = m_pOsFileUtil->GetAllFiles(strRootstrDirName);
    EXPECT_EQ(objstrFileNameList.GetSize(), 0);
    EXPECT_EQ(m_pOsFileUtil->DeleteAllFiles(strRootstrDirName), IMS_FALSE);
    EXPECT_EQ(m_pOsFileUtil->ExistDir(strRootstrDirName), IMS_FALSE);
}

TEST_F(OsFileTest, GetExternalStoragePath)
{
    AString strValue = "/data/local/tmp//data/local/tmp/";
    EXPECT_CALL(m_objMockSystem, GetExternalStoragePath(_))
            .Times(1)
            .WillOnce(Invoke(
                    [strValue](AString& storagePath)
                    {
                        storagePath = strValue;
                        return 1;
                    }));
    EXPECT_EQ(m_pOsFileUtil->GetExternalStoragePath(), strValue);
}

}  // namespace android
