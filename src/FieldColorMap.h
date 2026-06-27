#ifndef FIELDCOLORMAP_H
#define FIELDCOLORMAP_H

struct FieldColorMap
{
    double minValue = 40.0;
    double maxValue = 150.0;

    void setRange(double minValue, double maxValue);
    void colorForValue(double value, unsigned char rgb[3]) const;

private:
    static unsigned int lerpChannel(unsigned int a, unsigned int b, double t);
    static void hexToRgb(unsigned int hex, unsigned char rgb[3]);
    static void sampleGradient(double t, unsigned char rgb[3]);
};

#endif
