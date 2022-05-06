/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONFIG_SECTION_DATA_H_
#define _CONFIG_SECTION_DATA_H_

#include "conf/ConfigComment.h"

class ConfigSectionData
{
public:
    explicit ConfigSectionData(IN const AString& strKey_);
    ConfigSectionData(IN const AString& strKey_, IN const AString& strValue_);
    ~ConfigSectionData();

private:
    ConfigSectionData();
    ConfigSectionData(IN const ConfigSectionData& objRHS);
    ConfigSectionData& operator=(IN const ConfigSectionData& objRHS);

public:
    void AddComment(IN const AString& strComment);
    const AString& GetKey() const;
    const AString& GetValue() const;
    void SetValue(IN const AString& strValue);
    AString ToString() const;

private:
    ConfigComment objComment;

    AString strKey;
    AString strValue;
};

#endif  // _CONFIG_SECTION_DATA_H_
