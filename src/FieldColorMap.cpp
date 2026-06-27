#include "FieldColorMap.h"

#include <algorithm>
#include <cmath>

void FieldColorMap::setRange(double minValue, double maxValue)
{
    this->minValue = minValue;
    this->maxValue = std::max(minValue + 1.0, maxValue);
}

unsigned int FieldColorMap::lerpChannel(unsigned int a, unsigned int b, double t)
{
    return static_cast<unsigned int>(std::lround(a + (b - a) * t));
}

void FieldColorMap::hexToRgb(unsigned int hex, unsigned char rgb[3])
{
    rgb[0] = static_cast<unsigned char>((hex >> 16) & 0xff);
    rgb[1] = static_cast<unsigned char>((hex >> 8) & 0xff);
    rgb[2] = static_cast<unsigned char>(hex & 0xff);
}

void FieldColorMap::sampleGradient(double t, unsigned char rgb[3])
{
    t = std::clamp(t, 0.0, 1.0);

    struct Stop {
        double pos;
        unsigned int color;
    };

    static const Stop stops[] = {
        {0.0 / 6.0, 0x787878},
        {1.0 / 6.0, 0x00ffff},
        {2.0 / 6.0, 0xffff00},
        {3.0 / 6.0, 0xfadb14},
        {4.0 / 6.0, 0xf5222d},
        {5.0 / 6.0, 0x5c0011},
        {6.0 / 6.0, 0x5c0011},
    };

    for (int i = 1; i < 7; ++i) {
        if (t <= stops[i].pos) {
            const double span = stops[i].pos - stops[i - 1].pos;
            const double localT = span > 0.0 ? (t - stops[i - 1].pos) / span : 0.0;
            unsigned char c0[3];
            unsigned char c1[3];
            hexToRgb(stops[i - 1].color, c0);
            hexToRgb(stops[i].color, c1);
            rgb[0] = static_cast<unsigned char>(lerpChannel(c0[0], c1[0], localT));
            rgb[1] = static_cast<unsigned char>(lerpChannel(c0[1], c1[1], localT));
            rgb[2] = static_cast<unsigned char>(lerpChannel(c0[2], c1[2], localT));
            return;
        }
    }

    hexToRgb(stops[6].color, rgb);
}

void FieldColorMap::colorForValue(double value, unsigned char rgb[3]) const
{
    if (value <= minValue) {
        hexToRgb(0x787878, rgb);
        return;
    }

    const double t = (value - minValue) / (maxValue - minValue);
    sampleGradient(t, rgb);
}
