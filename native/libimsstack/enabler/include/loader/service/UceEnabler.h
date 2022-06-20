/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    </table>

    Description

*/

#ifndef _UCE_ENABLER_H_
#define _UCE_ENABLER_H_

#include "Enabler.h"

class ImsApp;

class UceEnabler
    : public Enabler
{
public:
    explicit UceEnabler(IN IMS_SINT32 nSlotId);
    virtual ~UceEnabler();

public:
    // IEnabler class
    virtual void Start();
    virtual void Stop();

private:
    ImsApp* pApp;
};

#endif // _UCE_ENABLER_H_
