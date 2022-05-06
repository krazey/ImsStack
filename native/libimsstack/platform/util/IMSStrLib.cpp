/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20060102  yhrhee@                   Initial creation
    20070711  JHS                       move file and modify function name
    20080102  yhrhee@                   LH2000 Adaptation
    20090302  bluable@                  Porting
    </table>

Description
String      */

#include <string.h>
#include <stdio.h>

#include "ServiceMemory.h"
#include "IMSLib.h"
#include "IMSStrLib.h"

#define SU_ZEROPAD 1  /* pad with zero */
#define SU_SIGN 2     /* unsigned/signed long */
#define SU_PLUS 4     /* show plus */
#define SU_SPACE 8    /* space if plus */
#define SU_LEFT 16    /* left justified */
#define SU_SPECIAL 32 /* 0x */
#define SU_LARGE 64   /* use 'ABCDEF' instead of 'abcdef' */
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

LOCAL
const IMS_WCHAR stLowerDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c',
        'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u',
        'v', 'w', 'x', 'y', 'z', 0};
LOCAL
const IMS_WCHAR stUpperDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
        'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
        'V', 'W', 'X', 'Y', 'Z', 0};

// Table for converting WanSung to JoHap
LOCAL
const IMS_WCHAR stJohapTable[2350] = {0x8861, 0x8862, 0x8865, 0x8868, 0x8869, 0x886A, 0x886B,
        0x8871, 0x8873, 0x8874, 0x8875, 0x8876, 0x8877, 0x8878, 0x8879, 0x887B, 0x887C, 0x887D,
        0x8881, 0x8882, 0x8885, 0x8889, 0x8891, 0x8893, 0x8895, 0x8896, 0x8897, 0x88A1, 0x88A2,
        0x88A5, 0x88A9, 0x88B5, 0x88B7, 0x88C1, 0x88C5, 0x88C9, 0x88E1, 0x88E2, 0x88E5, 0x88E8,
        0x88E9, 0x88EB, 0x88F1, 0x88F3, 0x88F5, 0x88F6, 0x88F7, 0x88F8, 0x88FB, 0x88FC, 0x88FD,
        0x8941, 0x8945, 0x8949, 0x8951, 0x8953, 0x8955, 0x8956, 0x8957, 0x8961, 0x8962, 0x8963,
        0x8965, 0x8968, 0x8969, 0x8971, 0x8973, 0x8975, 0x8976, 0x8977, 0x897B, 0x8981, 0x8985,
        0x8989, 0x8993, 0x8995, 0x89A1, 0x89A2, 0x89A5, 0x89A8, 0x89A9, 0x89AB, 0x89AD, 0x89B0,
        0x89B1, 0x89B3, 0x89B5, 0x89B7, 0x89B8, 0x89C1, 0x89C2, 0x89C5, 0x89C9, 0x89CB,

        0x89D1, 0x89D3, 0x89D5, 0x89D7, 0x89E1, 0x89E5, 0x89E9, 0x89F3, 0x89F6, 0x89F7, 0x8A41,
        0x8A42, 0x8A45, 0x8A49, 0x8A51, 0x8A53, 0x8A55, 0x8A57, 0x8A61, 0x8A65, 0x8A69, 0x8A73,
        0x8A75, 0x8A81, 0x8A82, 0x8A85, 0x8A88, 0x8A89, 0x8A8A, 0x8A8B, 0x8A90, 0x8A91, 0x8A93,
        0x8A95, 0x8A97, 0x8A98, 0x8AA1, 0x8AA2, 0x8AA5, 0x8AA9, 0x8AB6, 0x8AB7, 0x8AC1, 0x8AD5,
        0x8AE1, 0x8AE2, 0x8AE5, 0x8AE9, 0x8AF1, 0x8AF3, 0x8AF5, 0x8B41, 0x8B45, 0x8B49, 0x8B61,
        0x8B62, 0x8B65, 0x8B68, 0x8B69, 0x8B6A, 0x8B71, 0x8B73, 0x8B75, 0x8B77, 0x8B81, 0x8BA1,
        0x8BA2, 0x8BA5, 0x8BA8, 0x8BA9, 0x8BAB, 0x8BB1, 0x8BB3, 0x8BB5, 0x8BB7, 0x8BB8, 0x8BBC,
        0x8C61, 0x8C62, 0x8C63, 0x8C65, 0x8C69, 0x8C6B, 0x8C71, 0x8C73, 0x8C75, 0x8C76, 0x8C77,
        0x8C7B, 0x8C81, 0x8C82, 0x8C85, 0x8C89, 0x8C91,

        0x8C93, 0x8C95, 0x8C96, 0x8C97, 0x8CA1, 0x8CA2, 0x8CA9, 0x8CE1, 0x8CE2, 0x8CE3, 0x8CE5,
        0x8CE9, 0x8CF1, 0x8CF3, 0x8CF5, 0x8CF6, 0x8CF7, 0x8D41, 0x8D42, 0x8D45, 0x8D51, 0x8D55,
        0x8D57, 0x8D61, 0x8D65, 0x8D69, 0x8D75, 0x8D76, 0x8D7B, 0x8D81, 0x8DA1, 0x8DA2, 0x8DA5,
        0x8DA7, 0x8DA9, 0x8DB1, 0x8DB3, 0x8DB5, 0x8DB7, 0x8DB8, 0x8DB9, 0x8DC1, 0x8DC2, 0x8DC9,
        0x8DD6, 0x8DD7, 0x8DE1, 0x8DE2, 0x8DF7, 0x8E41, 0x8E45, 0x8E49, 0x8E51, 0x8E53, 0x8E57,
        0x8E61, 0x8E81, 0x8E82, 0x8E85, 0x8E89, 0x8E90, 0x8E91, 0x8E93, 0x8E95, 0x8E97, 0x8E98,
        0x8EA1, 0x8EA9, 0x8EB6, 0x8EB7, 0x8EC1, 0x8EC2, 0x8EC5, 0x8EC9, 0x8ED1, 0x8ED3, 0x8ED6,
        0x8EE1, 0x8EE5, 0x8EE9, 0x8EF1, 0x8EF3, 0x8F41, 0x8F61, 0x8F62, 0x8F65, 0x8F67, 0x8F69,
        0x8F6B, 0x8F70, 0x8F71, 0x8F73, 0x8F75, 0x8F77,

        0x8F7B, 0x8FA1, 0x8FA2, 0x8FA5, 0x8FA9, 0x8FB1, 0x8FB3, 0x8FB5, 0x8FB7, 0x9061, 0x9062,
        0x9063, 0x9065, 0x9068, 0x9069, 0x906A, 0x906B, 0x9071, 0x9073, 0x9075, 0x9076, 0x9077,
        0x9078, 0x9079, 0x907B, 0x907D, 0x9081, 0x9082, 0x9085, 0x9089, 0x9091, 0x9093, 0x9095,
        0x9096, 0x9097, 0x90A1, 0x90A2, 0x90A5, 0x90A9, 0x90B1, 0x90B7, 0x90E1, 0x90E2, 0x90E4,
        0x90E5, 0x90E9, 0x90EB, 0x90EC, 0x90F1, 0x90F3, 0x90F5, 0x90F6, 0x90F7, 0x90FD, 0x9141,
        0x9142, 0x9145, 0x9149, 0x9151, 0x9153, 0x9155, 0x9156, 0x9157, 0x9161, 0x9162, 0x9165,
        0x9169, 0x9171, 0x9173, 0x9176, 0x9177, 0x917A, 0x9181, 0x9185, 0x91A1, 0x91A2, 0x91A5,
        0x91A9, 0x91AB, 0x91B1, 0x91B3, 0x91B5, 0x91B7, 0x91BC, 0x91BD, 0x91C1, 0x91C5, 0x91C9,
        0x91D6, 0x9241, 0x9245, 0x9249, 0x9251, 0x9253,

        0x9255, 0x9261, 0x9262, 0x9265, 0x9269, 0x9273, 0x9275, 0x9277, 0x9281, 0x9282, 0x9285,
        0x9288, 0x9289, 0x9291, 0x9293, 0x9295, 0x9297, 0x92A1, 0x92B6, 0x92C1, 0x92E1, 0x92E5,
        0x92E9, 0x92F1, 0x92F3, 0x9341, 0x9342, 0x9349, 0x9351, 0x9353, 0x9357, 0x9361, 0x9362,
        0x9365, 0x9369, 0x936A, 0x936B, 0x9371, 0x9373, 0x9375, 0x9377, 0x9378, 0x937C, 0x9381,
        0x9385, 0x9389, 0x93A1, 0x93A2, 0x93A5, 0x93A9, 0x93AB, 0x93B1, 0x93B3, 0x93B5, 0x93B7,
        0x93BC, 0x9461, 0x9462, 0x9463, 0x9465, 0x9468, 0x9469, 0x946A, 0x946B, 0x946C, 0x9470,
        0x9471, 0x9473, 0x9475, 0x9476, 0x9477, 0x9478, 0x9479, 0x947D, 0x9481, 0x9482, 0x9485,
        0x9489, 0x9491, 0x9493, 0x9495, 0x9496, 0x9497, 0x94A1, 0x94E1, 0x94E2, 0x94E3, 0x94E5,
        0x94E8, 0x94E9, 0x94EB, 0x94EC, 0x94F1, 0x94F3,

        0x94F5, 0x94F7, 0x94F9, 0x94FC, 0x9541, 0x9542, 0x9545, 0x9549, 0x9551, 0x9553, 0x9555,
        0x9556, 0x9557, 0x9561, 0x9565, 0x9569, 0x9576, 0x9577, 0x9581, 0x9585, 0x95A1, 0x95A2,
        0x95A5, 0x95A8, 0x95A9, 0x95AB, 0x95AD, 0x95B1, 0x95B3, 0x95B5, 0x95B7, 0x95B9, 0x95BB,
        0x95C1, 0x95C5, 0x95C9, 0x95E1, 0x95F6, 0x9641, 0x9645, 0x9649, 0x9651, 0x9653, 0x9655,
        0x9661, 0x9681, 0x9682, 0x9685, 0x9689, 0x9691, 0x9693, 0x9695, 0x9697, 0x96A1, 0x96B6,
        0x96C1, 0x96D7, 0x96E1, 0x96E5, 0x96E9, 0x96F3, 0x96F5, 0x96F7, 0x9741, 0x9745, 0x9749,
        0x9751, 0x9757, 0x9761, 0x9762, 0x9765, 0x9768, 0x9769, 0x976B, 0x9771, 0x9773, 0x9775,
        0x9777, 0x9781, 0x97A1, 0x97A2, 0x97A5, 0x97A8, 0x97A9, 0x97B1, 0x97B3, 0x97B5, 0x97B6,
        0x97B7, 0x97B8, 0x9861, 0x9862, 0x9865, 0x9869,

        0x9871, 0x9873, 0x9875, 0x9876, 0x9877, 0x987D, 0x9881, 0x9882, 0x9885, 0x9889, 0x9891,
        0x9893, 0x9895, 0x9896, 0x9897, 0x98E1, 0x98E2, 0x98E5, 0x98E9, 0x98EB, 0x98EC, 0x98F1,
        0x98F3, 0x98F5, 0x98F6, 0x98F7, 0x98FD, 0x9941, 0x9942, 0x9945, 0x9949, 0x9951, 0x9953,
        0x9955, 0x9956, 0x9957, 0x9961, 0x9976, 0x99A1, 0x99A2, 0x99A5, 0x99A9, 0x99B7, 0x99C1,
        0x99C9, 0x99E1, 0x9A41, 0x9A45, 0x9A81, 0x9A82, 0x9A85, 0x9A89, 0x9A90, 0x9A91, 0x9A97,
        0x9AC1, 0x9AE1, 0x9AE5, 0x9AE9, 0x9AF1, 0x9AF3, 0x9AF7, 0x9B61, 0x9B62, 0x9B65, 0x9B68,
        0x9B69, 0x9B71, 0x9B73, 0x9B75, 0x9B81, 0x9B85, 0x9B89, 0x9B91, 0x9B93, 0x9BA1, 0x9BA5,
        0x9BA9, 0x9BB1, 0x9BB3, 0x9BB5, 0x9BB7, 0x9C61, 0x9C62, 0x9C65, 0x9C69, 0x9C71, 0x9C73,
        0x9C75, 0x9C76, 0x9C77, 0x9C78, 0x9C7C, 0x9C7D,

        0x9C81, 0x9C82, 0x9C85, 0x9C89, 0x9C91, 0x9C93, 0x9C95, 0x9C96, 0x9C97, 0x9CA1, 0x9CA2,
        0x9CA5, 0x9CB5, 0x9CB7, 0x9CE1, 0x9CE2, 0x9CE5, 0x9CE9, 0x9CF1, 0x9CF3, 0x9CF5, 0x9CF6,
        0x9CF7, 0x9CFD, 0x9D41, 0x9D42, 0x9D45, 0x9D49, 0x9D51, 0x9D53, 0x9D55, 0x9D57, 0x9D61,
        0x9D62, 0x9D65, 0x9D69, 0x9D71, 0x9D73, 0x9D75, 0x9D76, 0x9D77, 0x9D81, 0x9D85, 0x9D93,
        0x9D95, 0x9DA1, 0x9DA2, 0x9DA5, 0x9DA9, 0x9DB1, 0x9DB3, 0x9DB5, 0x9DB7, 0x9DC1, 0x9DC5,
        0x9DD7, 0x9DF6, 0x9E41, 0x9E45, 0x9E49, 0x9E51, 0x9E53, 0x9E55, 0x9E57, 0x9E61, 0x9E65,
        0x9E69, 0x9E73, 0x9E75, 0x9E77, 0x9E81, 0x9E82, 0x9E85, 0x9E89, 0x9E91, 0x9E93, 0x9E95,
        0x9E97, 0x9EA1, 0x9EB6, 0x9EC1, 0x9EE1, 0x9EE2, 0x9EE5, 0x9EE9, 0x9EF1, 0x9EF5, 0x9EF7,
        0x9F41, 0x9F42, 0x9F45, 0x9F49, 0x9F51, 0x9F53,

        0x9F55, 0x9F57, 0x9F61, 0x9F62, 0x9F65, 0x9F69, 0x9F71, 0x9F73, 0x9F75, 0x9F77, 0x9F78,
        0x9F7B, 0x9F7C, 0x9FA1, 0x9FA2, 0x9FA5, 0x9FA9, 0x9FB1, 0x9FB3, 0x9FB5, 0x9FB7, 0xA061,
        0xA062, 0xA065, 0xA067, 0xA068, 0xA069, 0xA06A, 0xA06B, 0xA071, 0xA073, 0xA075, 0xA077,
        0xA078, 0xA07B, 0xA07D, 0xA081, 0xA082, 0xA085, 0xA089, 0xA091, 0xA093, 0xA095, 0xA096,
        0xA097, 0xA098, 0xA0A1, 0xA0A2, 0xA0A9, 0xA0B7, 0xA0E1, 0xA0E2, 0xA0E5, 0xA0E9, 0xA0EB,
        0xA0F1, 0xA0F3, 0xA0F5, 0xA0F7, 0xA0F8, 0xA0FD, 0xA141, 0xA142, 0xA145, 0xA149, 0xA151,
        0xA153, 0xA155, 0xA156, 0xA157, 0xA161, 0xA162, 0xA165, 0xA169, 0xA175, 0xA176, 0xA177,
        0xA179, 0xA181, 0xA1A1, 0xA1A2, 0xA1A4, 0xA1A5, 0xA1A9, 0xA1AB, 0xA1B1, 0xA1B3, 0xA1B5,
        0xA1B7, 0xA1C1, 0xA1C5, 0xA1D6, 0xA1D7, 0xA241,

        0xA245, 0xA249, 0xA253, 0xA255, 0xA257, 0xA261, 0xA265, 0xA269, 0xA273, 0xA275, 0xA281,
        0xA282, 0xA283, 0xA285, 0xA288, 0xA289, 0xA28A, 0xA28B, 0xA291, 0xA293, 0xA295, 0xA297,
        0xA29B, 0xA29D, 0xA2A1, 0xA2A5, 0xA2A9, 0xA2B3, 0xA2B5, 0xA2C1, 0xA2E1, 0xA2E5, 0xA2E9,
        0xA341, 0xA345, 0xA349, 0xA351, 0xA355, 0xA361, 0xA365, 0xA369, 0xA371, 0xA375, 0xA3A1,
        0xA3A2, 0xA3A5, 0xA3A8, 0xA3A9, 0xA3AB, 0xA3B1, 0xA3B3, 0xA3B5, 0xA3B6, 0xA3B7, 0xA3B9,
        0xA3BB, 0xA461, 0xA462, 0xA463, 0xA464, 0xA465, 0xA468, 0xA469, 0xA46A, 0xA46B, 0xA46C,
        0xA471, 0xA473, 0xA475, 0xA477, 0xA47B, 0xA481, 0xA482, 0xA485, 0xA489, 0xA491, 0xA493,
        0xA495, 0xA496, 0xA497, 0xA49B, 0xA4A1, 0xA4A2, 0xA4A5, 0xA4B3, 0xA4E1, 0xA4E2, 0xA4E5,
        0xA4E8, 0xA4E9, 0xA4EB, 0xA4F1, 0xA4F3, 0xA4F5,

        0xA4F7, 0xA4F8, 0xA541, 0xA542, 0xA545, 0xA548, 0xA549, 0xA551, 0xA553, 0xA555, 0xA556,
        0xA557, 0xA561, 0xA562, 0xA565, 0xA569, 0xA573, 0xA575, 0xA576, 0xA577, 0xA57B, 0xA581,
        0xA585, 0xA5A1, 0xA5A2, 0xA5A3, 0xA5A5, 0xA5A9, 0xA5B1, 0xA5B3, 0xA5B5, 0xA5B7, 0xA5C1,
        0xA5C5, 0xA5D6, 0xA5E1, 0xA5F6, 0xA641, 0xA642, 0xA645, 0xA649, 0xA651, 0xA653, 0xA661,
        0xA665, 0xA681, 0xA682, 0xA685, 0xA688, 0xA689, 0xA68A, 0xA68B, 0xA691, 0xA693, 0xA695,
        0xA697, 0xA69B, 0xA69C, 0xA6A1, 0xA6A9, 0xA6B6, 0xA6C1, 0xA6E1, 0xA6E2, 0xA6E5, 0xA6E9,
        0xA6F7, 0xA741, 0xA745, 0xA749, 0xA751, 0xA755, 0xA757, 0xA761, 0xA762, 0xA765, 0xA769,
        0xA771, 0xA773, 0xA775, 0xA7A1, 0xA7A2, 0xA7A5, 0xA7A9, 0xA7AB, 0xA7B1, 0xA7B3, 0xA7B5,
        0xA7B7, 0xA7B8, 0xA7B9, 0xA861, 0xA862, 0xA865,

        0xA869, 0xA86B, 0xA871, 0xA873, 0xA875, 0xA876, 0xA877, 0xA87D, 0xA881, 0xA882, 0xA885,
        0xA889, 0xA891, 0xA893, 0xA895, 0xA896, 0xA897, 0xA8A1, 0xA8A2, 0xA8B1, 0xA8E1, 0xA8E2,
        0xA8E5, 0xA8E8, 0xA8E9, 0xA8F1, 0xA8F5, 0xA8F6, 0xA8F7, 0xA941, 0xA957, 0xA961, 0xA962,
        0xA971, 0xA973, 0xA975, 0xA976, 0xA977, 0xA9A1, 0xA9A2, 0xA9A5, 0xA9A9, 0xA9B1, 0xA9B3,
        0xA9B7, 0xAA41, 0xAA61, 0xAA77, 0xAA81, 0xAA82, 0xAA85, 0xAA89, 0xAA91, 0xAA95, 0xAA97,
        0xAB41, 0xAB57, 0xAB61, 0xAB65, 0xAB69, 0xAB71, 0xAB73, 0xABA1, 0xABA2, 0xABA5, 0xABA9,
        0xABB1, 0xABB3, 0xABB5, 0xABB7, 0xAC61, 0xAC62, 0xAC64, 0xAC65, 0xAC68, 0xAC69, 0xAC6A,
        0xAC6B, 0xAC71, 0xAC73, 0xAC75, 0xAC76, 0xAC77, 0xAC7B, 0xAC81, 0xAC82, 0xAC85, 0xAC89,
        0xAC91, 0xAC93, 0xAC95, 0xAC96, 0xAC97, 0xACA1,

        0xACA2, 0xACA5, 0xACA9, 0xACB1, 0xACB3, 0xACB5, 0xACB7, 0xACC1, 0xACC5, 0xACC9, 0xACD1,
        0xACD7, 0xACE1, 0xACE2, 0xACE3, 0xACE4, 0xACE5, 0xACE8, 0xACE9, 0xACEB, 0xACEC, 0xACF1,
        0xACF3, 0xACF5, 0xACF6, 0xACF7, 0xACFC, 0xAD41, 0xAD42, 0xAD45, 0xAD49, 0xAD51, 0xAD53,
        0xAD55, 0xAD56, 0xAD57, 0xAD61, 0xAD62, 0xAD65, 0xAD69, 0xAD71, 0xAD73, 0xAD75, 0xAD76,
        0xAD77, 0xAD81, 0xAD85, 0xAD89, 0xAD97, 0xADA1, 0xADA2, 0xADA3, 0xADA5, 0xADA9, 0xADAB,
        0xADB1, 0xADB3, 0xADB5, 0xADB7, 0xADBB, 0xADC1, 0xADC2, 0xADC5, 0xADC9, 0xADD7, 0xADE1,
        0xADE5, 0xADE9, 0xADF1, 0xADF5, 0xADF6, 0xAE41, 0xAE45, 0xAE49, 0xAE51, 0xAE53, 0xAE55,
        0xAE61, 0xAE62, 0xAE65, 0xAE69, 0xAE71, 0xAE73, 0xAE75, 0xAE77, 0xAE81, 0xAE82, 0xAE85,
        0xAE88, 0xAE89, 0xAE91, 0xAE93, 0xAE95, 0xAE97,

        0xAE99, 0xAE9B, 0xAE9C, 0xAEA1, 0xAEB6, 0xAEC1, 0xAEC2, 0xAEC5, 0xAEC9, 0xAED1, 0xAED7,
        0xAEE1, 0xAEE2, 0xAEE5, 0xAEE9, 0xAEF1, 0xAEF3, 0xAEF5, 0xAEF7, 0xAF41, 0xAF42, 0xAF49,
        0xAF51, 0xAF55, 0xAF57, 0xAF61, 0xAF62, 0xAF65, 0xAF69, 0xAF6A, 0xAF71, 0xAF73, 0xAF75,
        0xAF77, 0xAFA1, 0xAFA2, 0xAFA5, 0xAFA8, 0xAFA9, 0xAFB0, 0xAFB1, 0xAFB3, 0xAFB5, 0xAFB7,
        0xAFBC, 0xB061, 0xB062, 0xB064, 0xB065, 0xB069, 0xB071, 0xB073, 0xB076, 0xB077, 0xB07D,
        0xB081, 0xB082, 0xB085, 0xB089, 0xB091, 0xB093, 0xB096, 0xB097, 0xB0B7, 0xB0E1, 0xB0E2,
        0xB0E5, 0xB0E9, 0xB0EB, 0xB0F1, 0xB0F3, 0xB0F6, 0xB0F7, 0xB141, 0xB145, 0xB149, 0xB185,
        0xB1A1, 0xB1A2, 0xB1A5, 0xB1A8, 0xB1A9, 0xB1AB, 0xB1B1, 0xB1B3, 0xB1B7, 0xB1C1, 0xB1C2,
        0xB1C5, 0xB1D6, 0xB1E1, 0xB1F6, 0xB241, 0xB245,

        0xB249, 0xB251, 0xB253, 0xB261, 0xB281, 0xB282, 0xB285, 0xB289, 0xB291, 0xB293, 0xB297,
        0xB2A1, 0xB2B6, 0xB2C1, 0xB2E1, 0xB2E5, 0xB357, 0xB361, 0xB362, 0xB365, 0xB369, 0xB36B,
        0xB370, 0xB371, 0xB373, 0xB381, 0xB385, 0xB389, 0xB391, 0xB3A1, 0xB3A2, 0xB3A5, 0xB3A9,
        0xB3B1, 0xB3B3, 0xB3B5, 0xB3B7, 0xB461, 0xB462, 0xB465, 0xB466, 0xB467, 0xB469, 0xB46A,
        0xB46B, 0xB470, 0xB471, 0xB473, 0xB475, 0xB476, 0xB477, 0xB47B, 0xB47C, 0xB481, 0xB482,
        0xB485, 0xB489, 0xB491, 0xB493, 0xB495, 0xB496, 0xB497, 0xB4A1, 0xB4A2, 0xB4A5, 0xB4A9,
        0xB4AC, 0xB4B1, 0xB4B3, 0xB4B5, 0xB4B7, 0xB4BB, 0xB4BD, 0xB4C1, 0xB4C5, 0xB4C9, 0xB4D3,
        0xB4E1, 0xB4E2, 0xB4E5, 0xB4E6, 0xB4E8, 0xB4E9, 0xB4EA, 0xB4EB, 0xB4F1, 0xB4F3, 0xB4F4,
        0xB4F5, 0xB4F6, 0xB4F7, 0xB4F8, 0xB4FA, 0xB4FC,

        0xB541, 0xB542, 0xB545, 0xB549, 0xB551, 0xB553, 0xB555, 0xB557, 0xB561, 0xB562, 0xB563,
        0xB565, 0xB569, 0xB56B, 0xB56C, 0xB571, 0xB573, 0xB574, 0xB575, 0xB576, 0xB577, 0xB57B,
        0xB57C, 0xB57D, 0xB581, 0xB585, 0xB589, 0xB591, 0xB593, 0xB595, 0xB596, 0xB5A1, 0xB5A2,
        0xB5A5, 0xB5A9, 0xB5AA, 0xB5AB, 0xB5AD, 0xB5B0, 0xB5B1, 0xB5B3, 0xB5B5, 0xB5B7, 0xB5B9,
        0xB5C1, 0xB5C2, 0xB5C5, 0xB5C9, 0xB5D1, 0xB5D3, 0xB5D5, 0xB5D6, 0xB5D7, 0xB5E1, 0xB5E2,
        0xB5E5, 0xB5F1, 0xB5F5, 0xB5F7, 0xB641, 0xB642, 0xB645, 0xB649, 0xB651, 0xB653, 0xB655,
        0xB657, 0xB661, 0xB662, 0xB665, 0xB669, 0xB671, 0xB673, 0xB675, 0xB677, 0xB681, 0xB682,
        0xB685, 0xB689, 0xB68A, 0xB68B, 0xB691, 0xB693, 0xB695, 0xB697, 0xB6A1, 0xB6A2, 0xB6A5,
        0xB6A9, 0xB6B1, 0xB6B3, 0xB6B6, 0xB6B7, 0xB6C1,

        0xB6C2, 0xB6C5, 0xB6C9, 0xB6D1, 0xB6D3, 0xB6D7, 0xB6E1, 0xB6E2, 0xB6E5, 0xB6E9, 0xB6F1,
        0xB6F3, 0xB6F5, 0xB6F7, 0xB741, 0xB742, 0xB745, 0xB749, 0xB751, 0xB753, 0xB755, 0xB757,
        0xB759, 0xB761, 0xB762, 0xB765, 0xB769, 0xB76F, 0xB771, 0xB773, 0xB775, 0xB777, 0xB778,
        0xB779, 0xB77A, 0xB77B, 0xB77C, 0xB77D, 0xB781, 0xB785, 0xB789, 0xB791, 0xB795, 0xB7A1,
        0xB7A2, 0xB7A5, 0xB7A9, 0xB7AA, 0xB7AB, 0xB7B0, 0xB7B1, 0xB7B3, 0xB7B5, 0xB7B6, 0xB7B7,
        0xB7B8, 0xB7BC, 0xB861, 0xB862, 0xB865, 0xB867, 0xB868, 0xB869, 0xB86B, 0xB871, 0xB873,
        0xB875, 0xB876, 0xB877, 0xB878, 0xB881, 0xB882, 0xB885, 0xB889, 0xB891, 0xB893, 0xB895,
        0xB896, 0xB897, 0xB8A1, 0xB8A2, 0xB8A5, 0xB8A7, 0xB8A9, 0xB8B1, 0xB8B7, 0xB8C1, 0xB8C5,
        0xB8C9, 0xB8E1, 0xB8E2, 0xB8E5, 0xB8E9, 0xB8EB,

        0xB8F1, 0xB8F3, 0xB8F5, 0xB8F7, 0xB8F8, 0xB941, 0xB942, 0xB945, 0xB949, 0xB951, 0xB953,
        0xB955, 0xB957, 0xB961, 0xB965, 0xB969, 0xB971, 0xB973, 0xB976, 0xB977, 0xB981, 0xB9A1,
        0xB9A2, 0xB9A5, 0xB9A9, 0xB9AB, 0xB9B1, 0xB9B3, 0xB9B5, 0xB9B7, 0xB9B8, 0xB9B9, 0xB9BD,
        0xB9C1, 0xB9C2, 0xB9C9, 0xB9D3, 0xB9D5, 0xB9D7, 0xB9E1, 0xB9F6, 0xB9F7, 0xBA41, 0xBA45,
        0xBA49, 0xBA51, 0xBA53, 0xBA55, 0xBA57, 0xBA61, 0xBA62, 0xBA65, 0xBA77, 0xBA81, 0xBA82,
        0xBA85, 0xBA89, 0xBA8A, 0xBA8B, 0xBA91, 0xBA93, 0xBA95, 0xBA97, 0xBAA1, 0xBAB6, 0xBAC1,
        0xBAE1, 0xBAE2, 0xBAE5, 0xBAE9, 0xBAF1, 0xBAF3, 0xBAF5, 0xBB41, 0xBB45, 0xBB49, 0xBB51,
        0xBB61, 0xBB62, 0xBB65, 0xBB69, 0xBB71, 0xBB73, 0xBB75, 0xBB77, 0xBBA1, 0xBBA2, 0xBBA5,
        0xBBA8, 0xBBA9, 0xBBAB, 0xBBB1, 0xBBB3, 0xBBB5,

        0xBBB7, 0xBBB8, 0xBBBB, 0xBBBC, 0xBC61, 0xBC62, 0xBC65, 0xBC67, 0xBC69, 0xBC6C, 0xBC71,
        0xBC73, 0xBC75, 0xBC76, 0xBC77, 0xBC81, 0xBC82, 0xBC85, 0xBC89, 0xBC91, 0xBC93, 0xBC95,
        0xBC96, 0xBC97, 0xBCA1, 0xBCA5, 0xBCB7, 0xBCE1, 0xBCE2, 0xBCE5, 0xBCE9, 0xBCF1, 0xBCF3,
        0xBCF5, 0xBCF6, 0xBCF7, 0xBD41, 0xBD57, 0xBD61, 0xBD76, 0xBDA1, 0xBDA2, 0xBDA5, 0xBDA9,
        0xBDB1, 0xBDB3, 0xBDB5, 0xBDB7, 0xBDB9, 0xBDC1, 0xBDC2, 0xBDC9, 0xBDD6, 0xBDE1, 0xBDF6,
        0xBE41, 0xBE45, 0xBE49, 0xBE51, 0xBE53, 0xBE77, 0xBE81, 0xBE82, 0xBE85, 0xBE89, 0xBE91,
        0xBE93, 0xBE97, 0xBEA1, 0xBEB6, 0xBEB7, 0xBEE1, 0xBF41, 0xBF61, 0xBF71, 0xBF75, 0xBF77,
        0xBFA1, 0xBFA2, 0xBFA5, 0xBFA9, 0xBFB1, 0xBFB3, 0xBFB7, 0xBFB8, 0xBFBD, 0xC061, 0xC062,
        0xC065, 0xC067, 0xC069, 0xC071, 0xC073, 0xC075,

        0xC076, 0xC077, 0xC078, 0xC081, 0xC082, 0xC085, 0xC089, 0xC091, 0xC093, 0xC095, 0xC096,
        0xC097, 0xC0A1, 0xC0A5, 0xC0A7, 0xC0A9, 0xC0B1, 0xC0B7, 0xC0E1, 0xC0E2, 0xC0E5, 0xC0E9,
        0xC0F1, 0xC0F3, 0xC0F5, 0xC0F6, 0xC0F7, 0xC141, 0xC142, 0xC145, 0xC149, 0xC151, 0xC153,
        0xC155, 0xC157, 0xC161, 0xC165, 0xC176, 0xC181, 0xC185, 0xC197, 0xC1A1, 0xC1A2, 0xC1A5,
        0xC1A9, 0xC1B1, 0xC1B3, 0xC1B5, 0xC1B7, 0xC1C1, 0xC1C5, 0xC1C9, 0xC1D7, 0xC241, 0xC245,
        0xC249, 0xC251, 0xC253, 0xC255, 0xC257, 0xC261, 0xC271, 0xC281, 0xC282, 0xC285, 0xC289,
        0xC291, 0xC293, 0xC295, 0xC297, 0xC2A1, 0xC2B6, 0xC2C1, 0xC2C5, 0xC2E1, 0xC2E5, 0xC2E9,
        0xC2F1, 0xC2F3, 0xC2F5, 0xC2F7, 0xC341, 0xC345, 0xC349, 0xC351, 0xC357, 0xC361, 0xC362,
        0xC365, 0xC369, 0xC371, 0xC373, 0xC375, 0xC377,

        0xC3A1, 0xC3A2, 0xC3A5, 0xC3A8, 0xC3A9, 0xC3AA, 0xC3B1, 0xC3B3, 0xC3B5, 0xC3B7, 0xC461,
        0xC462, 0xC465, 0xC469, 0xC471, 0xC473, 0xC475, 0xC477, 0xC481, 0xC482, 0xC485, 0xC489,
        0xC491, 0xC493, 0xC495, 0xC496, 0xC497, 0xC4A1, 0xC4A2, 0xC4B7, 0xC4E1, 0xC4E2, 0xC4E5,
        0xC4E8, 0xC4E9, 0xC4F1, 0xC4F3, 0xC4F5, 0xC4F6, 0xC4F7, 0xC541, 0xC542, 0xC545, 0xC549,
        0xC551, 0xC553, 0xC555, 0xC557, 0xC561, 0xC565, 0xC569, 0xC571, 0xC573, 0xC575, 0xC576,
        0xC577, 0xC581, 0xC5A1, 0xC5A2, 0xC5A5, 0xC5A9, 0xC5B1, 0xC5B3, 0xC5B5, 0xC5B7, 0xC5C1,
        0xC5C2, 0xC5C5, 0xC5C9, 0xC5D1, 0xC5D7, 0xC5E1, 0xC5F7, 0xC641, 0xC649, 0xC661, 0xC681,
        0xC682, 0xC685, 0xC689, 0xC691, 0xC693, 0xC695, 0xC697, 0xC6A1, 0xC6A5, 0xC6A9, 0xC6B7,
        0xC6C1, 0xC6D7, 0xC6E1, 0xC6E2, 0xC6E5, 0xC6E9,

        0xC6F1, 0xC6F3, 0xC6F5, 0xC6F7, 0xC741, 0xC745, 0xC749, 0xC751, 0xC761, 0xC762, 0xC765,
        0xC769, 0xC771, 0xC773, 0xC777, 0xC7A1, 0xC7A2, 0xC7A5, 0xC7A9, 0xC7B1, 0xC7B3, 0xC7B5,
        0xC7B7, 0xC861, 0xC862, 0xC865, 0xC869, 0xC86A, 0xC871, 0xC873, 0xC875, 0xC876, 0xC877,
        0xC881, 0xC882, 0xC885, 0xC889, 0xC891, 0xC893, 0xC895, 0xC896, 0xC897, 0xC8A1, 0xC8B7,
        0xC8E1, 0xC8E2, 0xC8E5, 0xC8E9, 0xC8EB, 0xC8F1, 0xC8F3, 0xC8F5, 0xC8F6, 0xC8F7, 0xC941,
        0xC942, 0xC945, 0xC949, 0xC951, 0xC953, 0xC955, 0xC957, 0xC961, 0xC965, 0xC976, 0xC981,
        0xC985, 0xC9A1, 0xC9A2, 0xC9A5, 0xC9A9, 0xC9B1, 0xC9B3, 0xC9B5, 0xC9B7, 0xC9BC, 0xC9C1,
        0xC9C5, 0xC9E1, 0xCA41, 0xCA45, 0xCA55, 0xCA57, 0xCA61, 0xCA81, 0xCA82, 0xCA85, 0xCA89,
        0xCA91, 0xCA93, 0xCA95, 0xCA97, 0xCAA1, 0xCAB6,

        0xCAC1, 0xCAE1, 0xCAE2, 0xCAE5, 0xCAE9, 0xCAF1, 0xCAF3, 0xCAF7, 0xCB41, 0xCB45, 0xCB49,
        0xCB51, 0xCB57, 0xCB61, 0xCB62, 0xCB65, 0xCB68, 0xCB69, 0xCB6B, 0xCB71, 0xCB73, 0xCB75,
        0xCB81, 0xCB85, 0xCB89, 0xCB91, 0xCB93, 0xCBA1, 0xCBA2, 0xCBA5, 0xCBA9, 0xCBB1, 0xCBB3,
        0xCBB5, 0xCBB7, 0xCC61, 0xCC62, 0xCC63, 0xCC65, 0xCC69, 0xCC6B, 0xCC71, 0xCC73, 0xCC75,
        0xCC76, 0xCC77, 0xCC7B, 0xCC81, 0xCC82, 0xCC85, 0xCC89, 0xCC91, 0xCC93, 0xCC95, 0xCC96,
        0xCC97, 0xCCA1, 0xCCA2, 0xCCE1, 0xCCE2, 0xCCE5, 0xCCE9, 0xCCF1, 0xCCF3, 0xCCF5, 0xCCF6,
        0xCCF7, 0xCD41, 0xCD42, 0xCD45, 0xCD49, 0xCD51, 0xCD53, 0xCD55, 0xCD57, 0xCD61, 0xCD65,
        0xCD69, 0xCD71, 0xCD73, 0xCD76, 0xCD77, 0xCD81, 0xCD89, 0xCD93, 0xCD95, 0xCDA1, 0xCDA2,
        0xCDA5, 0xCDA9, 0xCDB1, 0xCDB3, 0xCDB5, 0xCDB7,

        0xCDC1, 0xCDD7, 0xCE41, 0xCE45, 0xCE61, 0xCE65, 0xCE69, 0xCE73, 0xCE75, 0xCE81, 0xCE82,
        0xCE85, 0xCE88, 0xCE89, 0xCE8B, 0xCE91, 0xCE93, 0xCE95, 0xCE97, 0xCEA1, 0xCEB7, 0xCEE1,
        0xCEE5, 0xCEE9, 0xCEF1, 0xCEF5, 0xCF41, 0xCF45, 0xCF49, 0xCF51, 0xCF55, 0xCF57, 0xCF61,
        0xCF65, 0xCF69, 0xCF71, 0xCF73, 0xCF75, 0xCFA1, 0xCFA2, 0xCFA5, 0xCFA9, 0xCFB1, 0xCFB3,
        0xCFB5, 0xCFB7, 0xD061, 0xD062, 0xD065, 0xD069, 0xD06E, 0xD071, 0xD073, 0xD075, 0xD077,
        0xD081, 0xD082, 0xD085, 0xD089, 0xD091, 0xD093, 0xD095, 0xD096, 0xD097, 0xD0A1, 0xD0B7,
        0xD0E1, 0xD0E2, 0xD0E5, 0xD0E9, 0xD0EB, 0xD0F1, 0xD0F3, 0xD0F5, 0xD0F7, 0xD141, 0xD142,
        0xD145, 0xD149, 0xD151, 0xD153, 0xD155, 0xD157, 0xD161, 0xD162, 0xD165, 0xD169, 0xD171,
        0xD173, 0xD175, 0xD176, 0xD177, 0xD181, 0xD185,

        0xD189, 0xD193, 0xD1A1, 0xD1A2, 0xD1A5, 0xD1A9, 0xD1AE, 0xD1B1, 0xD1B3, 0xD1B5, 0xD1B7,
        0xD1BB, 0xD1C1, 0xD1C2, 0xD1C5, 0xD1C9, 0xD1D5, 0xD1D7, 0xD1E1, 0xD1E2, 0xD1E5, 0xD1F5,
        0xD1F7, 0xD241, 0xD242, 0xD245, 0xD249, 0xD253, 0xD255, 0xD257, 0xD261, 0xD265, 0xD269,
        0xD273, 0xD275, 0xD281, 0xD282, 0xD285, 0xD289, 0xD28E, 0xD291, 0xD295, 0xD297, 0xD2A1,
        0xD2A5, 0xD2A9, 0xD2B1, 0xD2B7, 0xD2C1, 0xD2C2, 0xD2C5, 0xD2C9, 0xD2D7, 0xD2E1, 0xD2E2,
        0xD2E5, 0xD2E9, 0xD2F1, 0xD2F3, 0xD2F5, 0xD2F7, 0xD341, 0xD342, 0xD345, 0xD349, 0xD351,
        0xD355, 0xD357, 0xD361, 0xD362, 0xD365, 0xD367, 0xD368, 0xD369, 0xD36A, 0xD371, 0xD373,
        0xD375, 0xD377, 0xD37B, 0xD381, 0xD385, 0xD389, 0xD391, 0xD393, 0xD397, 0xD3A1, 0xD3A2,
        0xD3A5, 0xD3A9, 0xD3B1, 0xD3B3, 0xD3B5, 0xD3B7};

