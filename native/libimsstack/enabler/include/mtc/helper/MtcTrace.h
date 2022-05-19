/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20190916  dongo.yi@                 Created
    </table>

    Description

*/

#ifndef UC_TRACE_H
#define UC_TRACE_H

#include "ServiceTrace.h"

#undef IMS_TRACE_D
#undef IMS_TRACE_I
#undef IMS_TRACE_E

#define IMS_TRACE_D(FORMAT, A1, A2, A3)        U_IMS_TRACE_D(UCTAG, FORMAT, A1, A2, A3)
#define IMS_TRACE_I(FORMAT, A1, A2, A3)        U_IMS_TRACE_I(UCTAG, FORMAT, A1, A2, A3)
#define IMS_TRACE_E(ECODE, FORMAT, A1, A2, A3) U_IMS_TRACE_E(ECODE, UCTAG, FORMAT, A1, A2, A3)
#endif  // UC_TRACE_H
