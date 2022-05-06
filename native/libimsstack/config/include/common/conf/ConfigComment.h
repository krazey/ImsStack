/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONFIG_COMMENT_H_
#define _CONFIG_COMMENT_H_

#include "AStringArray.h"

class ConfigComment
{
public:
    ConfigComment();
    ~ConfigComment();

private:
    ConfigComment(IN const ConfigComment& objRHS);
    ConfigComment& operator=(IN const ConfigComment& objRHS);

public:
    void Add(IN const AString& strComment);
    AString ToString() const;

private:
    AStringArray objComments;
};

#endif  // _CONFIG_COMMENT_H_
