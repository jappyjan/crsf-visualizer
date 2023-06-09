#pragma once

#include "animation-base.hpp"
#include "const.h"

class BlinkAnimation : public AnimationBase {
    public:
        BlinkAnimation(CRGB onColor, CRGB offColor, int intervalMs) {
            this->onColor = onColor;
            this->offColor = offColor;
            this->intervalMs = intervalMs;
            this->on = false;
        }
        void tick();

    private:
        CRGB offColor;
        CRGB onColor;
        bool on;
        int intervalMs;
};