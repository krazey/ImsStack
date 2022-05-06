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
#ifndef INTERFACE_FILE_H_
#define INTERFACE_FILE_H_

#include "AString.h"

/// Open mode
enum FILE_OPEN_ENTYPE
{
    FILE_OPEN_READWRITE = 0,
    FILE_OPEN_READONLY = 1,
    FILE_OPEN_WRITEONLY = 2
};

/// Seek mode
enum FILE_SEEK_ENTYPE
{
    /// Begin of the file
    FILE_SEEK_BEGIN = 0,
    /// Current position of the file
    FILE_SEEK_CURRENT,
    /// End of the file
    FILE_SEEK_END
};

class IFile
{
public:
    /**
     * @brief Creates or opens a file.
     *
     * @param strName The file name
     * @param eMode The desired mode to create or open a file
     * @return IMS_TRUE if a file is created or opened, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Open(IN const AString& strName, IN FILE_OPEN_ENTYPE eMode) = 0;

    /**
     * @brief Closes the handle of a created or open file.
     */
    virtual void Close() = 0;

    /**
     * @brief Moves the file pointer of an open file from the offset, nPosition.
     *
     * @param nOffset The number of bytes to move the file pointer
     * @param eFrom The start point for the file pointer move
     * @return A new file pointer if the operation succeeds, INVALID_VALUE otherwise.
     */
    virtual IMS_UINT32 Seek(IN IMS_UINT32 nOffset, IN FILE_SEEK_ENTYPE eFrom) const = 0;

    /**
     * @brief Reads data from a file, starting at the position indicated by the file pointer.
     *
     * @param pBuffer The pointer to the buffer that receives the date read from the file
     * @param nNumberOfBytesToRead The number of bytes to be read from the file
     * @return Non-zero if the operation succeeds, 0 otherwise.
     */
    virtual IMS_UINT32 Read(OUT void* pBuffer, IN IMS_UINT32 nNumberOfBytesToRead) = 0;

    /**
     * @brief Writes data to a file, starting at the position indicated by the file pointer.
     *
     * @param pBuffer The pointer to the buffer containing the date to write the file
     * @param nNumberOfBytesToWrite The number of bytes to be written to the file
     * @return Non-zero if the operation succeeds, 0 otherwise.
     */
    virtual IMS_UINT32 Write(IN void* pBuffer, IN IMS_UINT32 nNumberOfBytesToWrite) = 0;

    /**
     * @brief Retrieves the size, in bytes, of the specified file.
     *
     * @return The file size.
     */
    virtual IMS_UINT32 GetSize() const = 0;

    /**
     * @brief Retrieves the current file pointer.
     *
     * @return The current file pointer.
     */
    virtual IMS_UINT32 GetPos() const = 0;

public:
    // Invalid value: Seek(), GetPos()
    enum
    {
        INVALID_VALUE = 0xFFFFFFFF
    };
};

class IFileUtil
{
public:
    /**
     * @brief Changes the file mode to share the file to another user / module.
     *
     * @param strFileName The file name to be changed
     * @param nMode The file mode
     * @return IMS_TRUE if the operation succeeds, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL ChangeMode(IN const AString& strFileName, IN IMS_SINT32 nMode) const = 0;

    /**
     * @brief Changes the file owner.
     *
     * @param strFileName The file name to be changed
     * @param nUid The user id of this file
     * @param nGid The group id of this file
     * @return IMS_TRUE if the operation succeeds, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL ChangeOwner(
            IN const AString& strFileName, IN IMS_SINT32 nUid, IN IMS_SINT32 nGid) const = 0;

    /**
     * @brief Checks if the specified file exists.
     *
     * @param strFileName The file name to be checked
     * @return IMS_TRUE if the specified file exists, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Exist(IN const AString& strFileName) const = 0;

    /**
     * @brief Renames the existing file name to new file name.
     *
     * @param strOldName The existing file name to be renamed
     * @param strNewName The new file name to be renamed
     * @return IMS_TRUE if the operation succeeds, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Rename(IN const AString& strOldName, IN const AString& strNewName) const = 0;

    /**
     * @brief Deletes an existing file from a file system.
     *
     * @param strFileName The file name to be deleted
     * @return IMS_TRUE if the operation succeeds, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Delete(IN const AString& strFileName) const = 0;

    /**
     * @brief Extracts the file name without the directory path & file extension from
     *        the specified file name.
     *
     * @param strFilePath The file name with full path
     * @return The file name only from the specified file path.
     */
    virtual AString GetName(IN const AString& strFilePath) const = 0;

