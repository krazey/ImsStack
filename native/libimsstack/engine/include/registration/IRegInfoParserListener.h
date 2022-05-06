/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_REG_INFO_PARSER_LISTENER_H_
#define _INTERFACE_REG_INFO_PARSER_LISTENER_H_

class IDocument;
class RegInfoParser;

class IRegInfoParserListener
{
public:
    /*

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
    virtual void RegInfoParser_ParsingCompleted(
            IN RegInfoParser* pParser, IN IDocument* piDocument) = 0;

    /*

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
    virtual void RegInfoParser_ParsingFailed(IN RegInfoParser* pParser) = 0;
};

#endif  // _INTERFACE_REG_INFO_PARSER_LISTENER_H_
