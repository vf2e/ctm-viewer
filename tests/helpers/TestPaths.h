#ifndef TESTPATHS_H
#define TESTPATHS_H

#include <QString>

namespace TestPaths {

inline QString fixturesRoot()
{
#ifdef TEST_FIXTURES_DIR
    return QStringLiteral(TEST_FIXTURES_DIR);
#else
    return QStringLiteral("tests/fixtures");
#endif
}

inline QString fixture(const char *name)
{
    return fixturesRoot() + QLatin1Char('/') + QString::fromUtf8(name);
}

} // namespace TestPaths

#endif
