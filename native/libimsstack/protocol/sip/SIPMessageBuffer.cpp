/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100409  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "SystemConfig.h"
#include "SIPMessageBuffer.h"

PUBLIC
SIPMessageBuffer::SIPMessageBuffer()
    : RCObject()
    , ppBuffer(IMS_NULL)
{
    IMS_MEM_Memset(MESSAGE, 0x00, MAX_MSG_SIZE);

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        ppBuffer = new IMS_BYTE*[nSimCount];

        ppBuffer[0] = &(MESSAGE[0]);

        for (IMS_SINT32 i = 1; i < nSimCount; ++i)
        {
            ppBuffer[i] = new IMS_BYTE[MAX_MSG_SIZE];
            IMS_MEM_Memset(ppBuffer[i], 0x00, MAX_MSG_SIZE);
        }
    }
}

PUBLIC
SIPMessageBuffer::SIPMessageBuffer(IN const SIPMessageBuffer& objRHS)
    : RCObject(objRHS)
    , ppBuffer(IMS_NULL)
{
    IMS_MEM_Memset(MESSAGE, 0x00, MAX_MSG_SIZE);

    // NOTE: If reference count is not used, you MUST implement this copy constructor
}

PUBLIC VIRTUAL
SIPMessageBuffer::~SIPMessageBuffer()
{
    if (ppBuffer != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (ppBuffer[i] != IMS_NULL)
            {
                if (ppBuffer[i] != &(MESSAGE[0]))
                {
                    delete[] ppBuffer[i];
                }
            }
        }

        delete[] ppBuffer;
    }
}

/*
 Returns a message buffer to form a SIP message (serialization)

Remarks

*/
PUBLIC
IMS_BYTE* SIPMessageBuffer::GetBuffer()
{
    //---------------------------------------------------------------------------------------------

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        return GetBuffer(ThreadService::GetCurrentSlotId(IMS_SLOT_0));
    }

    return &(MESSAGE[0]);
}

/*
 Returns a message buffer to form a SIP message (serialization) for given slot-id.

Remarks

*/
PUBLIC
IMS_BYTE* SIPMessageBuffer::GetBuffer(IN IMS_SINT32 nSlotId)
{
    //---------------------------------------------------------------------------------------------

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        if (ppBuffer != IMS_NULL)
        {
            if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
            {
                nSlotId = IMS_SLOT_0;
            }

            if (ppBuffer[nSlotId] != IMS_NULL)
            {
                return ppBuffer[nSlotId];
            }
        }
    }

    return &(MESSAGE[0]);
}


/*
 Returns a message buffer to form a SIP message (serialization)

Remarks

*/
PUBLIC GLOBAL
RCPtr<SIPMessageBuffer> SIPMessageBuffer::GetInstance()
{
    static SIPMessageBuffer* pBuffer = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pBuffer == IMS_NULL)
    {
        pBuffer = new SIPMessageBuffer();

        pBuffer->AddReference();
    }

    return pBuffer;
}
