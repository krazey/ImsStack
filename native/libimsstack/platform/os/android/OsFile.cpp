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
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <private/android_filesystem_config.h>
#include <sys/stat.h>

#include "OsFile.h"
#include "PlatformContext.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_IPL__;

class OsFilePrivate
{
public:
    inline OsFilePrivate() :
            m_pFile(IMS_NULL)
    {
    }

    inline ~OsFilePrivate()
    {
        if (m_pFile != IMS_NULL)
        {
            fclose(m_pFile);
            m_pFile = IMS_NULL;
        }
    }

public:
    FILE* m_pFile;
};

PUBLIC
OsFile::OsFile() :
        m_pFileP(new OsFilePrivate())
{
}

PUBLIC VIRTUAL OsFile::~OsFile()
{
    Close();

    if (m_pFileP != IMS_NULL)
    {
        delete m_pFileP;
        m_pFileP = IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_BOOL OsFile::Open(IN const AString& strName, IN FILE_OPEN_ENTYPE eMode)
{
    if (strName.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    const IMS_CHAR MODE[][3] = {"r+", "r", "w"};
    IMS_UINT32 nMode = 1;

    switch (eMode)
    {
        case FILE_OPEN_READWRITE:
            nMode = 0;
            break;
        case FILE_OPEN_READONLY:
            nMode = 1;
            break;
        case FILE_OPEN_WRITEONLY:
            nMode = 2;
            break;
        default:
            return IMS_FALSE;
    }

    m_pFileP->m_pFile = fopen(strName.GetStr(), MODE[nMode]);

    return (m_pFileP->m_pFile == IMS_NULL) ? IMS_FALSE : IMS_TRUE;
}

PUBLIC VIRTUAL void OsFile::Close()
{
    if (m_pFileP->m_pFile != IMS_NULL)
    {
        fclose(m_pFileP->m_pFile);
        m_pFileP->m_pFile = IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_UINT32 OsFile::Seek(IN IMS_UINT32 nOffset, IN FILE_SEEK_ENTYPE eFrom) const
{
    if (m_pFileP->m_pFile == IMS_NULL)
    {
        return INVALID_VALUE;
    }

    IMS_SINT32 nMoveMethod;

    switch (eFrom)
    {
        case FILE_SEEK_BEGIN:
            nMoveMethod = SEEK_SET;
            break;
        case FILE_SEEK_CURRENT:
            nMoveMethod = SEEK_CUR;
            break;
        case FILE_SEEK_END:
            nMoveMethod = SEEK_END;
            break;
        default:
            return INVALID_VALUE;
    }

    if (fseek(m_pFileP->m_pFile, (long)nOffset, nMoveMethod) == 0)
    {
        return 0;
    }

    return INVALID_VALUE;
}

PUBLIC VIRTUAL IMS_UINT32 OsFile::Read(OUT void* pBuffer, IN IMS_UINT32 nNumberOfBytesToRead)
{
    IMS_SINT32 nReadBytes = 0;

    if (m_pFileP->m_pFile != IMS_NULL)
    {
        nReadBytes = fread(pBuffer, 1, nNumberOfBytesToRead, m_pFileP->m_pFile);
    }

    return (nReadBytes > 0) ? nReadBytes : 0;
}

PUBLIC VIRTUAL IMS_UINT32 OsFile::Write(IN void* pBuffer, IN IMS_UINT32 nNumberOfBytesToWrite)
{
    IMS_SINT32 nWrittenBytes = 0;

    if (m_pFileP->m_pFile != IMS_NULL)
    {
        nWrittenBytes = fwrite(pBuffer, 1, nNumberOfBytesToWrite, m_pFileP->m_pFile);
    }

    return (nWrittenBytes > 0) ? nWrittenBytes : 0;
}

PUBLIC VIRTUAL IMS_UINT32 OsFile::GetSize() const
{
    IMS_UINT32 nSize = 0;
    fpos_t nCurPos;

    if (0 == fgetpos(m_pFileP->m_pFile, &nCurPos))
    {
        if (-1 == fseek(m_pFileP->m_pFile, 0L, SEEK_END))
        {
            return 0;
        }

        nSize = ftell(m_pFileP->m_pFile);

        fsetpos(m_pFileP->m_pFile, static_cast<const fpos_t*>(&nCurPos));
    }

    return nSize;
}

PUBLIC VIRTUAL IMS_UINT32 OsFile::GetPos() const
{
    if (Seek(0, FILE_SEEK_CURRENT) == INVALID_VALUE)
    {
        return INVALID_VALUE;
    }

    long nPos = ftell(m_pFileP->m_pFile);

    if (nPos < 0)
    {
        return INVALID_VALUE;
    }

    // FIXME: adjust the type of return value
    return LONG_TO_INT(nPos);
}

PRIVATE GLOBAL const IMS_CHAR OsFileUtil::FILE_SEPARATOR[] = "/";

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::ChangeMode(
        IN const AString& strFileName, IN IMS_SINT32 nMode) const
{
    mode_t nNewMode = 0;

    if ((nMode & MODE_USER_RO) == MODE_USER_RO)
    {
        nNewMode |= S_IRUSR;
    }

    if ((nMode & MODE_USER_WO) == MODE_USER_WO)
    {
        nNewMode |= S_IWUSR;
    }

    if ((nMode & MODE_USER_XO) == MODE_USER_XO)
    {
        nNewMode |= S_IXUSR;
    }

    if ((nMode & MODE_GROUP_RO) == MODE_GROUP_RO)
    {
        nNewMode |= S_IRGRP;
    }

    if ((nMode & MODE_GROUP_WO) == MODE_GROUP_WO)
    {
        nNewMode |= S_IWGRP;
    }

    if ((nMode & MODE_GROUP_XO) == MODE_GROUP_XO)
    {
        nNewMode |= S_IXGRP;
    }

    if ((nMode & MODE_OTHER_RO) == MODE_OTHER_RO)
    {
        nNewMode |= S_IROTH;
    }

    if ((nMode & MODE_OTHER_WO) == MODE_OTHER_WO)
    {
        nNewMode |= S_IWOTH;
    }

    if ((nMode & MODE_OTHER_XO) == MODE_OTHER_XO)
    {
        nNewMode |= S_IXOTH;
    }

    if (chmod(strFileName.GetStr(), nNewMode) != 0)
    {
        IMS_TRACE_E(0, "chmod(%s) failed (%d, %s)", strFileName.GetStr(), errno, strerror(errno));
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::ChangeOwner(
        IN const AString& strFileName, IN IMS_SINT32 nUid, IN IMS_SINT32 nGid) const
{
    // (-1) : no changes
    uid_t nOwner = -1;
    gid_t nGroup = -1;

    // owner id
    if (nUid == UID_ROOT)
    {
        nOwner = AID_ROOT;
    }
    else if (nUid == UID_SYSTEM)
    {
        nOwner = AID_SYSTEM;
    }

    // group id
    if (nGid == GID_ROOT)
    {
        nGroup = AID_ROOT;
    }
    else if (nGid == GID_SYSTEM)
    {
        nGroup = AID_SYSTEM;
    }

    if (chown(strFileName.GetStr(), nOwner, nGroup) != 0)
    {
        IMS_TRACE_E(0, "chown(%s) failed (%s, %d)", strFileName.GetStr(), errno, strerror(errno));
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::Exist(IN const AString& strFileName) const
{
    return (access(strFileName.GetStr(), F_OK) == 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::Rename(
        IN const AString& strOldName, IN const AString& strNewName) const
{
    return (rename(strOldName.GetStr(), strNewName.GetStr()) == 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::Delete(IN const AString& strFileName) const
{
    return (remove(strFileName.GetStr()) == 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL AString OsFileUtil::GetName(IN const AString& strFilePath) const
{
    AString strFileName = strFilePath.Trim();
    IMS_SINT32 nIndex = strFileName.GetLastIndexOf(FILE_SEPARATOR[0]);

    if (nIndex != AString::NPOS)
    {
        strFileName = strFileName.GetSubStr(nIndex + 1);
    }

    nIndex = strFileName.GetLastIndexOf('.');

    return (nIndex == AString::NPOS) ? strFileName : strFileName.GetSubStr(0, nIndex);
}

PUBLIC VIRTUAL AString OsFileUtil::GetExtension(IN const AString& strFileName) const
{
    IMS_SINT32 nIndex = strFileName.GetLastIndexOf('.');

    return (nIndex == AString::NPOS) ? AString::ConstNull() : strFileName.GetSubStr(nIndex + 1);
}

PUBLIC VIRTUAL const IMS_CHAR* OsFileUtil::GetFileSeparator() const
{
    return FILE_SEPARATOR;
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::MakeDir(
        IN const AString& strPathName, IN IMS_SINT32 nMode) const
{
    mode_t nNewMode = 0;

    if ((nMode & MODE_USER_RO) == MODE_USER_RO)
    {
        nNewMode |= S_IRUSR;
    }

    if ((nMode & MODE_USER_WO) == MODE_USER_WO)
    {
        nNewMode |= S_IWUSR;
    }

    if ((nMode & MODE_USER_XO) == MODE_USER_XO)
    {
        nNewMode |= S_IXUSR;
    }

    if ((nMode & MODE_GROUP_RO) == MODE_GROUP_RO)
    {
        nNewMode |= S_IRGRP;
    }

    if ((nMode & MODE_GROUP_WO) == MODE_GROUP_WO)
    {
        nNewMode |= S_IWGRP;
    }

    if ((nMode & MODE_GROUP_XO) == MODE_GROUP_XO)
    {
        nNewMode |= S_IXGRP;
    }

    if ((nMode & MODE_OTHER_RO) == MODE_OTHER_RO)
    {
        nNewMode |= S_IROTH;
    }

    if ((nMode & MODE_OTHER_WO) == MODE_OTHER_WO)
    {
        nNewMode |= S_IWOTH;
    }

    if ((nMode & MODE_OTHER_XO) == MODE_OTHER_XO)
    {
        nNewMode |= S_IXOTH;
    }

    return (mkdir(strPathName.GetStr(), nNewMode) == 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::MakeDirs(
        IN const AString& strPathName, IN IMS_SINT32 nMode) const
{
    if (strPathName.GetLength() <= 0)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objFolders = strPathName.Split('/');
    AString strPath = "/";
    for (IMS_UINT32 i = 0; i < objFolders.GetSize(); ++i)
    {
        strPath.Append(objFolders.GetAt(i)).Append("/");

        if (ExistDir(strPath) == IMS_FALSE)
        {
            if (MakeDir(strPath, nMode) == IMS_FALSE)
            {
                IMS_TRACE_E(0, "Failed to make directory (%s)", strPath.GetStr(), 0, 0);
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsFileUtil::RemoveDir(IN const AString& strPathName)
{
    rmdir(strPathName.GetStr());
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::ExistDir(IN const AString& strPathName) const
{
    return (access(strPathName.GetStr(), F_OK) == 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL AString OsFileUtil::GetExternalStoragePath() const
{
    AString strExternalStoragePath = AString::ConstNull();

    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();
    piSystem->GetExternalStoragePath(strExternalStoragePath);

    IMS_TRACE_D("GetExternalStoragePath(%s)", strExternalStoragePath.GetStr(), 0, 0);

    return strExternalStoragePath;
}

PUBLIC VIRTUAL IMS_BOOL OsFileUtil::DeleteAllFiles(
        IN const AString& strPathName, IN const AString& strFileType /*= AString::ConstNull()*/)
{
    IMS_TRACE_D("DeleteAllFiles [%s] [%s]", strPathName.GetStr(), strFileType.GetStr(), 0);

    DIR* pDir = opendir(strPathName.GetStr());

    if (pDir == IMS_NULL)
    {
        IMS_TRACE_E(0, "DeleteAllFiles :: opendir failed", 0, 0, 0);
        return IMS_FALSE;
    }

    dirent* pFile = IMS_NULL;

    while ((pFile = readdir(pDir)) != IMS_NULL)
    {
        AString strFileName(pFile->d_name);

        if (strFileName.Equals(".") || strFileName.Equals(".."))
        {
            IMS_TRACE_D("DeleteAllFiles :: . or .. : %s", pFile->d_name, 0, 0);
            continue;
        }

        AString strSubPathName(strPathName);
        strSubPathName.Append(strFileName);

        DIR* pSubDir = opendir(strSubPathName.GetStr());

        if (pSubDir != NULL)
        {
            // If it is the sub-directory
            strSubPathName.Append("/");

            IMS_TRACE_D("DeleteAllFiles :: sub-dir=%s", strSubPathName.GetStr(), 0, 0);

            closedir(pSubDir);

            DeleteAllFiles(strSubPathName, strFileType);

            // delete the sub-directory if it is empty and all file should be deleted
            if (strFileType.GetLength() == 0)
            {
                RemoveDir(strSubPathName);
            }
        }
        else
        {
            // If it is the file
            IMS_TRACE_D("DeleteAllFiles :: file=%s", strSubPathName.GetStr(), 0, 0);

            AString strFileExtension;

            IMS_SINT32 nIndex = strFileName.GetLastIndexOf(".");

            if (nIndex > 0 && nIndex < strFileName.GetLength())
            {
                strFileExtension = strFileName.GetSubStr(nIndex + 1);
            }

            // Delete the file only if its file type is same with given strFileType
            if ((strFileType.GetLength() == 0) ||
                    ((strFileType.GetLength() > 0) &&
                            strFileType.EqualsIgnoreCase(strFileExtension)))
            {
                Delete(strSubPathName);
            }
        }
    }

    closedir(pDir);

    return IMS_TRUE;
}

PUBLIC VIRTUAL ImsList<AString> OsFileUtil::GetAllFiles(
        IN const AString& strPathName, IN const AString& strFileType /*= AString::ConstNull()*/)
{
    IMS_TRACE_D("GetAllFiles [%s] [%s]", strPathName.GetStr(), strFileType.GetStr(), 0);

    ImsList<AString> objFileNameList;
    DIR* pDir = opendir(strPathName.GetStr());

    if (pDir == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetAllFiles :: opendir failed", 0, 0, 0);
        return objFileNameList;
    }

    dirent* pFile = IMS_NULL;

    while ((pFile = readdir(pDir)) != IMS_NULL)
    {
        AString strFileName(pFile->d_name);

        if (strFileName.Equals(".") || strFileName.Equals(".."))
        {
            IMS_TRACE_D("GetAllFiles :: . or .. : %s", pFile->d_name, 0, 0);
            continue;
        }

        AString strFileExtension;

        IMS_SINT32 nIndex = strFileName.GetLastIndexOf(".");

        if (nIndex > 0 && nIndex < strFileName.GetLength())
        {
            strFileExtension = strFileName.GetSubStr(nIndex + 1);
        }

        if ((strFileType.GetLength() == 0) ||
                ((strFileType.GetLength() > 0) && strFileType.EqualsIgnoreCase(strFileExtension)))
        {
            objFileNameList.Append(strFileName);
        }
    }

    closedir(pDir);

    return objFileNameList;
}

PUBLIC VIRTUAL IMS_SLONG OsFileUtil::GetLastModifiedTime(IN const AString& strPathName)
{
    struct stat info;

    IMS_MEM_Memset(&info, 0, sizeof(struct stat));

    if (stat(strPathName.GetStr(), &info) < 0)
    {
        return -1;
    }

    return info.st_ctime;
}