LOCAL IMS_SINT32 str_SkipAtoI(IN CONST IMS_WCHAR** ppwszS);
LOCAL IMS_WCHAR* str_ParseNumber(OUT IMS_WCHAR* pwStr, IN IMS_SLONG nNum, IN IMS_SINT32 nBase,
        IN IMS_SINT32 nSize, IN IMS_SINT32 nPrecision, IN IMS_SINT32 nType);
LOCAL IMS_SINT32 str_UniVsprintf(
        OUT IMS_WCHAR* pwszBuf, IN CONST IMS_WCHAR* pwszFmt, IN va_list args);
LOCAL IMS_WCHAR* str_MakeFloat(OUT IMS_WCHAR* pwszStr, IN IMS_DOUBLE dValue,
        IN IMS_SINT32 nFieldWidth, IN IMS_SINT32 nPrecision, IN IMS_SINT32 nFlags);
LOCAL IMS_DOUBLE str_Round(IN IMS_DOUBLE dValue);
LOCAL IMS_DOUBLE str_Pow10(IN IMS_SINT32 nExp);
LOCAL IMS_DOUBLE str_Modf(IN IMS_DOUBLE dFrac, OUT IMS_DOUBLE* pdPtr);
LOCAL IMS_UINT32 str_Utf8ToUcs(OUT IMS_WCHAR* pwszUcs, IN CONST IMS_CHAR* pszUtf8);
LOCAL IMS_UINT32 str_UcsToUtf8(OUT IMS_CHAR* pszUtf8, IN IMS_WCHAR wcUcs);
LOCAL IMS_WCHAR str_JohapToWansung(IN IMS_WCHAR wcJohap);
LOCAL IMS_WCHAR str_WansungToJohap(IN IMS_WCHAR wcWansung);
LOCAL IMS_WCHAR str_JohapToUcs(IN IMS_WCHAR wcJohap);
LOCAL IMS_WCHAR str_UcsToJohap(IN IMS_WCHAR wcUcs2);

