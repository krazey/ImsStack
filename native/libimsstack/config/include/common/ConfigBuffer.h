/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONFIG_BUFFER_H_
#define _CONFIG_BUFFER_H_

#include "IConfigBuffer.h"

class ConfigBuffer : public IConfigBuffer
{
public:
    ConfigBuffer(IN const AString& strLocator_, IN const AString& strName_);
    virtual ~ConfigBuffer();

private:
    ConfigBuffer(IN const ConfigBuffer& objRHS);
    ConfigBuffer& operator=(IN const ConfigBuffer& objRHS);

public:
    virtual IMS_BOOL Create(IN IMS_SINT32 nId) = 0;

protected:
    const AString& GetLocator() const;
    const AString& GetName() const;

private:
    AString strLocator;
    AString strName;
};

#endif  // _CONFIG_BUFFER_H_
