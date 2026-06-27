#include "FieldColorMap.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr unsigned int kBrainGrayHex = 0x949494;

void lerpRgb(const unsigned char a[3], const unsigned char b[3], double t, unsigned char rgb[3])
{
    t = std::clamp(t, 0.0, 1.0);
    rgb[0] = static_cast<unsigned char>(std::lround(a[0] + (b[0] - a[0]) * t));
    rgb[1] = static_cast<unsigned char>(std::lround(a[1] + (b[1] - a[1]) * t));
    rgb[2] = static_cast<unsigned char>(std::lround(a[2] + (b[2] - a[2]) * t));
}

} // namespace

void FieldColorMap::brainGrayRgb(unsigned char rgb[3])
{
    const unsigned char gray = static_cast<unsigned char>(std::lround(kBrainGray * 255.0));
    rgb[0] = gray;
    rgb[1] = gray;
    rgb[2] = gray;
}

void FieldColorMap::setRange(double minValue, double maxValue)
{
    this->minValue = minValue;
    this->maxValue = std::max(minValue + 1.0, maxValue);
}

void FieldColorMap::sampleHeatMap(double t, unsigned char rgb[3])
{
    t = std::clamp(t, 0.0, 1.0);

    struct Stop {
        double pos;
        unsigned int color;
    };

    static const Stop stops[] = {
        {0.00, kBrainGrayHex},
        {0.16, 0x5C8FD6},
        {0.34, 0x0088FF},
        {0.52, 0x00E5FF},
        {0.70, 0xB8FF40},
        {0.86, 0xFFFF00},
        {1.00, 0xFF2200},
    };

    constexpr int stopCount = static_cast<int>(sizeof(stops) / sizeof(stops[0]));

    if (t <= stops[0].pos) {
        brainGrayRgb(rgb);
        return;
    }

    for (int i = 1; i < stopCount; ++i) {
        if (t <= stops[i].pos) {
            unsigned char c0[3];
            unsigned char c1[3];
            const double span = stops[i].pos - stops[i - 1].pos;
            const double localT = span > 0.0 ? (t - stops[i - 1].pos) / span : 0.0;
            c0[0] = static_cast<unsigned char>((stops[i - 1].color >> 16) & 0xff);
            c0[1] = static_cast<unsigned char>((stops[i - 1].color >> 8) & 0xff);
            c0[2] = static_cast<unsigned char>(stops[i - 1].color & 0xff);
            c1[0] = static_cast<unsigned char>((stops[i].color >> 16) & 0xff);
            c1[1] = static_cast<unsigned char>((stops[i].color >> 8) & 0xff);
            c1[2] = static_cast<unsigned char>(stops[i].color & 0xff);
            lerpRgb(c0, c1, localT, rgb);
            return;
        }
    }

    rgb[0] = static_cast<unsigned char>((stops[stopCount - 1].color >> 16) & 0xff);
    rgb[1] = static_cast<unsigned char>((stops[stopCount - 1].color >> 8) & 0xff);
    rgb[2] = static_cast<unsigned char>(stops[stopCount - 1].color & 0xff);
}

void FieldColorMap::colorForNormalized(double t, unsigned char rgb[3]) const
{
    sampleHeatMap(t, rgb);
}

void FieldColorMap::colorForValue(double value, unsigned char rgb[3]) const
{
    if (value <= minValue) {
        brainGrayRgb(rgb);
        return;
    }

    const double span = maxValue - minValue;
    const double t = span > 0.0 ? std::clamp((value - minValue) / span, 0.0, 1.0) : 1.0;
    colorForNormalized(t, rgb);
}
