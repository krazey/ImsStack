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
#include "ServiceFile.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "conf/ConfigFileBuffer.h"

__IMS_TRACE_TAG_CONF__;

PRIVATE GLOBAL const IMS_CHAR ConfigFileBuffer::FILE_EXTENSION[] = "conf";

PUBLIC
ConfigFileBuffer::ConfigFileBuffer(IN const AString& strLocator, IN const AString& strName) :
        ConfigBuffer(strLocator, strName),
        m_nIndexOfWorkSection(0),
        m_pWorkSection(IMS_NULL),
        m_objSections(IMSList<ConfigSection*>())
{
}

PUBLIC VIRTUAL ConfigFileBuffer::~ConfigFileBuffer()
{
    if (!m_objSections.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSections.GetSize(); ++i)
        {
            ConfigSection* pSection = m_objSections.GetAt(i);

            if (pSection != IMS_NULL)
            {
                delete pSection;
            }
        }

        m_objSections.Clear();
    }
}

PRIVATE
ConfigFileBuffer::ConfigFileBuffer() :
        ConfigBuffer(AString::ConstNull(), AString::ConstNull()),
        m_nIndexOfWorkSection(0),
        m_pWorkSection(IMS_NULL),
        m_objSections(IMSList<ConfigSection*>())
{
}

