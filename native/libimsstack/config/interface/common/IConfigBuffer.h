/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_CONFIG_BUFFER_H_
#define _INTERFACE_CONFIG_BUFFER_H_

#include "AStringArray.h"

class IConfigBuffer
{
public:
    /*
     Destroys the configuration buffer.
    This interface can be instantiated by Configuration interface for IMS enablers.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void Destroy() = 0;

    /*
     Captures the current work section to read/write the item value.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszSectName             Section string to be worked
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName) = 0;

    /*
     Captures the current work section which has a property of list section
    to read/write the item value.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszSectName             Section string to be worked
    nIndex                  Index of section list
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName, IN IMS_SINT32 nIndex) = 0;

    /*
     Releases the current work section.
    After invoking this method, READ/WRITE operation will be failed.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void ReleaseSection() = 0;

    /*
     Reads the count of the specified key with 'count' property.
    For example,
        abc_count=3
        -> IMS_SINT32 nCount = piBuffer->ReadKeyCount("abc");
        -> nCount will be 3.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              Count value of the specified key
    </table>
    */
    virtual IMS_SINT32 ReadKeyCount(IN const IMS_CHAR* pszKey) const = 0;

    /*
     Reads the string value of the specified key with 'string' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Value of the specified key
    </table>
    */
    virtual const AString& ReadValue(IN const IMS_CHAR* pszKey) const = 0;

    /*
     Reads the string value of the specified key with 'string' property & 'list' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    nIndex                  Index of key list
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Value of the specified key
    </table>
    */
    virtual const AString& ReadValue(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex) const = 0;

    /*
     Reads the boolean value of the specified key with 'boolean' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_BOOL                IMS_TRUE / IMS_FALSE
    </table>
    */
    virtual IMS_BOOL ReadValueBoolean(IN const IMS_CHAR* pszKey) const = 0;

    /*
     Reads the integer value of the specified key with 'integer' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              Value of the specified key
    </table>
    */
    virtual IMS_SINT32 ReadValueInt(IN const IMS_CHAR* pszKey) const = 0;

    /*
     Writes the count of the specified key with 'count' property.
    For example,
        piBuffer->WriteKeyCount("abc", 3);
        -> abc_count=3

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL WriteKeyCount(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nCount) = 0;

    /*
     Writes the string value of the specified key with 'string' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    strValue                Value to be written
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL WriteValue(IN const IMS_CHAR* pszKey, IN const AString& strValue) = 0;

    /*
     Writes the string value of the specified key with 'string' property & 'list' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    nIndex                  Index of key list
    strValue                Value to be written
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL WriteValue(
            IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex, IN const AString& strValue) = 0;

    /*
     Reads the boolean value of the specified key with 'boolean' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    bValue                  Value to be written
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL WriteValueBoolean(IN const IMS_CHAR* pszKey, IN IMS_BOOL bValue) = 0;

    /*
     Writes the integer value of the specified key with 'integer' property.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pszKey                  Key string
    nValue                  Value to be written
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL WriteValueInt(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue) = 0;

    /*
     Writes the all configuration values to the specified medium.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The operation is done successfully
    IMS_FALSE               The operation is failed
    </table>
    */
    virtual IMS_BOOL WriteToMedium() const = 0;
};

#endif  // _INTERFACE_CONFIG_BUFFER_H_
