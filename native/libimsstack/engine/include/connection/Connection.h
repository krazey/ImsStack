/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090319  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "EngineActivity.h"

class IConnection;

class Connection : public EngineActivity
{
public:
    Connection();
    virtual ~Connection();

public:
    // IConnection interface
    virtual void Close();

    // Extensions
    IConnection* GetOwner() const;
    void SetOwner(IN IConnection* piOwner);

private:
    IConnection* piOwner;
};

#endif  // _CONNECTION_H_