PUBLIC GLOBAL IConfigBuffer* ConfigFileBuffer::CreateFileBuffer(IN const AString& strConfigData)
{
    ConfigFileBuffer* pConfigBuffer = new ConfigFileBuffer();

    if (pConfigBuffer == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!pConfigBuffer->ParseConfig(strConfigData))
    {
        delete pConfigBuffer;

        IMS_TRACE_E(0, "Parsing a configuration data failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pConfigBuffer;
}

PROTECTED VIRTUAL void ConfigFileBuffer::Destroy()
{
    delete this;
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::CaptureSection(IN const IMS_CHAR* pszSectName)
{
    if (m_nIndexOfWorkSection > m_objSections.GetSize())
    {
        m_nIndexOfWorkSection = 0;
    }

    for (IMS_UINT32 i = m_nIndexOfWorkSection; i < m_objSections.GetSize(); ++i)
    {
        ConfigSection* pSection = m_objSections.GetAt(i);

        if (pSection->GetName().EqualsIgnoreCase(pszSectName))
        {
            m_nIndexOfWorkSection = i;
            m_pWorkSection = pSection;
            return IMS_TRUE;
        }
    }

    if (m_nIndexOfWorkSection != 0)
    {
        for (IMS_UINT32 i = 0; i < m_nIndexOfWorkSection; ++i)
        {
            ConfigSection* pSection = m_objSections.GetAt(i);

            if (pSection->GetName().EqualsIgnoreCase(pszSectName))
            {
                m_nIndexOfWorkSection = i;
                m_pWorkSection = pSection;
                return IMS_TRUE;
            }
        }
    }

    IMS_TRACE_E(0, "Section (%s) does not exist", pszSectName, 0, 0);

    m_pWorkSection = IMS_NULL;

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::CaptureSection(
        IN const IMS_CHAR* pszSectName, IN IMS_SINT32 nIndex)
{
    AString strSectName;

    strSectName.Sprintf("%s_%d", pszSectName, nIndex);

    if (m_nIndexOfWorkSection > m_objSections.GetSize())
    {
        m_nIndexOfWorkSection = 0;
    }

    for (IMS_UINT32 i = m_nIndexOfWorkSection; i < m_objSections.GetSize(); ++i)
    {
        ConfigSection* pSection = m_objSections.GetAt(i);

        if (pSection->GetName().EqualsIgnoreCase(strSectName))
        {
            m_nIndexOfWorkSection = i;
            m_pWorkSection = pSection;
            return IMS_TRUE;
        }
    }

    if (m_nIndexOfWorkSection != 0)
    {
        for (IMS_UINT32 i = 0; i < m_nIndexOfWorkSection; ++i)
        {
            ConfigSection* pSection = m_objSections.GetAt(i);

            if (pSection->GetName().EqualsIgnoreCase(strSectName))
            {
                m_nIndexOfWorkSection = i;
                m_pWorkSection = pSection;
                return IMS_TRUE;
            }
        }
    }

    IMS_TRACE_E(0, "Section (%s_%d) does not exist", pszSectName, nIndex, 0);

    m_pWorkSection = IMS_NULL;

    return IMS_FALSE;
}

PROTECTED VIRTUAL void ConfigFileBuffer::ReleaseSection()
{
    m_pWorkSection = IMS_NULL;
}

PROTECTED VIRTUAL IMS_SINT32 ConfigFileBuffer::ReadKeyCount(IN const IMS_CHAR* pszKey) const
{
    if (m_pWorkSection == IMS_NULL)
    {
        return (-1);
    }

    AString strKey(pszKey);
    strKey.Append("_count");

    const AString& strKeyCount = m_pWorkSection->GetValue(strKey.GetStr());

    if (strKeyCount.GetLength() == 0)
    {
        return 0;
    }

    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nKeyCount = strKeyCount.ToInt32(&bOk);

    if (!bOk)
    {
        IMS_TRACE_E(0, "Converting a numeric string (key: %s) failed", strKeyCount.GetStr(), 0, 0);
        return (-1);
    }

    return nKeyCount;
}

PROTECTED VIRTUAL const AString& ConfigFileBuffer::ReadValue(IN const IMS_CHAR* pszKey) const
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s)", pszKey, 0, 0);
        return AString::ConstNull();
    }

    return m_pWorkSection->GetValue(pszKey);
}

PROTECTED VIRTUAL const AString& ConfigFileBuffer::ReadValue(
        IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex) const
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s_%d)", pszKey, nIndex, 0);
        return AString::ConstNull();
    }

    AString strKey;
    strKey.Sprintf("%s_%d", pszKey, nIndex);

    return m_pWorkSection->GetValue(strKey.GetStr());
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::ReadValueBoolean(IN const IMS_CHAR* pszKey) const
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s)", pszKey, 0, 0);
        return IMS_FALSE;
    }

    const AString& strValue = m_pWorkSection->GetValue(pszKey);

    // If the value is not "true" in case-insensitively, it returns IMS_FALSE.
    if (strValue.EqualsIgnoreCase(TextParser::STR_SMALL_TRUE))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_SINT32 ConfigFileBuffer::ReadValueInt(IN const IMS_CHAR* pszKey) const
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s)", pszKey, 0, 0);
        return (-1);
    }

    const AString& strValue = m_pWorkSection->GetValue(pszKey);
    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nValue = strValue.ToInt32(&bOk);

    if (!bOk)
    {
        IMS_TRACE_E(0, "Converting a numeric string (value: %s) failed", strValue.GetStr(), 0, 0);
        return (-1);
    }

    return nValue;
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::WriteKeyCount(
        IN const IMS_CHAR* pszKey, IN IMS_SINT32 nCount)
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "There is no captured section : key (%s_count), value (%d)", pszKey, nCount, 0);
        return IMS_FALSE;
    }

    AString strKey(pszKey);
    strKey.Append("_count");

    AString strValue;
    strValue.SetNumber(nCount);

    return m_pWorkSection->SetValue(strKey.GetStr(), strValue);
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::WriteValue(
        IN const IMS_CHAR* pszKey, IN const AString& strValue)
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s), value (%d)", pszKey,
                strValue.GetStr(), 0);
        return IMS_FALSE;
    }

    return m_pWorkSection->SetValue(pszKey, strValue);
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::WriteValue(
        IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex, IN const AString& strValue)
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s_%d), value (%d)", pszKey, nIndex,
                strValue.GetStr());
        return IMS_FALSE;
    }

    AString strKey;
    strKey.Sprintf("%s_%d", pszKey, nIndex);

    return m_pWorkSection->SetValue(strKey.GetStr(), strValue);
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::WriteValueBoolean(
        IN const IMS_CHAR* pszKey, IN IMS_BOOL bValue)
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s), value (%s)", pszKey,
                TextParser::BooleanToString(bValue), 0);
        return IMS_FALSE;
    }

    return m_pWorkSection->SetValue(pszKey, TextParser::BooleanToString(bValue));
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::WriteValueInt(
        IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue)
{
    if (m_pWorkSection == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no captured section : key (%s), value (%d)", pszKey, nValue, 0);
        return IMS_FALSE;
    }

    if (nValue < 0)
    {
        return IMS_FALSE;
    }

    AString strValue;
    strValue.SetNumber(nValue);

    return m_pWorkSection->SetValue(pszKey, strValue);
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::WriteToMedium() const
{
    AString strConfigData;
    AString strConfName = GetLocator();

    if (IMS_FILE_Exist(strConfName))
    {
        AString strNewConfName = strConfName + ".bak";

        // If the back-up file exists, delete it.
        if (IMS_FILE_Exist(strNewConfName))
        {
            IMS_FILE_Delete(strNewConfName);
        }

        IMS_FILE_Rename(strConfName, strNewConfName);
    }

    IFile* piFile = IMS_FILE_Create();

    if (piFile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piFile->Open(strConfName, FILE_OPEN_WRITEONLY))
    {
        IMS_TRACE_E(0, "Opening the configuration (%s) failed", strConfName.GetStr(), 0, 0);

        piFile->Close();
        IMS_FILE_Destroy(piFile);

        return IMS_FALSE;
    }

    FormConfig(strConfigData);

    if (piFile->Write(reinterpret_cast<void*>(strConfigData.GetStr()), strConfigData.GetLength()) ==
            0)
    {
        IMS_TRACE_E(0, "Writing the configuration (%s) failed", strConfName.GetStr(), 0, 0);

        piFile->Close();
        IMS_FILE_Destroy(piFile);
        return IMS_FALSE;
    }

    piFile->Close();
    IMS_FILE_Destroy(piFile);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL ConfigFileBuffer::Create(IN IMS_SINT32 nId)
{
    // FIXME: if file configuration is required, please use this input argument.
    (void)nId;
    AString strConfName = ResolveLocator();

    IFile* piFile = IMS_FILE_Create();

    if (piFile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piFile->Open(strConfName, FILE_OPEN_READONLY))
    {
        IMS_TRACE_E(0, "Opening the configuration (%s) failed", strConfName.GetStr(), 0, 0);

        piFile->Close();
        IMS_FILE_Destroy(piFile);
        return IMS_FALSE;
    }

    IMS_SINT32 nBuffSize = static_cast<IMS_SINT32>(piFile->GetSize());

    if (nBuffSize == 0)
    {
        IMS_TRACE_E(
                0, "Configuration (%s) does not exist; Size is zero", strConfName.GetStr(), 0, 0);

        piFile->Close();
        IMS_FILE_Destroy(piFile);
        return IMS_FALSE;
    }

    IMS_CHAR* pcBuffer = new IMS_CHAR[nBuffSize + 1];

    if (pcBuffer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating the buffer for the configuration (%s) failed",
                strConfName.GetStr(), 0, 0);

        piFile->Close();
        IMS_FILE_Destroy(piFile);
        return IMS_FALSE;
    }

    IMS_UINT32 nReadSize;

    if ((nReadSize = piFile->Read(reinterpret_cast<void*>(pcBuffer), nBuffSize)) == 0)
    {
        IMS_TRACE_E(0, "Reading the configuration (%s) failed", strConfName.GetStr(), 0, 0);

        delete[] pcBuffer;
        piFile->Close();
        IMS_FILE_Destroy(piFile);
        return IMS_FALSE;
    }

    piFile->Close();
    IMS_FILE_Destroy(piFile);

    pcBuffer[nReadSize] = '\0';

    AString strConfigData;

    strConfigData.Attach(pcBuffer, nReadSize);

    if (!ParseConfig(strConfigData))
    {
        IMS_TRACE_E(0, "Parsing the configuration (%s) failed", strConfName.GetStr(), 0, 0);

        delete[] pcBuffer;
        return IMS_FALSE;
    }

    delete[] pcBuffer;

    return IMS_TRUE;
}

PRIVATE
void ConfigFileBuffer::FormConfig(OUT AString& strConfigData) const
{
    strConfigData.Append(m_objStartComment.ToString());

    // Inserts CRLF between the main comment & the start section
    for (IMS_UINT32 i = 0; i < m_objSections.GetSize(); ++i)
    {
        const ConfigSection* pSection = m_objSections.GetAt(i);

        strConfigData.Append(TextParser::STR_CRLF);
        strConfigData.Append(pSection->ToString());
    }

    strConfigData.Append(m_objEndComment.ToString());
}

PRIVATE
IMS_BOOL ConfigFileBuffer::ParseConfig(IN const AString& strConfigData)
{
    IMSList<AString> objLines = strConfigData.Split(TextParser::CHAR_LF);
    IMS_UINT32 i;
    IMS_UINT32 nConfigStart = 0;

    // Find the start line to parse the configuration
    for (i = 0; i < objLines.GetSize(); ++i)
    {
        const AString& strLine = objLines.GetAt(i);

        // If the line is a comment, skip it.
        if (strLine.StartsWith(TextParser::CHAR_SEMICOLON))
        {
            m_objStartComment.Add(strLine);
            continue;
        }

        // Find the first empty line from the file
        if (strLine.IsEmpty())
        {
            nConfigStart = i + 1;
            break;
        }

        // I got the section data.
        if (strLine.StartsWith(TextParser::CHAR_LSBRACKET) &&
                strLine.EndsWith(TextParser::CHAR_RSBRACKET))
        {
            nConfigStart = i;
            break;
        }
    }

    ConfigSection* pSection = IMS_NULL;
    IMS_UINT32 nCommentStart = 0;
    IMS_BOOL bCommentPresent = IMS_FALSE;
    IMS_BOOL bSectionStarted = IMS_FALSE;

    for (i = nConfigStart; i < objLines.GetSize(); ++i)
    {
        const AString& strLine = objLines.GetAt(i);

        // If the line starts with SEMI-COLON, it is a comment.
        if (strLine.StartsWith(TextParser::CHAR_SEMICOLON))
        {
            if (!bCommentPresent)
            {
                nCommentStart = i;
                bCommentPresent = IMS_TRUE;
            }
            continue;
        }

        // Start of a section

        // If the line starts with the LSBRACKET and ends with the RSBRACKET,
        // I got the section data.
        if (strLine.StartsWith(TextParser::CHAR_LSBRACKET) &&
                strLine.EndsWith(TextParser::CHAR_RSBRACKET))
        {
            if (pSection != IMS_NULL)
            {
                delete pSection;

                IMS_TRACE_E(0, "New section is started before the end of a section", 0, 0, 0);
                return IMS_FALSE;
            }

            pSection = new ConfigSection();

            if (pSection == IMS_NULL)
            {
                return IMS_FALSE;
            }

            pSection->SetName(strLine.GetSubStr(1, strLine.GetLength() - 2).Trim());

            // add a comments
            if (bCommentPresent)
            {
                for (IMS_UINT32 j = nCommentStart; j < i; ++j)
                {
                    pSection->AddComment(objLines.GetAt(j));
                }

                bCommentPresent = IMS_FALSE;
            }

            bSectionStarted = IMS_TRUE;
            continue;
        }

        if (bSectionStarted)
        {
            // End of a section
            if (strLine.GetLength() == 0)
            {
                m_objSections.Append(pSection);

                pSection = IMS_NULL;
                nCommentStart = 0;
                bCommentPresent = IMS_FALSE;
                bSectionStarted = IMS_FALSE;
            }
            // Parameters of the current section
            else
            {
                if (pSection == IMS_NULL)
                {
                    IMS_TRACE_E(0, "Parameter comes without starting the section", 0, 0, 0);
                    return IMS_FALSE;
                }

                if (!pSection->AddSectionData(strLine))
                {
                    delete pSection;

                    IMS_TRACE_E(0, "Parameter (%s) is malformed", strLine.GetStr(), 0, 0);
                    return IMS_FALSE;
                }

                if (bCommentPresent)
                {
                    ConfigSectionData* pData = pSection->GetLastElement();

                    for (IMS_UINT32 j = nCommentStart; j < i; ++j)
                    {
                        pData->AddComment(objLines.GetAt(j));
                    }

                    bCommentPresent = IMS_FALSE;
                }
            }
        }
    }

    // Last line does not include the empty line
    if (bSectionStarted && (pSection != IMS_NULL))
    {
        m_objSections.Append(pSection);
    }

    // The configuration is ended with the comments.
    if (bCommentPresent)
    {
        for (IMS_UINT32 j = nCommentStart; j < i; ++j)
        {
            m_objEndComment.Add(objLines.GetAt(j));
        }
    }

#ifdef __IMS_DEBUG__
    IMS_TRACE_D("Section (%d)", m_objSections.GetSize(), 0, 0);
#endif

    return IMS_TRUE;
}

PRIVATE
AString ConfigFileBuffer::ResolveLocator() const
{
    AString strLocator(GetLocator());

    const AString& strConfName = GetName();
    IMSList<AString> objTokens = strConfName.Split(TextParser::CHAR_DOT);
    const IMS_CHAR* pszFileSeparator = IMS_FILE_GetSeparator();

    // 4 if the last dir. is not matched with "gims", then add this dir. name.

    // root directory name for configuration files
    strLocator.Append(pszFileSeparator);

    // file path
    if (objTokens.GetSize() > 2)
    {
        // The first element: IMS is ignored
        for (IMS_UINT32 i = 1; i < objTokens.GetSize(); ++i)
        {
            strLocator.Append(objTokens.GetAt(i));
            strLocator.Append(pszFileSeparator);
        }
    }

    // file name
    strLocator.Append(strConfName);

    // file extension
    strLocator.Append(TextParser::CHAR_DOT);
    strLocator.Append(FILE_EXTENSION);

    IMS_TRACE_D("Configuration Locator - (%s)", strLocator.GetStr(), 0, 0);

    return strLocator;
}
