#include "src/core/cstring.h"
#include "src/core/core.h"

#include <QtTest>

class TestCore : public QObject
{
    Q_OBJECT

private slots:
    void testSanitize() const;
};


void TestCore::testSanitize() const
{
    QCOMPARE(
        Core::sanitize("hello"),
        QString("hello"));
}


QTEST_MAIN(TestCore)
#include "core_test.moc"
