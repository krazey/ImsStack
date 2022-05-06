/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090319  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "Connection.h"

PUBLIC
Connection::Connection() :
        EngineActivity(),
        piOwner(IMS_NULL)
{
}

PUBLIC VIRTUAL Connection::~Connection() {}

/*

Remarks

*/
PUBLIC VIRTUAL void Connection::Close()
{
    //---------------------------------------------------------------------------------------------

    piOwner = IMS_NULL;
    PostMessage(AMSG_DESTROY, 0, 0);
}

/*

Remarks

*/
PUBLIC
IConnection* Connection::GetOwner() const
{
    //---------------------------------------------------------------------------------------------

    return piOwner;
}

/*

Remarks

*/
PUBLIC
void Connection::SetOwner(IN IConnection* piOwner)
{
    //---------------------------------------------------------------------------------------------

    this->piOwner = piOwner;
}
