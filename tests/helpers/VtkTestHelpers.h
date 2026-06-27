#ifndef VTKTESTHELPERS_H
#define VTKTESTHELPERS_H

#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

inline vtkSmartPointer<vtkPolyData> makeTrianglePolyData()
{
    vtkNew<vtkPoints> points;
    points->InsertNextPoint(0.0, 0.0, 0.0);
    points->InsertNextPoint(10.0, 0.0, 0.0);
    points->InsertNextPoint(0.0, 10.0, 0.0);

    vtkNew<vtkCellArray> triangles;
    vtkIdType ids[3] = {0, 1, 2};
    triangles->InsertNextCell(3, ids);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points.Get());
    polyData->SetPolys(triangles.Get());
    return polyData;
}

inline vtkSmartPointer<vtkPolyData> makeGridPolyData(int countPerAxis, double spacing = 10.0)
{
    vtkNew<vtkPoints> points;
    for (int z = 0; z < countPerAxis; ++z) {
        for (int y = 0; y < countPerAxis; ++y) {
            for (int x = 0; x < countPerAxis; ++x) {
                points->InsertNextPoint(x * spacing, y * spacing, z * spacing);
            }
        }
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points.Get());
    return polyData;
}

#endif