    /**
     * @brief Extracts the file extension only from the specified file name.
     *
     * @param strFileName The file name
     * @return The file extension only from the specified file.
     */
    virtual AString GetExtension(IN const AString& strFileName) const = 0;

    /**
     * @brief Returns the directory separator of the file system.
     *
     * @return The file separator.
     */
    virtual const IMS_CHAR* GetFileSeparator() const = 0;

    /**
     * @brief Creates an directory to file system.
     *
     * @param strPathName The directory name to be created
     * @param nMode The directory access mode
     * @return IMS_TRUE if the operation succeeds, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL MakeDir(IN const AString& strPathName, IN IMS_SINT32 nMode) const = 0;

    /**
     * @brief Creates directory with parents to file system.
     *
     * @param strPathName The directory name to be created
     * @param nMode The directory access mode
     * @return IMS_TRUE if the directory is created or already exists, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL MakeDirs(IN const AString& strPathName, IN IMS_SINT32 nMode) const = 0;

    /**
     * @brief Deletes an existing empty directory from a file system.
     *
     * @param strPathName The directory name to be deleted
     */
    virtual void RemoveDir(IN const AString& strPathName) = 0;

    /**
     * @brief Checks if the specified directory exists.
     *
     * @param strPathName The directory name to be checked
     * @return IMS_TRUE if the directory exists, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL ExistDir(IN const AString& strPathName) const = 0;

    /**
     * @brief Gets the external storage Path.
     *
     * @return The external storage path.
     */
    virtual AString GetExternalStoragePath() const = 0;

    /**
     * @brief Deletes all files that exists under the given path.
     *
     * Especially the strFileType is provied, only files that its extension is same
     * with given strFileType are deleted.
     *
     * @param strPathName The directory path
     * @param strFileType The file extension that should be deleted
     * @return IMS_TRUE if the operation succeeds, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL DeleteAllFiles(IN const AString& strPathName,
            IN const AString& strFileType = AString::ConstNull()) = 0;

    virtual IMSList<AString> GetAllFiles(IN const AString& strPathName,
            IN const AString& strFileType = AString::ConstNull()) = 0;

    virtual IMS_SLONG GetLastModifiedTime(IN const AString& strPathName) = 0;

public:
    /// UID & GID for access permission
    /// In this time, only supports: root/system for user/group id
    enum
    {
        UID_NONE = (-1),  // no change
        UID_ROOT = 1,
        UID_SYSTEM = 2
    };

    enum
    {
        GID_NONE = (-1),  // no change
        GID_ROOT = 1,
        GID_SYSTEM = 2
    };

    /// Mode bits for access permission
    enum
    {
        /// file mode: user, read-only, write-only, executable-only, all
        MODE_USER_RO = 0x0001,
        MODE_USER_WO = 0x0002,
        MODE_USER_XO = 0x0004,
        MODE_USER_RWX = 0x0007,

        /// file mode: group, read-only, write-only, executable-only, all
        MODE_GROUP_RO = 0x0010,
        MODE_GROUP_WO = 0x0020,
        MODE_GROUP_XO = 0x0040,
        MODE_GROUP_RWX = 0x0070,

        /// file mode: other, read-only, write-only, executable-only, all
        MODE_OTHER_RO = 0x0100,
        MODE_OTHER_WO = 0x0200,
        MODE_OTHER_XO = 0x0400,
        MODE_OTHER_RWX = 0x0700
    };
};

#endif
