#include "FieldSimulator.h"

#include "EfieldBinaryWriter.h"
#include "TestPaths.h"
#include "VtkTestHelpers.h"

#include <QDir>
#include <QTemporaryDir>
#include <QtTest>

#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

class FieldSimulatorTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void loadBinary_missingFile_fails();
    void loadBinary_truncatedHeader_fails();
    void loadBinary_truncatedPayload_fails();
    void loadBinary_validFile_succeeds();
    void hasData_reflectsLoadState();
    void isCoilWithinRange_handlesNullAndDistance();
    void resetBrainColors_setsUniformGray();
    void applyToBrain_colorsVerticesWithFieldData();
    void applyToBrain_respectsIntensityScale();
    void applyToBrain_emptySamples_isNoOp();

private:
    QString m_samplePath;
    QTemporaryDir m_tempDir;
};

void FieldSimulatorTest::initTestCase()
{
    QVERIFY(m_tempDir.isValid());

    const std::vector<EfieldSample> samples = {
        {0.0f, 0.0f, 0.0f, 120.0f},
        {1.0f, 0.0f, 0.0f, 180.0f},
        {0.0f, 1.0f, 0.0f, 60.0f},
    };

    m_samplePath = m_tempDir.path() + QStringLiteral("/sample_efield.bin");
    QVERIFY(writeEfieldBinary(m_samplePath, samples));
}

void FieldSimulatorTest::loadBinary_missingFile_fails()
{
    FieldSimulator simulator;
    QString error;
    QVERIFY(!simulator.loadBinary(QStringLiteral("Z:/no_such_efield.bin"), &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!simulator.hasData());
}

void FieldSimulatorTest::loadBinary_truncatedHeader_fails()
{
    const QString path = m_tempDir.path() + QStringLiteral("/short.bin");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write(QByteArray(2, '\0')) == 2);
    file.close();

    FieldSimulator simulator;
    QString error;
    QVERIFY(!simulator.loadBinary(path, &error));
    QVERIFY(error.contains(QStringLiteral("无效")) || error.contains(QStringLiteral("不匹配")));
}

void FieldSimulatorTest::loadBinary_truncatedPayload_fails()
{
    const QString path = m_tempDir.path() + QStringLiteral("/bad_count.bin");
    QByteArray bytes(8, '\0');
    bytes[0] = 2;
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write(bytes) == bytes.size());
    file.close();

    FieldSimulator simulator;
    QString error;
    QVERIFY(!simulator.loadBinary(path, &error));
    QVERIFY(!error.isEmpty());
}

void FieldSimulatorTest::loadBinary_validFile_succeeds()
{
    FieldSimulator simulator;
    QString error;
    QVERIFY(simulator.loadBinary(m_samplePath, &error));
    QVERIFY(error.isEmpty());
    QVERIFY(simulator.hasData());
}

void FieldSimulatorTest::hasData_reflectsLoadState()
{
    FieldSimulator simulator;
    QVERIFY(!simulator.hasData());

    QVERIFY(simulator.loadBinary(m_samplePath));
    QVERIFY(simulator.hasData());
}

void FieldSimulatorTest::isCoilWithinRange_handlesNullAndDistance()
{
    QVERIFY(!FieldSimulator::isCoilWithinRange(nullptr, 200.0));

    vtkNew<vtkMatrix4x4> nearPose;
    nearPose->Identity();
    nearPose->SetElement(0, 3, 50.0);
    nearPose->SetElement(1, 3, 0.0);
    nearPose->SetElement(2, 3, 0.0);
    QVERIFY(FieldSimulator::isCoilWithinRange(nearPose.Get(), 200.0));
    QVERIFY(!FieldSimulator::isCoilWithinRange(nearPose.Get(), 10.0));

    vtkNew<vtkMatrix4x4> farPose;
    farPose->Identity();
    farPose->SetElement(0, 3, 300.0);
    QVERIFY(!FieldSimulator::isCoilWithinRange(farPose.Get(), 200.0));
}

