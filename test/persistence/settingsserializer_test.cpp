/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/persistence/settingsserializer.h"

#include <QDebug>
#include <QTemporaryDir>
#include <QTest>
#include <QVariant>

class TestSettingsSerializer : public QObject
{
    Q_OBJECT

private slots:
    void testSaveAndLoad();
};

void TestSettingsSerializer::testSaveAndLoad()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.path() + "/test.ini";

    {
        SettingsSerializer serializer(filePath);
        serializer.beginGroup("TestGroup");
        serializer.setValue("testKey", QVariant(QString("testValue")));
        serializer.endGroup();
        serializer.save();
    }

    {
        SettingsSerializer serializer(filePath);
        serializer.load();
        serializer.beginGroup("TestGroup");
        QVariant val = serializer.value("testKey");
        serializer.endGroup();

        QCOMPARE(val.toString(), QString("testValue"));
    }
}

QTEST_GUILESS_MAIN(TestSettingsSerializer)
#include "settingsserializer_test.moc"
