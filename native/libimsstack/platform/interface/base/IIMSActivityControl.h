/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100324  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IIMS_ACTIVITYCONTROL_H_
#define _IIMS_ACTIVITYCONTROL_H_

class IIMSActivityControl
{
public:
    virtual IMS_BOOL Control(
            IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam, OUT IMS_UINTP* pnOutParam) = 0;
};

#endif  // _IIMS_ACTIVITYCONTROL_H_
