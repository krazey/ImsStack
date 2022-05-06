#ifndef INTERFACE_COMMENT_H_
#define INTERFACE_COMMENT_H_

/**
 * @brief This class represents a comment.
 *
 * This interface inherits from CharacterData and represents the content of a comment,
 * i.e., all the characters between the starting ' <!--' and ending '-->'.
 * Note that this is the definition of a comment in XML, and, in practice, HTML,
 * although some HTML tools may implement the full SGML comment structure.
 *
 * @see ICharacterData, INode
 */
class IComment : public ICharacterData, public INode
{
public:
};

#endif