/* return string length */
GLOBAL IMS_UINT32 IMS_StrLen(IN CONST IMS_CHAR* pszStr)
{
    if (pszStr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nCount = 0;

    while (*pszStr)
    {
        ++pszStr;
        ++nCount;
    }

    return nCount;
}

/* safe string copy

Remarks
nDestSize : buffer size including null('\0') character
*/
GLOBAL IMS_UINT32 IMS_StrCpy(
        OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_CHAR* pszSrc)
{
    if ((pszSrc == IMS_NULL) || (pszDest == IMS_NULL))
    {
        return 0;
    }

    IMS_SIZE_T nSrcLen = IMS_StrLen(pszSrc);

    pszDest[0] = '\0';

    if (nDestSize <= nSrcLen)
    {
        return 0;
    }

    IMS_MEM_Memcpy(pszDest, pszSrc, nSrcLen);
    pszDest[nSrcLen] = '\0';

    return nSrcLen;
}

/* safe string n copy

Remarks
nSrcSize : buffer size excluding null('\0') character
nDestSize : buffer size including null('\0') character
*/
GLOBAL IMS_UINT32 IMS_StrNCpy(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_CHAR* pszSrc, IN IMS_SIZE_T nSrcSize)
{
    if ((pszSrc == IMS_NULL) || (pszDest == IMS_NULL))
    {
        return 0;
    }

    IMS_SIZE_T nSrcLen = IMS_StrLen(pszSrc);

    pszDest[0] = '\0';

    if (nSrcLen > nSrcSize)
    {
        nSrcLen = nSrcSize;
    }

    if (nDestSize <= nSrcLen)
    {
        return 0;
    }

    IMS_MEM_Memcpy(pszDest, pszSrc, nSrcLen);
    pszDest[nSrcLen] = '\0';

    return nSrcLen;
}

/* safe string cat

Remarks
nDestSize : buffer size including null('\0') character
*/
GLOBAL IMS_UINT32 IMS_StrCat(
        OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_CHAR* pszSrc)
{
    if ((pszSrc == IMS_NULL) || (pszDest == IMS_NULL))
    {
        return 0;
    }

    IMS_SIZE_T nDestLen = IMS_StrLen(pszDest);
    IMS_SIZE_T nSrcLen = IMS_StrLen(pszSrc);

    if (nDestSize <= (nDestLen + nSrcLen))
    {
        return 0;
    }

    IMS_MEM_Memcpy(pszDest + nDestLen, pszSrc, nSrcLen);
    pszDest[nDestLen + nSrcLen] = '\0';

    return nDestLen + nSrcLen;
}

/* safe string n cat

Remarks
nSrcSize : buffer size excluding null('\0') character
nDestSize : buffer size including null('\0') character
*/
GLOBAL IMS_UINT32 IMS_StrNCat(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_CHAR* pszSrc, IN IMS_SIZE_T nSrcSize)
{
    if ((pszSrc == IMS_NULL) || (pszDest == IMS_NULL))
    {
        return 0;
    }

    IMS_SIZE_T nDestLen = IMS_StrLen(pszDest);
    IMS_SIZE_T nSrcLen = IMS_StrLen(pszSrc);

    if (nSrcLen > nSrcSize)
    {
        nSrcLen = nSrcSize;
    }

    if (nDestSize <= (nDestLen + nSrcLen))
    {
        return 0;
    }

    IMS_MEM_Memcpy(pszDest + nDestLen, pszSrc, nSrcLen);
    pszDest[nDestLen + nSrcLen] = '\0';

    return nDestLen + nSrcLen;
}

/* safe string printf

Remarks
nBufSize : buffer size excluding null('\0') character

Returns
<table>
return             description
----------         ----------
positive, 0        the number of characters written
-1                 if an output error occurs
</table> */
GLOBAL IMS_SINT32 IMS_Sprintf(
        OUT IMS_CHAR* pszBuf, IN IMS_SIZE_T nBufSize, IN CONST IMS_CHAR* pszFormat, ...)
{
    (void)nBufSize;

    if (pszBuf == IMS_NULL || pszFormat == IMS_NULL)
    {
        return 0;
    }

    IMS_SINT32 nSize;
    va_list args;

    *pszBuf = '\0';

    va_start(args, pszFormat);
    nSize = vsnprintf(pszBuf, nBufSize, pszFormat, args);
    va_end(args);

    return nSize;
}

GLOBAL IMS_CHAR* IMS_StrDup(IN CONST IMS_CHAR* pszSrc)
{
    if (pszSrc == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_UINT32 nSrcLen = IMS_StrLen(pszSrc);
    IMS_CHAR* pszTarget = (IMS_CHAR*)IMS_MEM_Malloc(nSrcLen + 1);

    if (pszTarget == IMS_NULL)
    {
        return IMS_NULL;
    }

    *pszTarget = '\0';
    IMS_MEM_Memcpy(pszTarget, pszSrc, nSrcLen + 1);

    return pszTarget;
}

GLOBAL IMS_UINT32 IMS_Itoa(OUT IMS_CHAR* pszStr, IN IMS_SINT32 nNum, IN IMS_UINT32 nBase)
{
    if (pszStr == IMS_NULL)
    {
        return 0;
    }

    *pszStr = '\0';

    switch (nBase)
    {
        case 8:
            sprintf(pszStr, "%o", nNum);
            break;
        case 10:
            sprintf(pszStr, "%d", nNum);
            break;
        case 16:
            sprintf(pszStr, "%x", nNum);
            break;
        default:
            return 0;
    }

    return IMS_StrLen(pszStr);
}

GLOBAL IMS_SINT32 IMS_Atoi(IN CONST IMS_CHAR* pszStr)
{
    if (pszStr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nStrLen = IMS_StrLen(pszStr);

    if ((nStrLen > 11) && (pszStr[0] == '-'))
    {
        nStrLen = 11;
    }
    else if (nStrLen > 10) /*2^31 = 2147483648*/
    {
        nStrLen = 10;
    }

    IMS_SINT32 nValue = 0;

    for (IMS_UINT32 i = 0; i < nStrLen; ++i)
    {
        if (pszStr[i] >= '0' && pszStr[i] <= '9')
        {
            nValue = nValue * 10 + (pszStr[i] - '0');
        }
        else if (pszStr[i] == '.')
        {
            break;
        }
    }

    if (pszStr[0] == '-')
    {
        nValue *= (-1);
    }

    return nValue;
}

GLOBAL IMS_CHAR* IMS_StrChr(IN CONST IMS_CHAR* pszStr, IN IMS_CHAR cChar)
{
    return (IMS_CHAR*)strchr(pszStr, cChar);
}

GLOBAL IMS_CHAR* IMS_StrRChr(IN CONST IMS_CHAR* pszStr, IN IMS_CHAR cChar)
{
    return (IMS_CHAR*)strrchr(pszStr, cChar);
}

GLOBAL IMS_CHAR* IMS_StrStr(IN CONST IMS_CHAR* pszA, IN CONST IMS_CHAR* pszB)
{
    return (IMS_CHAR*)strstr(pszA, pszB);
}

GLOBAL IMS_SINT32 IMS_StrCmp(IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB)
{
    if ((pszStrA == IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 0;
    }
    else if ((pszStrA == IMS_NULL) && (pszStrB != IMS_NULL))
    {
        return (-1);
    }
    else if ((pszStrA != IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 1;
    }

    while (*pszStrA && *pszStrB)
    {
        if (*pszStrA != *pszStrB)
        {
            return (*pszStrA) - (*pszStrB);
        }

        ++pszStrA;
        ++pszStrB;
    }

    if ((*pszStrA == 0) && (*pszStrB != 0))
    {
        return (-1);
    }
    else if ((*pszStrA != 0) && (*pszStrB == 0))
    {
        return 1;
    }

    return 0;
}

GLOBAL IMS_SINT32 IMS_StrNCmp(
        IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize)
{
    // 3 Please, modify this function according to the standard string manipulation
    IMS_SIZE_T nCount = nSize;

    if (pszStrA == IMS_NULL || pszStrB == IMS_NULL || nCount == 0)
    {
        return -1;
    }

    while ((*pszStrA) && (*pszStrB) && (nCount != 0))
    {
        if (*pszStrA != *pszStrB)
        {
            return (*pszStrA) - (*pszStrB);
        }

        ++pszStrA;
        ++pszStrB;
        nCount--;
    }

    return (nCount != 0) ? -1 : 0;
}

GLOBAL IMS_SINT32 IMS_StrICmp(IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB)
{
    IMS_CHAR ch1;
    IMS_CHAR ch2;

    if ((pszStrA == IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 0;
    }
    else if ((pszStrA == IMS_NULL) && (pszStrB != IMS_NULL))
    {
        return (-1);
    }
    else if ((pszStrA != IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 1;
    }

    while (*pszStrA && *pszStrB)
    {
        ch1 = IMS_TOLOWER(*pszStrA);
        ch2 = IMS_TOLOWER(*pszStrB);

        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        ++pszStrA;
        ++pszStrB;
    }

    if ((*pszStrA == 0) && (*pszStrB != 0))
    {
        return (-1);
    }
    else if ((*pszStrA != 0) && (*pszStrB == 0))
    {
        return 1;
    }

    return 0;
}

GLOBAL IMS_SINT32 IMS_StrNICmp(
        IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize)
{
    // 3 Please, modify this function according to the standard string manipulation
    IMS_SIZE_T nCount = nSize;

    if (pszStrA == IMS_NULL || pszStrB == IMS_NULL || nCount == 0)
    {
        return (-1);
    }

    while ((*pszStrA) && (*pszStrB) && (nCount != 0))
    {
        IMS_CHAR ch1 = IMS_TOLOWER(*pszStrA);
        IMS_CHAR ch2 = IMS_TOLOWER(*pszStrB);

        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        ++pszStrA;
        ++pszStrB;
        nCount--;
    }

    return (nCount != 0) ? -1 : 0;
}

GLOBAL IMS_SINT32 IMS_StrCmpUppercase(IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB)
{
    IMS_CHAR ch1;
    IMS_CHAR ch2;

    if ((pszStrA == IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 0;
    }
    else if ((pszStrA == IMS_NULL) && (pszStrB != IMS_NULL))
    {
        return (-1);
    }
    else if ((pszStrA != IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 1;
    }

    while (*pszStrA && *pszStrB)
    {
        ch1 = IMS_TOUPPER(*pszStrA);
        ch2 = IMS_TOUPPER(*pszStrB);

        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        ++pszStrA;
        ++pszStrB;
    }

    if ((*pszStrA == 0) && (*pszStrB != 0))
    {
        return (-1);
    }
    else if ((*pszStrA != 0) && (*pszStrB == 0))
    {
        return 1;
    }

    return 0;
}

GLOBAL IMS_BOOL IMS_StrToLowerCase(OUT IMS_CHAR* pszOutStr, IN CONST IMS_CHAR* pszInStr)
{
    if (pszInStr == IMS_NULL || pszOutStr == IMS_NULL || IMS_StrLen(pszInStr) == 0)
    {
        return IMS_FALSE;
    }

    while (*pszInStr)
    {
        *pszOutStr = *pszInStr;
        *pszOutStr = IMS_TOLOWER(*pszOutStr);

        ++pszInStr;
        ++pszOutStr;
    }

    *pszOutStr = '\0';

    return IMS_TRUE;
}

GLOBAL IMS_BOOL IMS_StrToUpperCase(OUT IMS_CHAR* pszOutStr, IN CONST IMS_CHAR* pszInStr)
{
    if (pszInStr == IMS_NULL || pszOutStr == IMS_NULL || IMS_StrLen(pszInStr) == 0)
    {
        return IMS_FALSE;
    }

    while (*pszInStr)
    {
        *pszOutStr = *pszInStr;
        *pszOutStr = IMS_TOUPPER(*pszOutStr);

        ++pszInStr;
        ++pszOutStr;
    }

    *pszOutStr = '\0';

    return IMS_TRUE;
}

GLOBAL IMS_UINT32 IMS_UcStrLen(IN CONST IMS_WCHAR* pwszStr)
{
    if (pwszStr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nCount = 0;

    while (*pwszStr)
    {
        ++pwszStr;
        ++nCount;
    }

    return nCount;
}

/* safe Convert Ascii string to UCS2 string

Remarks
nDestSize : buffer size excluding null('\0') character
*/
GLOBAL IMS_UINT32 IMS_StrToUcStr(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_CHAR* pszSrc)
{
    if ((pszSrc == IMS_NULL) || (pwszDest == IMS_NULL))
    {
        return 0;
    }

    pwszDest[0] = 0;

    if (nDestSize < IMS_StrLen(pszSrc))
    {
        return 0;
    }

    IMS_UINT32 nIndex = 0;

    while (pszSrc[nIndex] != 0 && nDestSize != 0)
    {
        pwszDest[nIndex] = (IMS_WCHAR)pszSrc[nIndex];

        ++nIndex;
        --nDestSize;
    }

    pwszDest[nIndex] = 0;

    return nIndex;
}

/* safe Convert UCS2 string to Ascii string

Remarks
nDestSize : buffer size excluding null('\0') character
*/
GLOBAL IMS_UINT32 IMS_UcStrToStr(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_WCHAR* pwszSrc, IN IMS_CHAR cDefaultChar)
{
    if ((pwszSrc == IMS_NULL) || (pszDest == IMS_NULL))
    {
        return 0;
    }

    pszDest[0] = '\0';

    if (nDestSize < IMS_UcStrLen(pwszSrc))
    {
        return 0;
    }

    IMS_UINT32 nIndex = 0;

    while (pwszSrc[nIndex] != 0 && nDestSize != 0)
    {
        /*if cur char is beyond ascii range*/
        if (pwszSrc[nIndex] & 0xFF00)
        {
            pszDest[nIndex] = cDefaultChar;

            if (cDefaultChar == '\0')
            {
                break;
            }
        }
        else
        {
            pszDest[nIndex] = (IMS_CHAR)pwszSrc[nIndex];
        }

        ++nIndex;
        --nDestSize;
    }

    pszDest[nIndex] = '\0';

    return nIndex;
}

/* safe UCS2 string copy

Remarks
nDestSize : buffer size excluding null('\0') character
*/
GLOBAL IMS_UINT32 IMS_UcStrCpy(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_WCHAR* pwszSrc)
{
    if ((pwszDest == IMS_NULL) || (pwszSrc == IMS_NULL))
    {
        return 0;
    }

    pwszDest[0] = 0;

    IMS_SIZE_T nSrcLen = IMS_UcStrLen(pwszSrc);

    if (nDestSize < nSrcLen)
    {
        return 0;
    }

    IMS_MEM_Memcpy(pwszDest, pwszSrc, nSrcLen * sizeof(IMS_WCHAR));
    pwszDest[nSrcLen] = 0;

    return nSrcLen;
}

/* safe UCS2 string n copy

Remarks
nSrcSize : buffer size excluding null('\0') character
nDestSize : buffer size excluding null('\0') character
*/
GLOBAL IMS_UINT32 IMS_UcStrNCpy(OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_WCHAR* pwszSrc, IN IMS_SIZE_T nSrcSize)
{
    if ((pwszDest == IMS_NULL) || (pwszSrc == IMS_NULL))
    {
        return 0;
    }

    pwszDest[0] = 0;

    IMS_SIZE_T nSrcLen = IMS_UcStrLen(pwszSrc);

    if (nSrcLen > nSrcSize)
    {
        nSrcLen = nSrcSize;
    }

    if (nDestSize < nSrcLen)
    {
        return 0;
    }

    IMS_MEM_Memcpy(pwszDest, pwszSrc, nSrcLen * sizeof(IMS_WCHAR));
    pwszDest[nSrcLen] = 0;

    return nSrcLen;
}

/* safe UCS2 string cat

Remarks
nDestSize : buffer size excluding null('\0') character
*/
GLOBAL IMS_UINT32 IMS_UcStrCat(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_WCHAR* pwszSrc)
{
    if ((pwszSrc == IMS_NULL) || (pwszDest == IMS_NULL))
    {
        return 0;
    }

    IMS_SIZE_T nDestLen = IMS_UcStrLen(pwszDest);
    IMS_SIZE_T nSrcLen = IMS_UcStrLen(pwszSrc);

    if (nDestSize < (nDestLen + nSrcLen))
    {
        return 0;
    }

    IMS_MEM_Memcpy(pwszDest + nDestLen, pwszSrc, nSrcLen * sizeof(IMS_WCHAR));
    pwszDest[nDestLen + nSrcLen] = 0;

    return nDestLen + nSrcLen;
}

/* safe UCS2 string n cat

Remarks
nSrcSize : buffer size excluding null('\0') character
nDestSize : buffer size excluding null('\0') character
*/
GLOBAL IMS_UINT32 IMS_UcStrNCat(OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_WCHAR* pwszSrc, IN IMS_SIZE_T nSrcSize)
{
    if ((pwszSrc == IMS_NULL) || (pwszDest == IMS_NULL))
    {
        return 0;
    }

    IMS_SIZE_T nDestLen = IMS_UcStrLen(pwszDest);
    IMS_SIZE_T nSrcLen = IMS_UcStrLen(pwszSrc);

    if (nSrcLen > nSrcSize)
    {
        nSrcLen = nSrcSize;
    }

    if (nDestSize < (nDestLen + nSrcLen))
    {
        return 0;
    }

    IMS_MEM_Memcpy(pwszDest + nDestLen, pwszSrc, nSrcLen * sizeof(IMS_WCHAR));
    pwszDest[nDestLen + nSrcLen] = 0;

    return nDestLen + nSrcLen;
}

GLOBAL IMS_SINT32 IMS_UcSprintf(OUT IMS_WCHAR* pwszBuf, IN IMS_SIZE_T nBufSize,
        IN CONST IMS_WCHAR* pwszFormat, IN va_list args)
{
    (void)nBufSize;

    if (pwszBuf == IMS_NULL || pwszFormat == IMS_NULL)
    {
        return 0;
    }

    *pwszBuf = 0;

    return str_UniVsprintf(pwszBuf, pwszFormat, args);
    // return vsnwprintf(pwszBuf, nBufSize, pwszFormat, args);
}

GLOBAL IMS_UINT32 IMS_UcItoa(
        OUT IMS_WCHAR* pwszBuf, IN IMS_SIZE_T nBufSize, IN IMS_SINT32 nNum, IN IMS_UINT32 nBase)
{
    if (pwszBuf == IMS_NULL)
    {
        return 0;
    }

    *pwszBuf = 0;

    // Max length for integer type including negative('-') : 11
    IMS_CHAR acNumber[11 + 1] = {
            0,
    };

    switch (nBase)
    {
        case 8:
            sprintf(acNumber, "%o", nNum);
            break;
        case 10:
            sprintf(acNumber, "%d", nNum);
            break;
        case 16:
            sprintf(acNumber, "%x", nNum);
            break;
        default:
            return 0;
    }

    return IMS_StrToUcStr(pwszBuf, nBufSize, acNumber);
}

GLOBAL IMS_SINT32 IMS_UcAtoi(IN CONST IMS_WCHAR* pwszStr)
{
    if (pwszStr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nStrLen = IMS_UcStrLen(pwszStr);

    if ((nStrLen > 11) && (pwszStr[0] == '-'))
    {
        nStrLen = 11;
    }
    else if (nStrLen > 10) /*2^31 = 2147483648*/
    {
        nStrLen = 10;
    }

    IMS_SINT32 nValue = 0;

    for (IMS_UINT32 i = 0; i < nStrLen; i++)
    {
        if (pwszStr[i] >= '0' && pwszStr[i] <= '9')
        {
            nValue = nValue * 10 + (pwszStr[i] - '0');
        }
        else if (pwszStr[i] == '.')
        {
            break;
        }
    }

    if (pwszStr[0] == '-')
    {
        nValue *= (-1);
    }

    return nValue;
}

GLOBAL IMS_WCHAR* IMS_UcStrStr(IN CONST IMS_WCHAR* pwszA, IN CONST IMS_WCHAR* pwszB)
{
    if (*pwszB == 0)
    {
        return const_cast<IMS_WCHAR*>(pwszA);
    }

    if (*pwszA == 0)
    {
        return (IMS_WCHAR*)IMS_NULL;
    }

    const IMS_WCHAR* pwszStr = pwszA;

    while (*pwszStr)
    {
        if (*pwszStr == *pwszB)
        {
            IMS_WCHAR* pwChar1 = const_cast<IMS_WCHAR*>(pwszStr);
            IMS_WCHAR* pwChar2 = const_cast<IMS_WCHAR*>(pwszB);

            while (*pwChar1 && *pwChar2 && ((*pwChar2 - *pwChar1) == 0))
            {
                ++pwChar2;
                ++pwChar1;
            }

            if (*pwChar2 == 0)
            {
                return const_cast<IMS_WCHAR*>(pwszStr);
            }
        }

        ++pwszStr;
    }

    return (IMS_WCHAR*)IMS_NULL;
}

GLOBAL IMS_SINT32 IMS_UcStrCmp(IN CONST IMS_WCHAR* pwszStrA, IN CONST IMS_WCHAR* pwszStrB)
{
    // 3 Please, modify this function according to the standard string manipulation

    if (pwszStrA == IMS_NULL || pwszStrB == IMS_NULL)
    {
        return -1;
    }

    while (*pwszStrA && *pwszStrB)
    {
        if (*pwszStrA != *pwszStrB)
        {
            return (*pwszStrA) - (*pwszStrB);
        }

        ++pwszStrA;
        ++pwszStrB;
    }

    return (*pwszStrA == 0 && *pwszStrB == 0) ? 0 : -1;
}

GLOBAL IMS_SINT32 IMS_UcStrNCmp(
        IN CONST IMS_WCHAR* pwszStrA, IN CONST IMS_WCHAR* pwszStrB, IN IMS_SIZE_T nSize)
{
    // 3 Please, modify this function according to the standard string manipulation

    if (pwszStrA == IMS_NULL || pwszStrB == IMS_NULL || nSize == 0)
    {
        return -1;
    }

    while ((*pwszStrA) && (*pwszStrB) && (nSize != 0))
    {
        if (*pwszStrA != *pwszStrB)
        {
            return (*pwszStrA) - (*pwszStrB);
        }

        ++pwszStrA;
        ++pwszStrB;
        --nSize;
    }

    return (nSize != 0) ? -1 : 0;
}

GLOBAL IMS_BOOL IMS_UcStrToLowerCase(OUT IMS_WCHAR* pwszOutStr, IN CONST IMS_WCHAR* pwszInStr)
{
    if (pwszInStr == IMS_NULL || pwszOutStr == IMS_NULL || IMS_UcStrLen(pwszInStr) == 0)
    {
        return IMS_FALSE;
    }

    while (*pwszInStr)
    {
        *pwszOutStr = *pwszInStr;
        *pwszOutStr = IMS_TOLOWER(*pwszOutStr);

        ++pwszInStr;
        ++pwszOutStr;
    }

    *pwszOutStr = 0;

    return IMS_TRUE;
}

GLOBAL IMS_BOOL IMS_UcStrToUpperCase(OUT IMS_WCHAR* pwszOutStr, IN CONST IMS_WCHAR* pwszInStr)
{
    if (pwszInStr == IMS_NULL || pwszOutStr == IMS_NULL || IMS_UcStrLen(pwszInStr) == 0)
    {
        return IMS_FALSE;
    }

    while (*pwszInStr)
    {
        *pwszOutStr = *pwszInStr;
        *pwszOutStr = IMS_TOUPPER(*pwszOutStr);

        ++pwszInStr;
        ++pwszOutStr;
    }

    *pwszOutStr = 0;

    return IMS_TRUE;
}

GLOBAL IMS_SINT32 IMS_UcStrCmpUppercase(IN CONST IMS_WCHAR* pwszStrA, IN CONST IMS_WCHAR* pwszStrB)
{
    // 3 Please, modify this function according to the standard string manipulation

    if (pwszStrA == IMS_NULL || pwszStrB == IMS_NULL)
    {
        return -1;
    }

    while (*pwszStrA && *pwszStrB)
    {
        IMS_WCHAR wcA;
        IMS_WCHAR wcB;

        wcA = IMS_TOUPPER(*pwszStrA);
        wcB = IMS_TOUPPER(*pwszStrB);

        if (wcA != wcB)
        {
            return wcA - wcB;
        }

        ++pwszStrA;
        ++pwszStrB;
    }

    return (*pwszStrA == 0 && *pwszStrB == 0) ? 0 : -1;
}

/* UTF8 String�� UCS-2 String���� ��ȯ�Ѵ�. Return Value: ������ UCS-2 ���� �� */
GLOBAL IMS_UINT32 IMS_Utf8ToUcs(OUT IMS_WCHAR* pwszUcs, IN CONST IMS_CHAR* pszUtf8)
{
    IMS_UINT32 nUtf8Len = IMS_StrLen(pszUtf8);
    IMS_UINT32 nUcsLen = 0;
    IMS_UINT32 nPassCount = 0;
    const IMS_CHAR* pcUtf8 = pszUtf8;
    IMS_WCHAR* pwcUcs = pwszUcs;

    while (nUtf8Len)
    {
        nPassCount = str_Utf8ToUcs(pwcUcs, pcUtf8);

        if (nPassCount == 0)
        {
            break;
        }

        nUtf8Len -= nPassCount;
        pcUtf8 += nPassCount;
        nUcsLen++;
        pwcUcs++;
    }

    return nUcsLen;
}

/* Convert UCS-2 string to UTF-8 string.
 The method returns the value as byte array of UTF-8 string.
 */
GLOBAL IMS_UINT32 IMS_UcsToUtf8(OUT IMS_CHAR* pszUtf8, IN CONST IMS_WCHAR* pwszUcs)
{
    IMS_UINT32 nUcsLen = IMS_UcStrLen(pwszUcs) + 1;
    IMS_UINT32 nUtf8Len = 0;
    IMS_UINT32 nPassCount = 0;
    const IMS_WCHAR* pwcUcs = pwszUcs;
    IMS_CHAR* pcUtf8 = pszUtf8;

    while (nUcsLen)
    {
        nPassCount = str_UcsToUtf8(pcUtf8, *pwcUcs);

        if (nPassCount == 0)
        {
            break;
        }

        nUcsLen--;
        pcUtf8 += nPassCount;
        nUtf8Len += nPassCount;
        pwcUcs++;
    }

    /* Don't add the length for null character */
    return (nUtf8Len - 1);
}

GLOBAL IMS_UINT32 IMS_Utf8ToEuckr(OUT IMS_CHAR* pszEuckr, IN CONST IMS_CHAR* pszUtf8)
{
    pszEuckr[0] = 0;

    if (pszUtf8[0] == 0)
    {
        return 0;
    }

    IMS_UINT32 nEuckr;
    IMS_UINT32 n2ByteStr;
    IMS_WCHAR w2ByteStr;
    IMS_WCHAR wcJohap;

    IMS_WCHAR* pwszUcs = (IMS_WCHAR*)IMS_MEM_Malloc((IMS_StrLen(pszUtf8) * 2) + 2);
    IMS_UINT32 nUcs = IMS_Utf8ToUcs(pwszUcs, pszUtf8);

    for (nEuckr = 0, n2ByteStr = 0; n2ByteStr < nUcs; n2ByteStr++)
    {
        if (pwszUcs[n2ByteStr] < 0x80)
        {
            pszEuckr[nEuckr++] = (IMS_UCHAR)pwszUcs[n2ByteStr];
        }
        else
        {
            wcJohap = str_UcsToJohap(pwszUcs[n2ByteStr]);
            w2ByteStr = str_JohapToWansung(wcJohap);
            pszEuckr[nEuckr++] = (IMS_UCHAR)((w2ByteStr >> 8) & 0xFF);
            pszEuckr[nEuckr++] = (IMS_UCHAR)(w2ByteStr & 0xFF);
        }
    }
    pszEuckr[nEuckr] = 0;
    IMS_MEM_Free(pwszUcs);

    return nEuckr;
}

GLOBAL IMS_UINT32 IMS_EuckrToUtf8(OUT IMS_CHAR* pszUtf8, IN CONST IMS_CHAR* pszEuckr)
{
    pszUtf8[0] = 0;

    if (pszEuckr[0] == 0)
    {
        return 0;
    }

    IMS_UINT32 nEuckr;
    IMS_UINT32 nUcs;
    IMS_UINT32 nUtf8;
    IMS_WCHAR w2ByteStr;
    IMS_WCHAR wcJohap;
    IMS_UINT32 nLenEuckr = IMS_StrLen(pszEuckr);
    IMS_WCHAR* pwszUcs = (IMS_WCHAR*)IMS_MEM_Malloc((nLenEuckr * 2) + 2);

    for (nEuckr = 0, nUcs = 0; nEuckr < nLenEuckr; nUcs++)
    {
        if ((IMS_UCHAR)pszEuckr[nEuckr] < 0x80)
        {
            pwszUcs[nUcs] = (IMS_WCHAR)pszEuckr[nEuckr++];
        }
        else
        {
            w2ByteStr = (IMS_WCHAR)(pszEuckr[nEuckr] * 0x100) +
                    (IMS_WCHAR)(pszEuckr[nEuckr + 1] & 0xFF);
            wcJohap = str_WansungToJohap(w2ByteStr);
            pwszUcs[nUcs] = str_JohapToUcs(wcJohap);
            nEuckr += 2;
        }
    }
    pwszUcs[nUcs] = 0;

    nUtf8 = IMS_UcsToUtf8(pszUtf8, pwszUcs);
    pszUtf8[nUtf8] = 0;

    IMS_MEM_Free(pwszUcs);

    return nUtf8;
}

GLOBAL IMS_UINT32 IMS_UcsToEuckr(
        OUT IMS_CHAR* pszEuckr, IN IMS_SIZE_T nEuckrSize, IN CONST IMS_WCHAR* pwszUcs)
{
    if (pszEuckr == IMS_NULL)
    {
        return 0;
    }

    pszEuckr[0] = 0;

    if (pwszUcs == IMS_NULL)
    {
        return 0;
    }

    IMS_SIZE_T nEuckr;
    IMS_UINT32 n2ByteStr;
    IMS_WCHAR w2ByteStr;
    IMS_WCHAR wcJohap;
    IMS_UINT32 nUcs = IMS_UcStrLen(pwszUcs);

    for (nEuckr = 0, n2ByteStr = 0; n2ByteStr < nUcs; n2ByteStr++)
    {
        if (pwszUcs[n2ByteStr] < 0x80)
        {
            if (nEuckr >= nEuckrSize)
            {
                break;
            }

            pszEuckr[nEuckr++] = (IMS_UCHAR)pwszUcs[n2ByteStr];
        }
        else
        {
            if ((nEuckr + 1) >= nEuckrSize)
            {
                break;
            }

            wcJohap = str_UcsToJohap(pwszUcs[n2ByteStr]);
            w2ByteStr = str_JohapToWansung(wcJohap);
            pszEuckr[nEuckr++] = (IMS_UCHAR)((w2ByteStr >> 8) & 0xFF);
            pszEuckr[nEuckr++] = (IMS_UCHAR)(w2ByteStr & 0xFF);
        }
    }
    pszEuckr[nEuckr] = 0;

    return nEuckr;
}

GLOBAL IMS_UINT32 IMS_EuckrToUcs(
        OUT IMS_WCHAR* pwszUcs, IN IMS_SIZE_T nUcsSize, IN CONST IMS_CHAR* pszEuckr)
{
    if (pwszUcs == IMS_NULL)
    {
        return 0;
    }

    pwszUcs[0] = 0;

    if (pszEuckr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nEuckr;
    IMS_SIZE_T nUcs;
    IMS_WCHAR w2ByteStr;
    IMS_WCHAR wcJohap;
    IMS_UINT32 nLenEuckr = IMS_StrLen(pszEuckr);

    for (nEuckr = 0, nUcs = 0; nEuckr < nLenEuckr && nUcs < nUcsSize; nUcs++)
    {
        if ((IMS_UCHAR)pszEuckr[nEuckr] < 0x80)
        {
            pwszUcs[nUcs] = (IMS_WCHAR)pszEuckr[nEuckr++];
        }
        else
        {
            w2ByteStr = (IMS_WCHAR)(pszEuckr[nEuckr] * 0x100) +
                    (IMS_WCHAR)(pszEuckr[nEuckr + 1] & 0xFF);
            wcJohap = str_WansungToJohap(w2ByteStr);
            pwszUcs[nUcs] = str_JohapToUcs(wcJohap);
            nEuckr += 2;
        }
    }
    pwszUcs[nUcs] = 0;

    return nUcs;
}

LOCAL IMS_SINT32 str_SkipAtoI(CONST IMS_WCHAR** ppwszS)
{
    IMS_SINT32 nNumber = 0;

    while (IS_DIGIT(**ppwszS))
    {
        nNumber = nNumber * 10 + *((*ppwszS)++) - '0';
    }

    return nNumber;
}

LOCAL IMS_WCHAR* str_ParseNumber(OUT IMS_WCHAR* pwszStr, IN IMS_SLONG nNum, IN IMS_SINT32 nBase,
        IN IMS_SINT32 nSize, IN IMS_SINT32 nPrecision, IN IMS_SINT32 nType)
{
    const IMS_WCHAR* pszDigits;

    if (nType & SU_LARGE)
    {
        pszDigits = stUpperDigits;
    }
    else
    {
        pszDigits = stLowerDigits;
    }

    if (nType & SU_LEFT)
    {
        nType &= ~SU_ZEROPAD;
    }

    if (nBase < 2 || nBase > 36)
    {
        return 0;
    }

    IMS_WCHAR cC = (nType & SU_ZEROPAD) ? '0' : ' ';
    IMS_WCHAR cSign = 0;

    if (nType & SU_SIGN)
    {
        if (nNum < 0)
        {
            cSign = '-';
            nNum = -nNum;
            nSize--;
        }
        else if (nType & SU_PLUS)
        {
            cSign = '+';
            nSize--;
        }
        else if (nType & SU_SPACE)
        {
            cSign = ' ';
            nSize--;
        }
    }

    if (nType & SU_SPECIAL)
    {
        if (nBase == 16)
        {
            nSize -= 2;
        }
        else if (nBase == 8)
        {
            nSize--;
        }
    }

    IMS_SINT32 nIndex = 0;
    IMS_WCHAR szTmp[66];

    if (nNum == 0)
    {
        szTmp[nIndex++] = '0';
    }
    else
    {
        while (nNum != 0)
        {
            szTmp[nIndex++] = pszDigits[((IMS_ULONG)nNum) % (unsigned)nBase];
            nNum = ((IMS_ULONG)nNum) / (unsigned)nBase;
        }
    }

    if (nIndex > nPrecision)
    {
        nPrecision = nIndex;
    }

    nSize -= nPrecision;

    if (!(nType & (SU_ZEROPAD + SU_LEFT)))
    {
        while (nSize-- > 0)
        {
            *pwszStr++ = ' ';
        }
    }

    if (cSign)
    {
        *pwszStr++ = cSign;
    }
    if (nType & SU_SPECIAL)
    {
        if (nBase == 8)
        {
            *pwszStr++ = '0';
        }
        else if (nBase == 16)
        {
            *pwszStr++ = '0';
            *pwszStr++ = pszDigits[33];
        }
    }

    if (!(nType & SU_LEFT))
    {
        while (nSize-- > 0)
        {
            *pwszStr++ = cC;
        }
    }

    while (nIndex < nPrecision--)
    {
        *pwszStr++ = '0';
    }

    while (nIndex-- > 0)
    {
        *pwszStr++ = szTmp[nIndex];
    }

    while (nSize-- > 0)
    {
        *pwszStr++ = ' ';
    }

    return pwszStr;
}

LOCAL IMS_SINT32 str_UniVsprintf(
        OUT IMS_WCHAR* pwszBuf, IN CONST IMS_WCHAR* pwszFmt, IN va_list args)
{
    IMS_SINT32 nLen;
    IMS_ULONG nNum;
    IMS_SINT32 nIndex;
    IMS_SINT32 nBase;
    IMS_WCHAR* pwszStr;
    IMS_SINT32 nFlags;       // nFlags to str_ParseNumber()
    IMS_SINT32 nFieldWidth;  // width of output field
    IMS_SINT32 nPrecision;   // min. # of digits for integers; max number of chars for from string
    IMS_SINT32 nQualifier;   // 'h', 'l', or 'L' for integer fields
    const IMS_WCHAR* pszS;
    const IMS_WCHAR NULLSTR[] = {(IMS_WCHAR)'<', (IMS_WCHAR)'N', (IMS_WCHAR)'U', (IMS_WCHAR)'L',
            (IMS_WCHAR)'L', (IMS_WCHAR)'>'};

    for (pwszStr = pwszBuf; *pwszFmt; ++pwszFmt)
    {
        if (*pwszFmt != '%')
        {
            *pwszStr++ = *pwszFmt;
            continue;
        }

        /* process nFlags */
        nFlags = 0;
    JP_REPEAT:
        ++pwszFmt; /* this also skips first '%' */

        switch (*pwszFmt)
        {
            case '-':
                nFlags |= SU_LEFT;
                goto JP_REPEAT;
            case '+':
                nFlags |= SU_PLUS;
                goto JP_REPEAT;
            case ' ':
                nFlags |= SU_SPACE;
                goto JP_REPEAT;
            case '#':
                nFlags |= SU_SPECIAL;
                goto JP_REPEAT;
            case '0':
                nFlags |= SU_ZEROPAD;
                goto JP_REPEAT;
        }

        /* get field width */
        nFieldWidth = -1;
        if (IS_DIGIT(*pwszFmt))
        {
            nFieldWidth = str_SkipAtoI(&pwszFmt);
        }
        else if (*pwszFmt == '*')
        {
            ++pwszFmt;
            /* it's the next argument */
            nFieldWidth = va_arg(args, IMS_SINT32);
            if (nFieldWidth < 0)
            {
                nFieldWidth = -nFieldWidth;
                nFlags |= SU_LEFT;
            }
        }

        /* get the nPrecision */
        nPrecision = -1;
        if (*pwszFmt == '.')
        {
            ++pwszFmt;
            if (IS_DIGIT(*pwszFmt))
            {
                nPrecision = str_SkipAtoI(&pwszFmt);
            }
            else if (*pwszFmt == '*')
            {
                ++pwszFmt;
                /* it's the next argument */
                nPrecision = va_arg(args, IMS_SINT32);
            }

            if (nPrecision < 0)
            {
                nPrecision = 0;
            }
        }

        /* get the conversion nQualifier */
        nQualifier = -1;
        if (*pwszFmt == 'h' || *pwszFmt == 'l' || *pwszFmt == 'L')
        {
            nQualifier = *pwszFmt;
            ++pwszFmt;
        }

        /* default nBase */
        nBase = 10;

        switch (*pwszFmt)
        {
            case 'c':
                if (!(nFlags & SU_LEFT))
                {
                    while (--nFieldWidth > 0)
                    {
                        *pwszStr++ = ' ';
                    }
                }

                *pwszStr++ = (IMS_WCHAR)va_arg(args, IMS_SINT32);

                while (--nFieldWidth > 0)
                {
                    *pwszStr++ = ' ';
                }
                continue;

            case 's':
                pszS = va_arg(args, IMS_WCHAR*);

                if (!pszS)
                {
                    pszS = NULLSTR;
                }

                nLen = IMS_UcStrLen((IMS_WCHAR*)pszS);
                nLen = (nLen > nPrecision) ? nLen : nPrecision;

                if (!(nFlags & SU_LEFT))
                {
                    while (nLen < nFieldWidth--)
                    {
                        *pwszStr++ = ' ';
                    }
                }

                for (nIndex = 0; nIndex < nLen; ++nIndex)
                {
                    *pwszStr++ = (IMS_WCHAR)*pszS++;
                }

                while (nLen < nFieldWidth--)
                {
                    *pwszStr++ = ' ';
                }
                continue;

            case 'p':
                if (nFieldWidth == -1)
                {
                    nFieldWidth = 2 * sizeof(void*);
                    nFlags |= SU_ZEROPAD;
                }
                pwszStr = str_ParseNumber(pwszStr, (IMS_ULONG)va_arg(args, void*), 16, nFieldWidth,
                        nPrecision, nFlags);
                continue;

            case 'n':
                if (nQualifier == 'l')
                {
                    IMS_SLONG* ip = va_arg(args, IMS_SLONG*);
                    *ip = (pwszStr - pwszBuf);
                }
                else
                {
                    IMS_SINT32* ip = va_arg(args, IMS_SINT32*);
                    *ip = (pwszStr - pwszBuf);
                }
                continue;

                /* integer number formats - set up the nFlags and "break" */
            case 'o':
                nBase = 8;
                break;

            case 'X':
                nFlags |= SU_LARGE;
                __IMS_FALLTHROUGH__
            case 'x':
                nBase = 16;
                break;

            case 'd':
            case 'i':
                nFlags |= SU_SIGN;
                __IMS_FALLTHROUGH__
            case 'u':
                break;
            case 'f':
                pwszStr = str_MakeFloat(
                        pwszStr, va_arg(args, IMS_DOUBLE), nFieldWidth, nPrecision, nFlags);
                continue;
            default:
                if (*pwszFmt != '%')
                {
                    *pwszStr++ = '%';
                }

                if (*pwszFmt)
                {
                    *pwszStr++ = *pwszFmt;
                }
                else
                {
                    --pwszFmt;
                }
                continue;
        }

        if (nQualifier == 'l')
        {
            nNum = va_arg(args, IMS_ULONG);
        }
        else if (nQualifier == 'h')
        {
            nNum = (IMS_WCHAR)va_arg(args, IMS_SINT32);

            if (nFlags & SU_SIGN)
            {
                nNum = (IMS_SINT16)nNum;
            }
        }
        else if (nFlags & SU_SIGN)
        {
            nNum = va_arg(args, IMS_SINT32);
        }
        else
        {
            nNum = va_arg(args, IMS_UINT32);
        }

        pwszStr = str_ParseNumber(pwszStr, nNum, nBase, nFieldWidth, nPrecision, nFlags);
    }

    *pwszStr = '\0';
    return pwszStr - pwszBuf;
}

LOCAL IMS_WCHAR* str_MakeFloat(OUT IMS_WCHAR* pwszStr, IN IMS_DOUBLE dValue,
        IN IMS_SINT32 nFieldWidth, IN IMS_SINT32 nPrecision, IN IMS_SINT32 nFlags)
{
    IMS_WCHAR cSignValue, szIConvert[100], szFConvert[100];
    IMS_SINT32 nZPadLen, nPadLen, nCpas, nIPlace, nFPlace, nIndex, nTempPrec = 0;
    IMS_DOUBLE nUFValue, nIntPart = 0, nFracPart, nPowPrec, nTemp;

    nIPlace = 0;
    nFPlace = 0;
    nCpas = (nFlags & SU_LARGE) ? IMS_TRUE : IMS_FALSE;

    if (dValue < 0)
    {
        nUFValue = -dValue;
        cSignValue = (IMS_WCHAR)'-';
    }
    else
    {
        nUFValue = dValue;

        if (nFlags & SU_PLUS)
        {
            cSignValue = (IMS_WCHAR)'+';
        }
        else
        {
            if (nFlags & SU_SPACE)
            {
                cSignValue = (IMS_WCHAR)' ';
            }
            else
            {
                cSignValue = 0;
            }
        }
    }

    if (nPrecision < 0)
    {
        nPrecision = 6;
    }
    else if (nPrecision > 9)
    {
        nTempPrec = nPrecision;
        nPrecision = 9;
    }

    nPowPrec = str_Pow10(nPrecision);
    nFracPart = str_Round(str_Modf(nUFValue, &nIntPart) * nPowPrec);

    if (nFracPart >= nPowPrec)
    {
        nIntPart += 1.0;
        nFracPart -= nPowPrec;
    }

    do
    {
        nTemp = nIntPart * 0.1;
        str_Modf(nTemp, &nIntPart);
        nIndex = (IMS_SINT32)((nTemp - nIntPart + 0.05) * 10.0);
        szIConvert[nIPlace++] = (nCpas ? stUpperDigits : stLowerDigits)[nIndex];
    } while (nIntPart && (nIPlace < 99));

    szIConvert[nIPlace] = 0;

    if (nFracPart)
    {
        do
        {
            nTemp = nFracPart * 0.1;
            str_Modf(nTemp, &nFracPart);
            nIndex = (IMS_SINT32)((nTemp - nFracPart + 0.05) * 10.0);
            szFConvert[nFPlace++] = (nCpas ? stUpperDigits : stLowerDigits)[nIndex];
        } while (nFracPart && (nFPlace < 99));
    }

    szFConvert[nFPlace] = 0;
    nPadLen = ((nFieldWidth == -1) ? 0 : nFieldWidth) - nIPlace -
            (nTempPrec > nPrecision ? nTempPrec : nPrecision) - 1 - ((cSignValue) ? 1 : 0);
    nZPadLen = (nTempPrec > nPrecision ? nTempPrec : nPrecision) - nFPlace;

    if (nZPadLen < 0)
    {
        nZPadLen = 0;
    }

    if (nPadLen < 0)
    {
        nPadLen = 0;
    }

    if (!(nFlags & SU_PLUS))
    {
        nPadLen = -nPadLen;
    }

    if ((nFlags & SU_ZEROPAD) && (nPadLen > 0))
    {
        if (cSignValue)
        {
            *pwszStr++ = (IMS_WCHAR)cSignValue;
            --nPadLen;
            cSignValue = 0;
        }

        while (nPadLen > 0)
        {
            *pwszStr++ = (IMS_WCHAR)'0';
            --nPadLen;
        }
    }

    while (nPadLen > 0)
    {
        *pwszStr++ = (IMS_WCHAR)' ';
        --nPadLen;
    }

    if (cSignValue)
    {
        *pwszStr++ = (IMS_WCHAR)cSignValue;
    }

    while (nIPlace > 0)
    {
        *pwszStr++ = (IMS_WCHAR)szIConvert[--nIPlace];
    }

    if (nPrecision > 0)
    {
        *pwszStr++ = (IMS_WCHAR)'.';

        while (nFPlace > 0)
        {
            *pwszStr++ = (IMS_WCHAR)szFConvert[--nFPlace];
        }
    }

    while (nZPadLen > 0)
    {
        *pwszStr++ = (IMS_WCHAR)'0';
        --nZPadLen;
    }

    while (nPadLen < 0)
    {
        *pwszStr++ = (IMS_WCHAR)' ';
        ++nPadLen;
    }

    return pwszStr;
}

LOCAL IMS_DOUBLE str_Round(IN IMS_DOUBLE dValue)
{
    IMS_SLONG nIntPart = (IMS_SLONG)dValue;

    dValue = dValue - (IMS_DOUBLE)nIntPart;

    if (dValue >= 0.5)
    {
        nIntPart++;
    }

    return (IMS_DOUBLE)nIntPart;
}

LOCAL IMS_DOUBLE str_Pow10(IN IMS_SINT32 nExp)
{
    IMS_DOUBLE dResult = 1.0;

    while (nExp--)
    {
        dResult *= 10.0;
    }

    return dResult;
}

LOCAL IMS_DOUBLE str_Modf(IN IMS_DOUBLE dFrac, OUT IMS_DOUBLE* pdPtr)
{
    IMS_SINT32 nIndex;
    IMS_SLONG lIdx;
    IMS_DOUBLE xIdx = dFrac;
    IMS_DOUBLE fIdx = 1.0;

    for (nIndex = 0; nIndex < 100; nIndex++)
    {
        lIdx = (IMS_SLONG)xIdx;

        if ((IMS_DOUBLE)lIdx <= (xIdx + 1.0) && (IMS_DOUBLE)lIdx >= (xIdx - 1.0))
        {
            break;
        }

        xIdx *= 0.1;
        fIdx *= 10.0;
    }

    if (nIndex == 100)
    {
        (*pdPtr) = 0.0;
        return 0.0;
    }

    if (nIndex != 0)
    {
        IMS_DOUBLE nIdx2 = 0;
        IMS_DOUBLE dRet = str_Modf(dFrac - (IMS_DOUBLE)lIdx * fIdx, &nIdx2);

        (*pdPtr) = (IMS_DOUBLE)lIdx * fIdx + (IMS_DOUBLE)nIdx2;

        return dRet;
    }

    (*pdPtr) = (IMS_DOUBLE)lIdx;

    return xIdx - (*pdPtr);
}

/* Convert a character from UTF-8 string to UCS-2 word.
The method returns UTF-8's byte size to make an UCS-2 character.
*/
LOCAL IMS_UINT32 str_Utf8ToUcs(OUT IMS_WCHAR* pwszUcs, IN CONST IMS_CHAR* pszUtf8)
{
    IMS_UINT32 nLen;

    if ((*pszUtf8 & 0xf8) == 0xf0) /* 11110XXXX */
    {
        nLen = 4;
    }
    else if ((*pszUtf8 & 0xf0) == 0xe0) /* 1110XXXXX */
    {
        nLen = 3;
    }
    else if ((*pszUtf8 & 0xe0) == 0xc0) /* 110XXXXXX */
    {
        nLen = 2;
    }
    else if ((*pszUtf8 & 0x80) == 0x00) /* 0XXXXXXXX */
    {
        nLen = 1;
    }
    else /* Error */
    {
        nLen = 0;
    }

    switch (nLen)
    {
        case 1:
            *pwszUcs = (*pszUtf8);
            break;
        case 2:
            *pwszUcs = (((*pszUtf8) & 0x1f) << 6) /* 110XXXXX */
                    | ((*(pszUtf8 + 1)) & 0x3f);  /* 10XXXXXX */
            break;
        case 3:
            *pwszUcs = (((*pszUtf8) & 0x0f) << 12)     /* 1110XXXX */
                    | (((*(pszUtf8 + 1)) & 0x3f) << 6) /* 10XXXXXX */
                    | ((*(pszUtf8 + 2)) & 0x3f);       /* 10XXXXXX */
            break;
        case 4:
            *pwszUcs = (((*pszUtf8) & 0x07) << 18)      /* 11110XXX */
                    | (((*(pszUtf8 + 1)) & 0x3f) << 12) /* 10XXXXXX */
                    | (((*(pszUtf8 + 2)) & 0x3f) << 6)  /* 10XXXXXX */
                    | ((*(pszUtf8 + 3)) & 0x3f);        /* 10XXXXXX */
            break;
        default:
            return 0;
    }

    return nLen;
}

/* Converts a character from UCS-2 word to UTF-8 string.
The method returns the length of UTF-8 srtring.
*/
LOCAL IMS_UINT32 str_UcsToUtf8(OUT IMS_CHAR* pszUtf8, IN IMS_WCHAR wcUcs)
{
    IMS_UINT32 nLen;
    IMS_UINT32 ch = wcUcs;

    if (ch < 0x00000080)
    {
        nLen = 1;
    }
    else if (ch < 0x00000800)
    {
        nLen = 2;
    }
    else if (ch < 0x00010000)
    {
        nLen = 3;
    }
    else if (ch < 0x00110000)
    {
        nLen = 4;
    }
    else
    {
        nLen = 0;
    }

    switch (nLen)
    {
        case 1:
            *(pszUtf8) = (IMS_CHAR)ch;
            break;
        case 2:
            *(pszUtf8) = 0xc0 | ((ch >> 6) & 0x1f);
            *(pszUtf8 + 1) = 0x80 | ((ch >> 0) & 0x3f);
            break;
        case 3:
            *(pszUtf8) = 0xe0 | ((ch >> 12) & 0x0f);
            *(pszUtf8 + 1) = 0x80 | ((ch >> 6) & 0x3f);
            *(pszUtf8 + 2) = 0x80 | ((ch >> 0) & 0x3f);
            break;
        case 4:
            *(pszUtf8) = 0xf0 | ((ch >> 18) & 0x07);
            *(pszUtf8 + 1) = 0x80 | ((ch >> 12) & 0x3f);
            *(pszUtf8 + 2) = 0x80 | ((ch >> 6) & 0x3f);
            *(pszUtf8 + 3) = 0x80 | ((ch >> 0) & 0x3f);
            break;
        default:
            return 0;
    }

    return nLen;
}

/* Convert string from JoHap to Wansung type.
 */
LOCAL IMS_WCHAR str_JohapToWansung(IN IMS_WCHAR wcJohap)
{
    switch (wcJohap)
    {
        case 0x8441:
            return 0x2000;  // '  '
        case 0x8841:
            return 0xA4A1;  // '��'
        case 0x8C41:
            return 0xA4A2;  // '��'
        case 0x9041:
            return 0xA4A4;  // '��'
        case 0x9441:
            return 0xA4A7;  // '��'
        case 0x9841:
            return 0xA4A8;  // '��'
        case 0x9C41:
            return 0xA4A9;  // '��'
        case 0xA041:
            return 0xA4B1;  // '��'
        case 0xA441:
            return 0xA4B2;  // '��'
        case 0xA841:
            return 0xA4B3;  // '��'
        case 0xAC41:
            return 0xA4B5;  // '��'
        case 0xB041:
            return 0xA4B6;  // '��'
        case 0xB441:
            return 0xA4B7;  // '��'
        case 0xB841:
            return 0xA4B8;  // '��'
        case 0xBC41:
            return 0xA4B9;  // '��'
        case 0xC041:
            return 0xA4BA;  // '��'
        case 0xC441:
            return 0xA4BB;  // '��'
        case 0xC841:
            return 0xA4BC;  // '��'
        case 0xCC41:
            return 0xA4BD;  // '��'
        case 0xD041:
            return 0xA4BE;  // '��'

        case 0x8461:
            return 0xA4BF;  // '��'
        case 0x8481:
            return 0xA4C0;  // '��'
        case 0x84A1:
            return 0xA4C1;  // '��'
        case 0x84C1:
            return 0xA4C2;  // '��'
        case 0x84E1:
            return 0xA4C3;  // '��'
        case 0x8541:
            return 0xA4C4;  // '��'
        case 0x8561:
            return 0xA4C5;  // '��'
        case 0x8581:
            return 0xA4C6;  // '��'
        case 0x85A1:
            return 0xA4C7;  // '��'
        case 0x85C1:
            return 0xA4C8;  // '��'
        case 0x85E1:
            return 0xA4C9;  // '��'
        case 0x8641:
            return 0xA4CA;  // '��'
        case 0x8661:
            return 0xA4CB;  // '��'
        case 0x8681:
            return 0xA4CC;  // '��'
        case 0x86A1:
            return 0xA4CD;  // '��'
        case 0x86C1:
            return 0xA4CE;  // '��'
        case 0x86E1:
            return 0xA4CF;  // '��'
        case 0x8741:
            return 0xA4D0;  // '��'
        case 0x8761:
            return 0xA4D1;  // '��'
        case 0x8781:
            return 0xA4D2;  // '��'
        case 0x87A1:
            return 0xA4D3;  // '��'
    }

    IMS_UINT16 nMinI = 0;
    IMS_UINT16 nMaxI = 2349;
    IMS_UINT16 nIndex;
    IMS_UINT16 nCode;

    while (IMS_TRUE)
    {
        nIndex = (nMinI + nMaxI) / 2;
        nCode = stJohapTable[nIndex];

        if (nCode == wcJohap)
        {
            break;
        }

        if (nMinI >= nMaxI)
        {
            return 0xFFFF;
        }

        if (nCode < wcJohap)
        {
            nMinI = nIndex + 1;
        }
        else
        {
            nMaxI = nIndex - 1;
        }
    }

    {
        IMS_UINT8 nHigh = (nIndex / 94) + 0xB0;
        IMS_UINT8 nLow = (nIndex % 94) + 0xA1;

        nCode = nHigh * 256 + nLow;
    }

    return nCode;
}

/* Convert string from Wansung to JoHap type. */
LOCAL IMS_WCHAR str_WansungToJohap(IN IMS_WCHAR wcWansung)
{
    switch (wcWansung)
    {
        case 0x2000:
            return 0x8441;  // '  '
        case 0xA4A1:
            return 0x8841;  // '��'
        case 0xA4A2:
            return 0x8C41;  // '��'
        case 0xA4A4:
            return 0x9041;  // '��'
        case 0xA4A7:
            return 0x9441;  // '��'
        case 0xA4A8:
            return 0x9841;  // '��'
        case 0xA4A9:
            return 0x9C41;  // '��'
        case 0xA4B1:
            return 0xA041;  // '��'
        case 0xA4B2:
            return 0xA441;  // '��'
        case 0xA4B3:
            return 0xA841;  // '��'
        case 0xA4B5:
            return 0xAC41;  // '��'
        case 0xA4B6:
            return 0xB041;  // '��'
        case 0xA4B7:
            return 0xB441;  // '��'
        case 0xA4B8:
            return 0xB841;  // '��'
        case 0xA4B9:
            return 0xBC41;  // '��'
        case 0xA4BA:
            return 0xC041;  // '��'
        case 0xA4BB:
            return 0xC441;  // '��'
        case 0xA4BC:
            return 0xC841;  // '��'
        case 0xA4BD:
            return 0xCC41;  // '��'
        case 0xA4BE:
            return 0xD041;  // '��'

        case 0xA4BF:
            return 0x8461;  // '��'
        case 0xA4C0:
            return 0x8481;  // '��'
        case 0xA4C1:
            return 0x84A1;  // '��'
        case 0xA4C2:
            return 0x84C1;  // '��'
        case 0xA4C3:
            return 0x84E1;  // '��'
        case 0xA4C4:
            return 0x8541;  // '��'
        case 0xA4C5:
            return 0x8561;  // '��'
        case 0xA4C6:
            return 0x8581;  // '��'
        case 0xA4C7:
            return 0x85A1;  // '��'
        case 0xA4C8:
            return 0x85C1;  // '��'
        case 0xA4C9:
            return 0x85E1;  // '��'
        case 0xA4CA:
            return 0x8641;  // '��'
        case 0xA4CB:
            return 0x8661;  // '��'
        case 0xA4CC:
            return 0x8681;  // '��'
        case 0xA4CD:
            return 0x86A1;  // '��'
        case 0xA4CE:
            return 0x86C1;  // '��'
        case 0xA4CF:
            return 0x86E1;  // '��'
        case 0xA4D0:
            return 0x8741;  // '��'
        case 0xA4D1:
            return 0x8761;  // '��'
        case 0xA4D2:
            return 0x8781;  // '��'
        case 0xA4D3:
            return 0x87A1;  // '��'
    }

    IMS_UINT8 nHigh = (IMS_CHAR)(wcWansung >> 8) - 0xB0;
    IMS_UINT8 nLow = (IMS_CHAR)wcWansung - 0xA1;
    IMS_UINT16 nIndex = (IMS_UINT8)nHigh * 94 + nLow;

    if (nIndex < 2350)
    {
        return stJohapTable[nIndex];
    }

    return 0x0000;
}

LOCAL IMS_WCHAR str_JohapToUcs(IN IMS_WCHAR wcJohap)
{
    switch (wcJohap)
    {
        // case 0x8441 : return 0x2000;        // '  '
        case 0x8841:
            return 0x3131;  // '��'
        case 0x8C41:
            return 0x3132;  // '��'
        case 0x9041:
            return 0x3134;  // '��'
        case 0x9441:
            return 0x3137;  // '��'
        case 0x9841:
            return 0x3138;  // '��'
        case 0x9C41:
            return 0x3139;  // '��'
        case 0xA041:
            return 0x3141;  // '��'
        case 0xA441:
            return 0x3142;  // '��'
        case 0xA841:
            return 0x3143;  // '��'
        case 0xAC41:
            return 0x3145;  // '��'
        case 0xB041:
            return 0x3146;  // '��'
        case 0xB441:
            return 0x3147;  // '��'
        case 0xB841:
            return 0x3148;  // '��'
        case 0xBC41:
            return 0x3149;  // '��'
        case 0xC041:
            return 0x314A;  // '��'
        case 0xC441:
            return 0x314B;  // '��'
        case 0xC841:
            return 0x314C;  // '��'
        case 0xCC41:
            return 0x314D;  // '��'
        case 0xD041:
            return 0x314E;  // '��'

        case 0x8461:
            return 0x314F;  // '��'
        case 0x8481:
            return 0x3150;  // '��'
        case 0x84A1:
            return 0x3151;  // '��'
        case 0x84C1:
            return 0x3152;  // '��'
        case 0x84E1:
            return 0x3153;  // '��'
        case 0x8541:
            return 0x3154;  // '��'
        case 0x8561:
            return 0x3155;  // '��'
        case 0x8581:
            return 0x3156;  // '��'
        case 0x85A1:
            return 0x3157;  // '��'
        case 0x85C1:
            return 0x3158;  // '��'
        case 0x85E1:
            return 0x3159;  // '��'
        case 0x8641:
            return 0x315A;  // '��'
        case 0x8661:
            return 0x315B;  // '��'
        case 0x8681:
            return 0x315C;  // '��'
        case 0x86A1:
            return 0x315D;  // '��'
        case 0x86C1:
            return 0x315E;  // '��'
        case 0x86E1:
            return 0x315F;  // '��'
        case 0x8741:
            return 0x3160;  // '��'
        case 0x8761:
            return 0x3161;  // '��'
        case 0x8781:
            return 0x3162;  // '��'
        case 0x87A1:
            return 0x3163;  // '��'
    }

    IMS_WCHAR wcC = (wcJohap & 0x001F);

    if (wcC >= 19)
    {
        wcC -= 2;
    }
    else
    {
        wcC -= 1;
    }

    IMS_WCHAR wcB = ((wcJohap >> 5) & 0x001F);

    if (wcB >= 26)
    {
        wcB -= 9;
    }
    else if (wcB >= 18)
    {
        wcB -= 7;
    }
    else if (wcB >= 10)
    {
        wcB -= 5;
    }
    else
    {
        wcB -= 3;
    }

    IMS_WCHAR wcA = ((wcJohap >> 10) & 0x001F) - 2;
    IMS_WCHAR wcResult = (IMS_WCHAR)(0xAC00 + (wcA * 21 + wcB) * 28 + wcC);

    return wcResult;
}

LOCAL IMS_WCHAR str_UcsToJohap(IN IMS_WCHAR wUcs2)
{
    switch (wUcs2)
    {
        // return 0x8441; case 0x2000:        // '  '
        case 0x3131:
            return 0x8841;  // '��'
        case 0x3132:
            return 0x8C41;  // '��'
        case 0x3134:
            return 0x9041;  // '��'
        case 0x3137:
            return 0x9441;  // '��'
        case 0x3138:
            return 0x9841;  // '��'
        case 0x3139:
            return 0x9C41;  // '��'
        case 0x3141:
            return 0xA041;  // '��'
        case 0x3142:
            return 0xA441;  // '��'
        case 0x3143:
            return 0xA841;  // '��'
        case 0x3145:
            return 0xAC41;  // '��'
        case 0x3146:
            return 0xB041;  // '��'
        case 0x3147:
            return 0xB441;  // '��'
        case 0x3148:
            return 0xB841;  // '��'
        case 0x3149:
            return 0xBC41;  // '��'
        case 0x314A:
            return 0xC041;  // '��'
        case 0x314B:
            return 0xC441;  // '��'
        case 0x314C:
            return 0xC841;  // '��'
        case 0x314D:
            return 0xCC41;  // '��'
        case 0x314E:
            return 0xD041;  // '��'

        case 0x314F:
            return 0x8461;  // '��'
        case 0x3150:
            return 0x8481;  // '��'
        case 0x3151:
            return 0x84A1;  // '��'
        case 0x3152:
            return 0x84C1;  // '��'
        case 0x3153:
            return 0x84E1;  // '��'
        case 0x3154:
            return 0x8541;  // '��'
        case 0x3155:
            return 0x8561;  // '��'
        case 0x3156:
            return 0x8581;  // '��'
        case 0x3157:
            return 0x85A1;  // '��'
        case 0x3158:
            return 0x85C1;  // '��'
        case 0x3159:
            return 0x85E1;  // '��'
        case 0x315A:
            return 0x8641;  // '��'
        case 0x315B:
            return 0x8661;  // '��'
        case 0x315C:
            return 0x8681;  // '��'
        case 0x315D:
            return 0x86A1;  // '��'
        case 0x315E:
            return 0x86C1;  // '��'
        case 0x315F:
            return 0x86E1;  // '��'
        case 0x3160:
            return 0x8741;  // '��'
        case 0x3161:
            return 0x8761;  // '��'
        case 0x3162:
            return 0x8781;  // '��'
        case 0x3163:
            return 0x87A1;  // '��'
    }

    IMS_WCHAR wcBase = wUcs2 - 0xAC00;
    IMS_WCHAR wcA = (wcBase / (21 * 28)) + 2;

    wcBase = wcBase % (21 * 28);

    IMS_WCHAR wcB = (wcBase / 28);

    if (wcB >= 17)
    {
        wcB += 9;
    }
    else if (wcB >= 11)
    {
        wcB += 7;
    }
    else if (wcB >= 5)
    {
        wcB += 5;
    }
    else
    {
        wcB += 3;
    }

    IMS_WCHAR wcC = (wcBase % 28);

    if (wcC >= 17)
    {
        wcC += 2;
    }
    else
    {
        wcC += 1;
    }

    IMS_WCHAR wcResult = 0x8000 | (0x7C00 & (wcA << 10)) | (0x03E0 & (wcB << 5)) | (0x001F & wcC);

    return wcResult;
}
