/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110608  dongo.yi@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_CRLF_TRANSACTION_H_
#define _INTERFACE_CRLF_TRANSACTION_H_

#include "IMSTypeDef.h"

class ICRLFTransactionListener
{
public:
    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>
    */
    virtual void SIPTransaction_Notify() = 0;
};

#endif  // _INTERFACE_CRLF_TRANSACTION_H_
