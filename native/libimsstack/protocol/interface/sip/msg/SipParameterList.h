/***********************************************************
 * Project Name : SIP_RTP
 * Group        : MSG-2
 * Security     : Confidential
 ***********************************************************/

/**********************************************************
 * Filename          : SipParameterList.h
 * Purpose           :
 * Platform          : Windows XP
 * Author(s)         : Saurabh Srivastava
 * E-mail id.        : saurabh31.srivastava@
 * Creation date     : July 26, 2010
 *
 * Modifications:
 * 1. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0a
 *
 * 2. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0b
 **********************************************************/
#ifndef __SIP_PARAMETER_LIST_H__
#define __SIP_PARAMETER_LIST_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "SipRefBase.h"
#include "msg/sip_comdef.h"
#include "SipPercentEncoding.h"
#include "msg/IParameterComponent.h"
#include "SipVector.h"
/****************************************************************************
  Macro Definitions
 *****************************************************************************/


/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/

class SipNameValue : public SipRefBase
{
    public:
        SIP_CHAR* m_pszName;
        SipVector<SIP_CHAR *> m_valueList;
        SIP_INT32 m_ePrmType;
        SIP_CHAR m_Sep;
        SIP_INT32 m_eHdrType;


        SipNameValue();
        SipNameValue(SIP_INT32 eHdrType);
        SipNameValue(const SipNameValue& objNmVl);
        virtual ~SipNameValue();
        SIP_BOOL DecUriNameVal(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
                IParameterComponent* pParameterComponent = SIP_NULL);


        SIP_BOOL DecUriHdrNameVal(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
                IParameterComponent* pParameterComponent = SIP_NULL);

        SIP_BOOL DecHdrNameVal(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

        SIP_BOOL SetSeparator(SIP_CHAR cSeparator);

        SIP_BOOL EncodeFromUriList(SIP_CHAR** ppCurrPos,
                IParameterComponent* pParameterComponent = SIP_NULL);

        SIP_BOOL EncodeFromUriHdrList(SIP_CHAR** ppCurrPos,
                IParameterComponent* pParameterComponent = SIP_NULL);

        SIP_BOOL EncodeFromList(SIP_CHAR** ppCurrPos);
};

class SipParameterList: public SipRefBase
{
    private:
        SipVector<SipNameValue*> m_objPrmList;
        SIP_INT32 m_eHdrType;

    public:

        SipParameterList();
        SipParameterList(SIP_INT32 eHdrType);
        SipParameterList(const SipParameterList& objPrmList);
        ~SipParameterList();
        /*Add only name*/
        SIP_BOOL Add(const SIP_CHAR* pszName);

        SIP_BOOL Add(const SIP_CHAR* pszName, const SIP_CHAR* pszValue);

        SIP_BOOL Add(const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_INT32 ePrmType);

        SIP_BOOL EncodeList(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter);

        SIP_BOOL EncodeUriParamList(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter,
                IParameterComponent* pParameterComponent = SIP_NULL);

        SIP_BOOL DecUriSipParameterList(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
                SIP_CHAR cDelimiter, IParameterComponent* pParameterComponent = SIP_NULL);

        SIP_BOOL DecUriHdrSipParameterList(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
                SIP_CHAR cDelimiter, IParameterComponent* pParameterComponent = SIP_NULL);

        SIP_BOOL Remove(const SIP_CHAR* pszName);

        SIP_BOOL DecHdrSipParameterList(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
                SIP_CHAR cDelimiter);

        inline SipVector<SipNameValue*>& GetList()
        {
            return m_objPrmList;
        }

        inline SIP_UINT32 GetCount() const
        {
            return m_objPrmList.GetSize();
        }

        SIP_BOOL IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pPos);

        inline SipNameValue* GetNameValNode(SIP_UINT32 iIndex) const
        {
            return (iIndex < m_objPrmList.GetSize()) ? m_objPrmList.GetAt(iIndex) : SIP_NULL;
        }

        SIP_BOOL FindElement(const SIP_CHAR* pszName, SipNameValue *&pNmvl, SIP_UINT32& nPos);
        SIP_BOOL SetParamValue(const SIP_CHAR* pszName, const SIP_CHAR* pszValue,
                SIP_UINT32 nPos = SIP_ZERO);
        SIP_CHAR* GetParamValue(const SIP_CHAR* pszName, SIP_UINT32 nPos = SIP_ZERO);
        SipNameValue* GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos);

        /* To Print all the header params & its values */
        SIP_VOID PrintParams();
};

class SipParameters
{
    public:
        /*Enumeration for Prm Type*/
        enum
        {
            GENERIC,
            FEATURE,
            INVALID = SIP_INVALID
        };

        SipParameterList* pParameterList;

        SipParameters();
        SipParameters(SipParameterList* pParameterList_);
        SipParameters(const SipParameters& objParameters);
        ~SipParameters();

        SIP_BOOL AddParam(const SIP_CHAR* pszName);

        SIP_BOOL AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue);

        SIP_BOOL IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pnPos);

        SIP_BOOL AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_INT32 ePrmType);

        SIP_BOOL RemoveParam(const SIP_CHAR* pszName);

        SIP_VOID SetParameterList(SipParameterList* pSipPrm);

        inline SIP_UINT32 GetParamCount() const
        {
            return (pParameterList != SIP_NULL) ? pParameterList->GetCount() : SIP_ZERO;
        }
        SipParameterList* GetParameterList();
        SIP_CHAR* GetParamValue(const SIP_CHAR* pszName);

        SIP_BOOL SetParamValue(const SIP_CHAR* pszName, const SIP_CHAR* pszValue,
                SIP_UINT32 nPos = SIP_ZERO);
        SipNameValue* GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos);
};

#endif //__SIP_PARAMETER_LIST_H__
