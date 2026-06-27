#ifndef FIELDCOLORMAP_H
#define FIELDCOLORMAP_H

struct FieldColorMap
{
    static constexpr double kBrainGray = 0.58;

    double minValue = 40.0;
    double maxValue = 150.0;

    void setRange(double minValue, double maxValue);
    void colorForValue(double value, unsigned char rgb[3]) const;
    void colorForNormalized(double t, unsigned char rgb[3]) const;
    static void brainGrayRgb(unsigned char rgb[3]);

private:
    static void sampleHeatMap(double t, unsigned char rgb[3]);
};

#endif
