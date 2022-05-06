/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091026  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_MEDIA_CONFIG_H_
#define _INTERFACE_MEDIA_CONFIG_H_

#include "AStringArray.h"

class IMediaConfig
{
public:
    /*

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
    virtual const AStringArray& GetMediaCapabilities(IN IMS_SINT32 nMediaType) const = 0;

    /*

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
    virtual const AStringArray& GetMediaProfile(
            IN CONST AString& strName, IN IMS_SINT32 nMediaType) const = 0;

public:
    enum
    {
        MEDIA_INVALID = 0,
        STREAM_AUDIO = 1,
        STREAM_VIDEO = 2,
        FRAMED = 3,
        BASIC_UNRELIABLE = 4,
        BASIC_RELIABLE = 5
    };
};

#endif  // _INTERFACE_MEDIA_CONFIG_H_
