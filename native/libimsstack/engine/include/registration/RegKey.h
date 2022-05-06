/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170628  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_KEY_H_
#define _REG_KEY_H_

#include "IMSTypeDef.h"

class RegKey
{
public:
    inline RegKey() :
            nSlotId(IMS_SLOT_ANY),
            nFlowId(0)
    {
    }
    inline RegKey(IN IMS_SINT32 nSlotId_, IN IMS_UINT32 nFlowId_) :
            nSlotId(nSlotId_),
            nFlowId(nFlowId_)
    {
    }
    inline RegKey(IN const RegKey& objRHS) :
            nSlotId(objRHS.nSlotId),
            nFlowId(objRHS.nFlowId)
    {
    }
    inline ~RegKey() {}

public:
    inline RegKey& operator=(IN const RegKey& objRHS)
    {
        if (this != &objRHS)
        {
            nFlowId = objRHS.nFlowId;
            nSlotId = objRHS.nSlotId;
        }

        return (*this);
    }

    // For IMSMap class
    inline IMS_BOOL operator<(IN const RegKey& objRHS)
    {
        return GetHashCode() < objRHS.GetHashCode();
    }

public:
    inline IMS_BOOL Equals(IN const RegKey& objRHS) const
    {
        return (nSlotId == objRHS.nSlotId) && (nFlowId == objRHS.nFlowId);
    }
    inline IMS_UINT32 GetHashCode() const { return (nSlotId * 997) + (nFlowId * 13); }

    inline IMS_UINT32 GetFlowId() const { return nFlowId; }
    inline IMS_SINT32 GetSlotId() const { return nSlotId; }

    inline void Invalidate()
    {
        nSlotId = IMS_SLOT_ANY;
        nFlowId = 0;
    }

private:
    IMS_SINT32 nSlotId;
    IMS_UINT32 nFlowId;
};

inline IMS_BOOL operator==(IN const RegKey& objK1, IN const RegKey& objK2)
{
    return objK1.Equals(objK2);
}

inline IMS_BOOL operator!=(IN const RegKey& objK1, IN const RegKey& objK2)
{
    return !objK1.Equals(objK2);
}

// For IMSMap class
inline IMS_BOOL operator<(IN const RegKey& objK1, IN const RegKey& objK2)
{
    return objK1.GetHashCode() < objK2.GetHashCode();
}

#endif  // _REG_KEY_H_
