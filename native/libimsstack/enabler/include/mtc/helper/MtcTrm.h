/*
 * author : hyunbin.shin@
 * version : 2.0
 * date : 201805
 * brief :
 */

#ifndef UC_TRM_H_
#define UC_TRM_H_

#include "IMSActivityEx.h"
#include "IMSList.h"
#include "ITrm.h"

#include "define/MtcInternalMsgDef.h"
#include "define/MtcStringDef.h"

#include "IMtcCallStateListener.h"

class IMutex;

class UCTRM : public IMSActivityEx
{
public:
    UCTRM();
    virtual ~UCTRM();

private:
    UCTRM(IN CONST UCTRM& objRHS);
    UCTRM& operator=(IN CONST UCTRM& objRHS);

public:
    static UCTRM* GetInstance();

    class CT_Handler : public IMSActivityEx
    {
    public:
        CT_Handler(IN IMS_SINT32 nSlotID);
        virtual ~CT_Handler();

    public:
        virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

    private:
        void handleCallChangedState(IN IMSMSG& objMSG);
        void handleCallChangedTotalState(IN IMSMSG& objMSG);
        void handleCallStateIdle(IN IMSMSG& objMSG);
        void handleCallStateOffhook(IN IMSMSG& objMSG);
        void handleCallStateRingback(IN IMSMSG& objMSG);
        void handleCallStateRinging(IN IMSMSG& objMSG);
        void handleCallStateAlerting(IN IMSMSG& objMSG);
        void handleCallStateTerminating(IN IMSMSG& objMSG);
        void handleCallTotalStateIdle(IN IMSMSG& objMSG);
        void handleCallTotalStateOffhook(IN IMSMSG& objMSG);
        void handleCallTotalStateRingback(IN IMSMSG& objMSG);
        void handleCallTotalStateRinging(IN IMSMSG& objMSG);
        void handleCallTotalStateAlerting(IN IMSMSG& objMSG);
        void handleCallTotalStateTerminating(IN IMSMSG& objMSG);

    public:
        IMS_SINT32 m_nSlotID;
    };

    class TRM
    {
    public:
        TRM(IN IMS_SINT32 nSlotID);
        virtual ~TRM();

    public:
        IMS_BOOL SetNTRM(IN IMS_BOOL bSet);
        IMS_BOOL SetETRM(IN IMS_BOOL bSet);

    public:
        IMS_SINT32 m_nSlotID;

        ITrm* m_pITRM;

        IMS_BOOL m_bNSet;
        IMS_BOOL m_bESet;
    };

public:
    void Init(IN IMS_SINT32 nSlotID);
    void DeInit(IN IMS_SINT32 nSlotID);

    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

    void SetTRM(IN IMS_SINT32 nSlotID, IN IMS_UINT32 eType, IN IMS_BOOL bSet);
    IMS_BOOL IsTRM(IN IMS_SINT32 nSlotID, IN IMS_UINT32 eType);

protected:
private:
    void openCT_Handler(IN IMS_SINT32 nSlotID);
    void closeCT_Handler(IN IMS_SINT32 nSlotID);
    UCTRM::CT_Handler* getCT_Handler(IN IMS_SINT32 nSlotID);
    void openTRM(IN IMS_SINT32 nSlotID);
    void closeTRM(IN IMS_SINT32 nSlotID);
    UCTRM::TRM* getTRM(IN IMS_SINT32 nSlotID);

public:
    enum
    {
        TRM_BASE_DEFAULT = MTC_INTERNAL_MSG::TRM_MSG_BASE,
        TRM_CHANGED_STATE,
        TRM_CHANGED_TOTALSTATE,
    };

    enum
    {
        TRM_TYPE_NORMAL_CALL = 0,
        TRM_TYPE_EMERGENCY_CALL = 1,
    };

public:
protected:
private:
    IMutex* m_pIMutex;
    IMSList<CT_Handler*> m_lstCT_Handler;
    IMSList<TRM*> m_lstTRM;
};

#endif  // UC_TRM_H_
