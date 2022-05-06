/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_OBSERVER_H_
#define _REG_OBSERVER_H_

#include "IMSTypeDef.h"

class RegObserver
{
public:
    inline RegObserver() {}

    inline virtual ~RegObserver() {}

public:
    virtual void Update(IN IMS_SINT32 nWhat) = 0;
};

#endif  // _REG_OBSERVER_H_
