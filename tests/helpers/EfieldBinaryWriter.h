#ifndef EFIELDBINARYWRITER_H
#define EFIELDBINARYWRITER_H

#include <QByteArray>
#include <QFile>
#include <QString>

#include <cstring>
#include <vector>

struct EfieldSample {
    float x;
    float y;
    float z;
    float magnitude;
};

inline bool writeEfieldBinary(const QString &path, const std::vector<EfieldSample> &samples)
{
    QByteArray bytes;
    bytes.resize(4 + static_cast<int>(samples.size()) * 16);

    const quint32 count = static_cast<quint32>(samples.size());
    auto *out = reinterpret_cast<unsigned char *>(bytes.data());
    out[0] = static_cast<unsigned char>(count & 0xff);
    out[1] = static_cast<unsigned char>((count >> 8) & 0xff);
    out[2] = static_cast<unsigned char>((count >> 16) & 0xff);
    out[3] = static_cast<unsigned char>((count >> 24) & 0xff);

    int offset = 4;
    for (const EfieldSample &sample : samples) {
        std::memcpy(bytes.data() + offset, &sample.x, 16);
        offset += 16;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(bytes) == bytes.size();
}

#endif
