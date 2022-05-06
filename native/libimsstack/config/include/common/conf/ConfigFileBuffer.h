/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONFIG_FILE_BUFFER_H_
#define _CONFIG_FILE_BUFFER_H_

#include "conf/ConfigSection.h"
#include "ConfigBuffer.h"

class ConfigFileBuffer : public ConfigBuffer
{
public:
    ConfigFileBuffer(IN const AString& strLocator_, IN const AString& strName_);
    virtual ~ConfigFileBuffer();

private:
    ConfigFileBuffer();

public:
    static IConfigBuffer* CreateFileBuffer(IN const AString& strConfigData);

private:
    // IConfigBuffer interface
    virtual void Destroy();

    virtual IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName);
    virtual IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName, IN IMS_SINT32 nIndex);
    virtual void ReleaseSection();

    virtual IMS_SINT32 ReadKeyCount(IN const IMS_CHAR* pszKey) const;
    virtual const AString& ReadValue(IN const IMS_CHAR* pszKey) const;
    virtual const AString& ReadValue(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex) const;
    virtual IMS_BOOL ReadValueBoolean(IN const IMS_CHAR* pszKey) const;
    virtual IMS_SINT32 ReadValueInt(IN const IMS_CHAR* pszKey) const;

    virtual IMS_BOOL WriteKeyCount(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nCount);
    virtual IMS_BOOL WriteValue(IN const IMS_CHAR* pszKey, IN const AString& strValue);
    virtual IMS_BOOL WriteValue(
            IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex, IN const AString& strValue);
    virtual IMS_BOOL WriteValueBoolean(IN const IMS_CHAR* pszKey, IN IMS_BOOL bValue);
    virtual IMS_BOOL WriteValueInt(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue);

    virtual IMS_BOOL WriteToMedium() const;

    // ConfigBuffer class
    virtual IMS_BOOL Create(IN IMS_SINT32 nId);

    void FormConfig(OUT AString& strConfigData) const;
    IMS_BOOL ParseConfig(IN const AString& strConfigData);
    AString ResolveLocator() const;

private:
    static const IMS_CHAR FILE_EXTENSION[];

    IMS_UINT32 nIndexOfWorkSection;
    ConfigSection* pWorkSection;

    IMSList<ConfigSection*> objSections;
    ConfigComment objStartComment;
    ConfigComment objEndComment;
};

#endif  // _CONFIG_FILE_BUFFER_H_
