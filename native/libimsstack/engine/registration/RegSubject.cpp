/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "RegSubject.h"

PUBLIC
RegSubject::RegSubject() {}

PUBLIC VIRTUAL RegSubject::~RegSubject() {}

PUBLIC VIRTUAL void RegSubject::RegisterObserver(IN RegObserver* pObserver)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver* pTmpObserver = objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
            return;
    }

    objObservers.Append(pObserver);
}

PUBLIC VIRTUAL void RegSubject::RemoveObserver(IN RegObserver* pObserver)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver* pTmpObserver = objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            objObservers.RemoveAt(i);
            return;
        }
    }
}

PROTECTED VIRTUAL void RegSubject::NotifyObservers(IN IMS_SINT32 nWhat)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver* pObserver = objObservers.GetAt(i);

        if (pObserver == IMS_NULL)
            continue;

        pObserver->Update(nWhat);
    }
}
