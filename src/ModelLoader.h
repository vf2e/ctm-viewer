#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <QString>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class ModelLoader
{
public:
    ModelLoader();

    bool loadModel(const QString &source, QString *errorMessage = nullptr);

    bool hasNormals() const { return m_hasNormals; }
    bool hasTexCoords() const { return m_hasTexCoords; }
    int vertexCount() const;
    int triangleCount() const;

    vtkSmartPointer<vtkPolyData> polyData() const { return m_polyData; }

private:
    void loadCTM(const QString &filePath, QString *errorMessage);
    void loadOBJ(const QString &filePath, QString *errorMessage);

    vtkSmartPointer<vtkPolyData> m_polyData;
    bool m_hasNormals;
    bool m_hasTexCoords;
};

#endif
