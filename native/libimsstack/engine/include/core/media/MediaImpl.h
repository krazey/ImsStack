/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100512  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _MEDIA_IMPL_H_
#define _MEDIA_IMPL_H_

#include "media/IMedia.h"

class Media;

class MediaImpl
{
public:
    inline MediaImpl() {}

    inline virtual ~MediaImpl() {}

public:
    virtual IMS_BOOL Equals(IN CONST IMedia* piMedia) const = 0;
    virtual IMedia* GetInterface() = 0;
    virtual Media* GetMedia() const = 0;
};

#endif  // _MEDIA_IMPL_H_
