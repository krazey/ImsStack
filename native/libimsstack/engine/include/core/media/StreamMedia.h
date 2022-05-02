/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#ifndef _STREAM_MEDIA_H_
#define _STREAM_MEDIA_H_

#include "media/Media.h"

class StreamMediaImpl;



class StreamMedia
    : public Media
{
public:
    StreamMedia(IN Service *pService_, IN ISDPOAState *piOAState_);
    virtual ~StreamMedia();

private:
    StreamMedia(IN CONST StreamMedia &objRHS);
    StreamMedia& operator=(IN CONST StreamMedia &objRHS);

public:
    // IMedia interface
    virtual IMS_SINT32 GetType() const;

protected:
    virtual MediaProposal* CreateMediaProposal(IN ISDPOAState *piOAState);
};

#endif // _STREAM_MEDIA_H_
