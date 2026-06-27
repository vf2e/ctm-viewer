#ifndef FIELDSIMULATOR_H
#define FIELDSIMULATOR_H

#include "FieldColorMap.h"

#include <QString>
#include <unordered_map>
#include <vector>

#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class FieldSimulator
{
public:
    struct Sample {
        float x;
        float y;
        float z;
        float magnitude;
    };

    struct Vec3 {
        double x;
        double y;
        double z;
    };

    bool loadBinary(const QString &filePath, QString *errorMessage = nullptr);
    bool hasData() const { return !m_samples.empty(); }

    void resetBrainColors(vtkPolyData *brain) const;
    void applyToBrain(vtkPolyData *brain,
                      const vtkMatrix4x4 *coilPose,
                      double intensityPercent,
                      const FieldColorMap &colorMap) const;

    static bool isCoilWithinRange(const vtkMatrix4x4 *coilPose, double maxDistanceMm = 200.0);

private:
    struct GridPoint {
        Vec3 pos;
        float magnitude;
    };

    using GridBucket = std::vector<GridPoint>;

    void buildGrid() const;
    static Vec3 fieldLocalPosition(const Sample &sample);
    static Vec3 transformPoint(vtkMatrix4x4 *matrix, const Vec3 &point);
    static QString gridKey(int x, int y, int z);

    std::vector<Sample> m_samples;
    mutable std::unordered_map<std::string, GridBucket> m_grid;
    mutable bool m_gridBuilt = false;
};

#endif
