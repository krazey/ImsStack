/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_SUBJECT_H_
#define _REG_SUBJECT_H_

#include "IMSList.h"
#include "RegObserver.h"

class RegSubject
{
public:
    RegSubject();
    virtual ~RegSubject();

public:
    virtual void RegisterObserver(IN RegObserver* pObserver);
    virtual void RemoveObserver(IN RegObserver* pObserver);

protected:
    virtual void NotifyObservers(IN IMS_SINT32 nWhat);

private:
    IMSList<RegObserver*> objObservers;
};

#endif  // _REG_SUBJECT_H_
