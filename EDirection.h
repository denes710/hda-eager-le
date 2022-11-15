#ifndef RING_EDIRECTION_H
#define RING_EDIRECTION_H

namespace RING
{
    enum class EDirection
    {
        Right,
        Left,
        Both
    };

    inline EDirection GetInverse(EDirection p_direction)
    {
        switch (p_direction)
        {
            case EDirection::Right:
                return EDirection::Left;
            case EDirection::Left:
                return EDirection::Right;
            default:
                return EDirection::Both;
        }
        
        return EDirection::Both;
    }
}

#endif