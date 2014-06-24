#include "animation.h"

QMap<QString, Animation> Animation::animations;

Animation::Animation()
    : id{0}, name{}
{
}
