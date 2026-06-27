#include "FieldSimulator.h"

#include "FieldColorMap.h"

#include <QFile>

#include <vtkFloatArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace {

constexpr double kGridSize = 10.0;

} // namespace

bool FieldSimulator::loadBinary(const QString &filePath, QString *errorMessage)
{
    m_samples.clear();
    m_grid.clear();
    m_gridBuilt = false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法打开电场数据: %1").arg(filePath);
        }
        return false;
    }

    const QByteArray bytes = file.readAll();
    if (bytes.size() < 4) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("电场数据文件无效");
        }
        return false;
    }

    const auto *data = reinterpret_cast<const unsigned char *>(bytes.constData());
    const quint32 count = static_cast<quint32>(data[0])
        | (static_cast<quint32>(data[1]) << 8)
        | (static_cast<quint32>(data[2]) << 16)
        | (static_cast<quint32>(data[3]) << 24);

    const int expectedSize = 4 + static_cast<int>(count) * 16;
    if (bytes.size() < expectedSize) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("电场数据长度不匹配");
        }
        return false;
    }

    m_samples.reserve(count);
    int offset = 4;
    for (quint32 i = 0; i < count; ++i) {
        Sample sample;
        std::memcpy(&sample.x, bytes.constData() + offset, 16);
        offset += 16;
        m_samples.push_back(sample);
    }

    return true;
}

FieldSimulator::Vec3 FieldSimulator::fieldLocalPosition(const Sample &sample)
{
    return {
        sample.x * 10.0,
        sample.z * 10.0,
        sample.y * 10.0,
    };
}

QString FieldSimulator::gridKey(int x, int y, int z)
{
    return QStringLiteral("%1,%2,%3").arg(x).arg(y).arg(z);
}

void FieldSimulator::buildGrid() const
{
    if (m_gridBuilt) {
        return;
    }

    m_grid.clear();
    m_grid.reserve(m_samples.size() / 4 + 1);

    for (const Sample &sample : m_samples) {
        const Vec3 pos = fieldLocalPosition(sample);
        const int gx = static_cast<int>(std::floor(pos.x / kGridSize));
        const int gy = static_cast<int>(std::floor(pos.y / kGridSize));
        const int gz = static_cast<int>(std::floor(pos.z / kGridSize));
        const std::string key = gridKey(gx, gy, gz).toStdString();
        m_grid[key].push_back({pos, sample.magnitude});
    }

    m_gridBuilt = true;
}

FieldSimulator::Vec3 FieldSimulator::transformPoint(vtkMatrix4x4 *matrix, const Vec3 &point)
{
    double output[4] = {0, 0, 0, 0};
    const double input[4] = {point.x, point.y, point.z, 1.0};
    matrix->MultiplyPoint(input, output);
    return {output[0], output[1], output[2]};
}

bool FieldSimulator::isCoilWithinRange(const vtkMatrix4x4 *coilPose, double maxDistanceMm)
{
    if (!coilPose) {
        return false;
    }

    const double distance = std::sqrt(coilPose->GetElement(0, 3) * coilPose->GetElement(0, 3)
                                    + coilPose->GetElement(1, 3) * coilPose->GetElement(1, 3)
                                    + coilPose->GetElement(2, 3) * coilPose->GetElement(2, 3));
    return distance <= maxDistanceMm;
}

void FieldSimulator::resetBrainColors(vtkPolyData *brain) const
{
    if (!brain || brain->GetNumberOfPoints() == 0) {
        return;
    }

    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    colors->SetNumberOfTuples(brain->GetNumberOfPoints());

    const unsigned char gray = static_cast<unsigned char>(std::lround(FieldColorMap::kBrainGray * 255.0));
    for (vtkIdType i = 0; i < brain->GetNumberOfPoints(); ++i) {
        colors->SetTuple3(i, gray, gray, gray);
    }

    brain->GetPointData()->SetScalars(colors.Get());
}

void FieldSimulator::applyToBrain(vtkPolyData *brain,
                                  const vtkMatrix4x4 *coilPose,
                                  double intensityPercent,
                                  const FieldColorMap &colorMap) const
{
    if (!brain || !coilPose || m_samples.empty()) {
        return;
    }

    buildGrid();

    vtkNew<vtkMatrix4x4> coilMatrix;
    coilMatrix->DeepCopy(coilPose);

    vtkNew<vtkTransform> fieldTransform;
    fieldTransform->SetMatrix(coilMatrix.Get());
    fieldTransform->RotateZ(90.0);

    vtkNew<vtkMatrix4x4> inverse;
    inverse->DeepCopy(fieldTransform->GetMatrix());
    inverse->Invert();

    vtkPoints *points = brain->GetPoints();
    if (!points) {
        return;
    }

    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    colors->SetNumberOfTuples(points->GetNumberOfPoints());

    const double scale = intensityPercent / 100.0;
    const double maxGridDistance = (kGridSize * std::sqrt(3.0));

    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
        double p[3];
        points->GetPoint(i, p);
        const Vec3 fieldVertex = transformPoint(inverse.Get(), {p[0], p[1], p[2]});

        const int gx = static_cast<int>(std::floor(fieldVertex.x / kGridSize));
        const int gy = static_cast<int>(std::floor(fieldVertex.y / kGridSize));
        const int gz = static_cast<int>(std::floor(fieldVertex.z / kGridSize));

        double weightSum = 0.0;
        double magnitudeSum = 0.0;

        for (int dx = 0; dx <= 1; ++dx) {
            for (int dy = 0; dy <= 1; ++dy) {
                for (int dz = 0; dz <= 1; ++dz) {
                    const std::string key = gridKey(gx + dx, gy + dy, gz + dz).toStdString();
                    const auto it = m_grid.find(key);
                    if (it == m_grid.end()) {
                        continue;
                    }

                    const Vec3 gridCenter = {
                        (gx + dx + 0.5) * kGridSize,
                        (gy + dy + 0.5) * kGridSize,
                        (gz + dz + 0.5) * kGridSize,
                    };

                    const double dxg = fieldVertex.x - gridCenter.x;
                    const double dyg = fieldVertex.y - gridCenter.y;
                    const double dzg = fieldVertex.z - gridCenter.z;
                    if (std::sqrt(dxg * dxg + dyg * dyg + dzg * dzg) > maxGridDistance) {
                        continue;
                    }

                    for (const GridPoint &gridPoint : it->second) {
                        const double ddx = fieldVertex.x - gridPoint.pos.x;
                        const double ddy = fieldVertex.y - gridPoint.pos.y;
                        const double ddz = fieldVertex.z - gridPoint.pos.z;
                        const double distance = std::sqrt(ddx * ddx + ddy * ddy + ddz * ddz);
                        if (distance > maxGridDistance) {
                            continue;
                        }

                        const double weight = 1.0 / (distance * distance + 4.0);
                        magnitudeSum += gridPoint.magnitude * weight;
                        weightSum += weight;
                    }
                }
            }
        }

        unsigned char rgb[3];
        if (weightSum <= 0.0) {
            FieldColorMap::brainGrayRgb(rgb);
        } else {
            const double mappedValue = (magnitudeSum / weightSum) * scale;
            colorMap.colorForValue(mappedValue, rgb);
        }
        colors->SetTuple3(i, rgb[0], rgb[1], rgb[2]);
    }

    brain->GetPointData()->SetScalars(colors.Get());
}