void FieldSimulatorTest::resetBrainColors_setsUniformGray()
{
    auto brain = makeTrianglePolyData();
    FieldSimulator simulator;
    simulator.resetBrainColors(brain.Get());

    auto *colors = vtkUnsignedCharArray::SafeDownCast(brain->GetPointData()->GetScalars());
    QVERIFY(colors != nullptr);
    QCOMPARE(colors->GetNumberOfTuples(), brain->GetNumberOfPoints());

    const unsigned char gray = static_cast<unsigned char>(std::lround(FieldColorMap::kBrainGray * 255.0));
    for (vtkIdType i = 0; i < colors->GetNumberOfTuples(); ++i) {
        unsigned char rgb[3] = {};
        colors->GetTupleValue(i, rgb);
        QCOMPARE(rgb[0], gray);
        QCOMPARE(rgb[1], gray);
        QCOMPARE(rgb[2], gray);
    }
}

void FieldSimulatorTest::applyToBrain_colorsVerticesWithFieldData()
{
    const std::vector<EfieldSample> samples = {
        {0.0f, 0.0f, 0.0f, 200.0f},
        {0.5f, 0.0f, 0.0f, 180.0f},
        {0.0f, 0.5f, 0.0f, 160.0f},
    };
    const QString path = m_tempDir.path() + QStringLiteral("/localized_efield.bin");
    QVERIFY(writeEfieldBinary(path, samples));

    FieldSimulator simulator;
    QVERIFY(simulator.loadBinary(path));

    auto brain = makeGridPolyData(1, 10.0);
    vtkNew<vtkMatrix4x4> coilPose;
    coilPose->Identity();

    FieldColorMap colorMap;
    colorMap.setRange(40.0, 200.0);
    simulator.applyToBrain(brain.Get(), coilPose.Get(), 100.0, colorMap);

    auto *colors = vtkUnsignedCharArray::SafeDownCast(brain->GetPointData()->GetScalars());
    QVERIFY(colors != nullptr);
    QCOMPARE(colors->GetNumberOfTuples(), brain->GetNumberOfPoints());

    unsigned char rgb[3] = {};
    colors->GetTupleValue(0, rgb);
    unsigned char gray[3] = {};
    FieldColorMap::brainGrayRgb(gray);
    QVERIFY(rgb[0] != gray[0] || rgb[1] != gray[1] || rgb[2] != gray[2]);
}

void FieldSimulatorTest::applyToBrain_respectsIntensityScale()
{
    FieldSimulator simulator;
    QVERIFY(simulator.loadBinary(m_samplePath));

    auto brain = makeGridPolyData(2, 10.0);
    vtkNew<vtkMatrix4x4> coilPose;
    coilPose->Identity();

    FieldColorMap colorMap;
    colorMap.setRange(1.0, 255.0);

    auto brainFull = brain;
    auto brainHalf = makeGridPolyData(2, 10.0);

    simulator.applyToBrain(brainFull.Get(), coilPose.Get(), 100.0, colorMap);
    simulator.applyToBrain(brainHalf.Get(), coilPose.Get(), 50.0, colorMap);

    auto *fullColors = vtkUnsignedCharArray::SafeDownCast(brainFull->GetPointData()->GetScalars());
    auto *halfColors = vtkUnsignedCharArray::SafeDownCast(brainHalf->GetPointData()->GetScalars());
    QVERIFY(fullColors != nullptr);
    QVERIFY(halfColors != nullptr);

    bool foundDifference = false;
    for (vtkIdType i = 0; i < fullColors->GetNumberOfTuples(); ++i) {
        unsigned char fullRgb[3] = {};
        unsigned char halfRgb[3] = {};
        fullColors->GetTupleValue(i, fullRgb);
        halfColors->GetTupleValue(i, halfRgb);
        if (fullRgb[0] != halfRgb[0] || fullRgb[1] != halfRgb[1] || fullRgb[2] != halfRgb[2]) {
            foundDifference = true;
            break;
        }
    }
    QVERIFY(foundDifference);
}

void FieldSimulatorTest::applyToBrain_emptySamples_isNoOp()
{
    FieldSimulator simulator;
    auto brain = makeTrianglePolyData();
    vtkNew<vtkMatrix4x4> coilPose;
    coilPose->Identity();

    FieldColorMap colorMap;
    simulator.applyToBrain(brain.Get(), coilPose.Get(), 100.0, colorMap);
    QVERIFY(brain->GetPointData()->GetScalars() == nullptr);
}

QTEST_APPLESS_MAIN(FieldSimulatorTest)
#include "test_fieldsimulator.moc"
