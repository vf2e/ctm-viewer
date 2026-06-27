#include "ModelLoader.h"

#include <QFile>
#include <QFileInfo>

#include <openctm.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkOBJReader.h>
#include <vtkPointData.h>
#include <vtkPoints.h>

#include <cstring>

ModelLoader::ModelLoader()
    : m_polyData(vtkSmartPointer<vtkPolyData>::New())
    , m_hasNormals(false)
    , m_hasTexCoords(false)
{
}

int ModelLoader::vertexCount() const
{
    return m_polyData ? static_cast<int>(m_polyData->GetNumberOfPoints()) : 0;
}

int ModelLoader::triangleCount() const
{
    return m_polyData ? static_cast<int>(m_polyData->GetNumberOfCells()) : 0;
}

bool ModelLoader::loadModel(const QString &source, QString *errorMessage)
{
    if (source.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("文件路径为空");
        }
        return false;
    }

    m_hasNormals = false;
    m_hasTexCoords = false;
    m_polyData = vtkSmartPointer<vtkPolyData>::New();

    const QString suffix = QFileInfo(source).suffix().toLower();
    if (suffix == QStringLiteral("ctm")) {
        loadCTM(source, errorMessage);
    } else if (suffix == QStringLiteral("obj")) {
        loadOBJ(source, errorMessage);
    } else {
        if (errorMessage) {
            *errorMessage = QStringLiteral("仅支持 .ctm 和 .obj 格式");
        }
        return false;
    }

    if (vertexCount() == 0) {
        if (errorMessage && errorMessage->isEmpty()) {
            *errorMessage = QStringLiteral("模型数据为空");
        }
        return false;
    }

    return true;
}

void ModelLoader::loadCTM(const QString &filePath, QString *errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法打开文件: %1").arg(filePath);
        }
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    CTMcontext context = ctmNewContext(CTM_IMPORT);
    if (!context) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("创建 CTM 上下文失败");
        }
        return;
    }

    ctmLoadCustom(context,
                  [](void *buf, CTMuint size, void *userData) -> CTMuint {
                      auto *buffer = static_cast<QByteArray *>(userData);
                      const CTMuint bytesToRead =
                          qMin(size, static_cast<CTMuint>(buffer->size()));
                      std::memcpy(buf, buffer->constData(), bytesToRead);
                      buffer->remove(0, static_cast<int>(bytesToRead));
                      return bytesToRead;
                  },
                  &data);

    if (ctmGetError(context) != CTM_NONE) {
        if (errorMessage) {
            *errorMessage = QString::fromUtf8(ctmErrorString(ctmGetError(context)));
        }
        ctmFreeContext(context);
        return;
    }

    const CTMuint vertexCount = ctmGetInteger(context, CTM_VERTEX_COUNT);
    const CTMfloat *vertices = ctmGetFloatArray(context, CTM_VERTICES);

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(static_cast<vtkIdType>(vertexCount));
    for (CTMuint i = 0; i < vertexCount; ++i) {
        points->SetPoint(static_cast<vtkIdType>(i),
                         vertices[i * 3],
                         vertices[i * 3 + 1],
                         vertices[i * 3 + 2]);
    }

    const CTMuint triangleCount = ctmGetInteger(context, CTM_TRIANGLE_COUNT);
    const CTMuint *indices = ctmGetIntegerArray(context, CTM_INDICES);

    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    for (CTMuint i = 0; i < triangleCount; ++i) {
        vtkIdType pts[3] = {
            static_cast<vtkIdType>(indices[i * 3]),
            static_cast<vtkIdType>(indices[i * 3 + 1]),
            static_cast<vtkIdType>(indices[i * 3 + 2])
        };
        triangles->InsertNextCell(3, pts);
    }

    m_polyData->SetPoints(points);
    m_polyData->SetPolys(triangles);

    m_hasNormals = ctmGetInteger(context, CTM_HAS_NORMALS) != 0;
    if (m_hasNormals) {
        const CTMfloat *normals = ctmGetFloatArray(context, CTM_NORMALS);
        vtkSmartPointer<vtkFloatArray> normalArray = vtkSmartPointer<vtkFloatArray>::New();
        normalArray->SetNumberOfComponents(3);
        normalArray->SetName("Normals");
        for (CTMuint i = 0; i < vertexCount; ++i) {
            normalArray->InsertNextTuple3(normals[i * 3],
                                          normals[i * 3 + 1],
                                          normals[i * 3 + 2]);
        }
        m_polyData->GetPointData()->SetNormals(normalArray);
    }

    m_hasTexCoords = ctmGetInteger(context, CTM_UV_MAP_COUNT) > 0;
    if (m_hasTexCoords) {
        const CTMfloat *texCoords = ctmGetFloatArray(context, CTM_UV_MAP_1);
        vtkSmartPointer<vtkFloatArray> texCoordArray = vtkSmartPointer<vtkFloatArray>::New();
        texCoordArray->SetNumberOfComponents(2);
        texCoordArray->SetName("TextureCoordinates");
        for (CTMuint i = 0; i < vertexCount; ++i) {
            texCoordArray->InsertNextTuple2(texCoords[i * 2], texCoords[i * 2 + 1]);
        }
        m_polyData->GetPointData()->SetTCoords(texCoordArray);
    }

    ctmFreeContext(context);
}

void ModelLoader::loadOBJ(const QString &filePath, QString *errorMessage)
{
    vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName(filePath.toLocal8Bit().constData());
    reader->Update();

    vtkSmartPointer<vtkPolyData> temp = reader->GetOutput();
    if (!temp || temp->GetNumberOfPoints() == 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("OBJ 文件读取失败: %1").arg(filePath);
        }
        return;
    }

    m_polyData->SetPoints(temp->GetPoints());
    m_polyData->SetPolys(temp->GetPolys());

    if (temp->GetPointData()->GetNormals()) {
        m_polyData->GetPointData()->SetNormals(temp->GetPointData()->GetNormals());
        m_hasNormals = true;
    }
    if (temp->GetPointData()->GetTCoords()) {
        m_polyData->GetPointData()->SetTCoords(temp->GetPointData()->GetTCoords());
        m_hasTexCoords = true;
    }
}
