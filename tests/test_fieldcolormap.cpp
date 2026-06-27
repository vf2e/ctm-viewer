#include "FieldColorMap.h"

#include <QtTest>

namespace {

bool rgbEqual(const unsigned char a[3], const unsigned char b[3])
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

unsigned char expectedBrainGray()
{
    return static_cast<unsigned char>(std::lround(FieldColorMap::kBrainGray * 255.0));
}

} // namespace

class FieldColorMapTest : public QObject
{
    Q_OBJECT

private slots:
    void brainGrayRgb_matchesConstant();
    void setRange_enforcesMinimumSpan();
    void colorForValue_belowMin_returnsBrainGray();
    void colorForValue_atMin_returnsBrainGray();
    void colorForValue_aboveMax_clampsToHeatMax();
    void colorForValue_midRange_differsFromGray();
    void colorForNormalized_endpoints_areStable();
    void colorForNormalized_isMonotonicInRedNearHighEnd();
    void defaultRange_matchesProductDefaults();
};

void FieldColorMapTest::brainGrayRgb_matchesConstant()
{
    unsigned char rgb[3] = {};
    FieldColorMap::brainGrayRgb(rgb);

    const unsigned char gray = expectedBrainGray();
    QCOMPARE(rgb[0], gray);
    QCOMPARE(rgb[1], gray);
    QCOMPARE(rgb[2], gray);
    QCOMPARE(gray, static_cast<unsigned char>(148));
}

void FieldColorMapTest::setRange_enforcesMinimumSpan()
{
    FieldColorMap map;
    map.setRange(80.0, 50.0);

    QCOMPARE(map.minValue, 80.0);
    QCOMPARE(map.maxValue, 81.0);
}

void FieldColorMapTest::colorForValue_belowMin_returnsBrainGray()
{
    FieldColorMap map;
    map.setRange(40.0, 150.0);

    unsigned char rgb[3] = {};
    unsigned char gray[3] = {};
    FieldColorMap::brainGrayRgb(gray);

    map.colorForValue(0.0, rgb);
    QVERIFY(rgbEqual(rgb, gray));

    map.colorForValue(39.9, rgb);
    QVERIFY(rgbEqual(rgb, gray));
}

void FieldColorMapTest::colorForValue_atMin_returnsBrainGray()
{
    FieldColorMap map;
    map.setRange(40.0, 150.0);

    unsigned char rgb[3] = {};
    unsigned char gray[3] = {};
    FieldColorMap::brainGrayRgb(gray);

    map.colorForValue(40.0, rgb);
    QVERIFY(rgbEqual(rgb, gray));
}

void FieldColorMapTest::colorForValue_aboveMax_clampsToHeatMax()
{
    FieldColorMap map;
    map.setRange(40.0, 150.0);

    unsigned char atMax[3] = {};
    unsigned char aboveMax[3] = {};
    map.colorForValue(150.0, atMax);
    map.colorForValue(999.0, aboveMax);

    QVERIFY(rgbEqual(atMax, aboveMax));
    QVERIFY(aboveMax[0] > 200);
    QVERIFY(aboveMax[2] < 80);
}

void FieldColorMapTest::colorForValue_midRange_differsFromGray()
{
    FieldColorMap map;
    map.setRange(40.0, 150.0);

    unsigned char rgb[3] = {};
    unsigned char gray[3] = {};
    FieldColorMap::brainGrayRgb(gray);

    map.colorForValue(95.0, rgb);
    QVERIFY(!rgbEqual(rgb, gray));
}

void FieldColorMapTest::colorForNormalized_endpoints_areStable()
{
    FieldColorMap map;

    unsigned char low[3] = {};
    unsigned char high[3] = {};
    map.colorForNormalized(0.0, low);
    map.colorForNormalized(1.0, high);

    unsigned char gray[3] = {};
    FieldColorMap::brainGrayRgb(gray);
    QVERIFY(rgbEqual(low, gray));
    QVERIFY(high[0] > 200);
    QVERIFY(high[2] < 80);
}

void FieldColorMapTest::colorForNormalized_isMonotonicInRedNearHighEnd()
{
    FieldColorMap map;

    unsigned char t70[3] = {};
    unsigned char t86[3] = {};
    unsigned char t100[3] = {};
    map.colorForNormalized(0.70, t70);
    map.colorForNormalized(0.86, t86);
    map.colorForNormalized(1.00, t100);

    QVERIFY(t70[0] <= t86[0]);
    QVERIFY(t86[0] <= t100[0]);
}

void FieldColorMapTest::defaultRange_matchesProductDefaults()
{
    FieldColorMap map;
    QCOMPARE(map.minValue, 40.0);
    QCOMPARE(map.maxValue, 150.0);
}

QTEST_APPLESS_MAIN(FieldColorMapTest)
#include "test_fieldcolormap.moc"
