/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONFIG_SECTION_H_
#define _CONFIG_SECTION_H_

#include "conf/ConfigComment.h"
#include "conf/ConfigSectionData.h"

class ConfigSection
{
public:
    ConfigSection();
    ~ConfigSection();

private:
    ConfigSection(IN const ConfigSection& objRHS);
    ConfigSection& operator=(IN const ConfigSection& objRHS);

public:
    void AddComment(IN const AString& strComment);
    const AString& GetName() const;
    void GetKeys(OUT AStringArray& objKeys) const;
    const AString& GetValue(IN const IMS_CHAR* pszKey) const;
    IMS_BOOL SetValue(IN const IMS_CHAR* pszKey, IN const AString& strValue);
    AString ToString() const;

private:
    IMS_BOOL AddSectionData(IN const AString& strKeyValue);
    ConfigSectionData* GetLastElement() const;
    void SetName(IN const AString& strSectName);

private:
    friend class ConfigFileBuffer;

    ConfigComment objComment;

    AString strSectionName;
    IMSList<ConfigSectionData*> objSectionData;
};

#endif  // _CONFIG_SECTION_H_
