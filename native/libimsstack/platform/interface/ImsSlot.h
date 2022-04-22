/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170516  hwangoo.park@             Created
    </table>

    Description
    This class is a base class to support a multiple SIM architecture.
*/

#ifndef _IMS_SLOT_H_
#define _IMS_SLOT_H_

class IMSSlot
{
public:
    inline IMSSlot(IN IMS_SINT32 nSlotId_ = IMS_SLOT_0)
        : nSlotId(nSlotId_)
    {}
    inline IMSSlot(IN const IMSSlot &objRHS)
        : nSlotId(objRHS.nSlotId)
    {}
    inline virtual ~IMSSlot()
    {}

public:
    inline IMSSlot& operator=(IN const IMSSlot &objRHS)
    {
        if (this != &objRHS)
        {
            nSlotId = objRHS.nSlotId;
        }

        return *this;
    }

public:
    inline IMS_SINT32 GetSlotId() const
    { return nSlotId; }

private:
    IMS_SINT32 nSlotId;
};

#endif // _IMS_SLOT_H_
