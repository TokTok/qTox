/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/persistence/settingsserializer.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

namespace {

// Magic bytes that identify a binary-format settings file.
// Must match SettingsSerializer::magic[].
const char settingsMagic[] = {0x51, 0x54, 0x4F, 0x58};

// Encode a positive integer in qTox's VInt format (matches vintToData).
QByteArray encodeVInt(int value)
{
    QByteArray result;
    while (value >= 0x80) {
        result.append(static_cast<char>(value | 0x80));
        value >>= 7;
    }
    result.append(static_cast<char>(value));
    return result;
}

// Build a bytes-only serialized settings file.
// Writes the raw VInt-based format that readSerialized() expects
// (after the 4-byte magic header).
// Format: magic(4) | tag(1) | key_vint | key_bytes | val_vint | val_bytes
QByteArray makeValidSettings(const QByteArray& key, const QByteArray& value)
{
    QByteArray data(settingsMagic, 4);
    // RecordTag::Value = 0x00
    data.append(static_cast<char>(0));
    // Key: VInt-encoded length + raw bytes
    data.append(encodeVInt(key.size()));
    data.append(key);
    // Value: VInt-encoded length + raw bytes
    data.append(encodeVInt(value.size()));
    data.append(value);
    return data;
}

// OOM payload: VInt-encoded size of 0x7FFFFFFF (~2GB) in the value field.
// Before the fix, readStream() called QByteArray::resize(INT_MAX) -> OOM crash.
// After the fix, the shift-count guard rejects it (5 VInt bytes push the
// accumulated shift past 32 bits). The value bytearray is cleared.
QByteArray makeOomPayload()
{
    QByteArray data(settingsMagic, 4);
    data.append(static_cast<char>(0)); // RecordTag::Value
    // Key: 1-byte key
    data.append(encodeVInt(1));
    data.append('K');
    // Value: VInt encoding 0x7FFFFFFF — requires shifts 0,7,14,21,28
    // which exceeds sizeof(int)*CHAR_BIT (32), so the guard fires.
    data.append(encodeVInt(0x7FFFFFFF));
    return data;
}

// Shift-overflow payload: VInt with 5 continuation bytes, pushing the shift
// count past 32 bits. Even if the wrapped value were small, the guard
// should reject it because the shift count itself is invalid.
QByteArray makeShiftOverflowPayload()
{
    QByteArray data(settingsMagic, 4);
    data.append(static_cast<char>(0)); // RecordTag::Value
    data.append(encodeVInt(1));
    data.append('K');
    // 5 continuation bytes: each has the high bit set.
    // Shifts: 0,7,14,21,28 — after byte 4, num2=35 > 32, guard fires.
    data.append(static_cast<char>(0xFF));
    data.append(static_cast<char>(0xFF));
    data.append(static_cast<char>(0xFF));
    data.append(static_cast<char>(0xFF));
    data.append(static_cast<char>(0x0F));
    return data;
}

// Truncated payload: valid key VInt, but the stream ends mid-value.
// The stream-status guard should detect the premature EOF and clear the value.
QByteArray makeTruncatedPayload()
{
    QByteArray data(settingsMagic, 4);
    data.append(static_cast<char>(0)); // RecordTag::Value
    data.append(encodeVInt(1));
    data.append('K');
    // Value VInt says 10 bytes, but no data follows.
    data.append(encodeVInt(10));
    // Deliberately no value data — stream is truncated.
    return data;
}

// Zero-size VInt in value field: the post-loop guard checks num <= 0.
QByteArray makeZeroSizePayload()
{
    QByteArray data(settingsMagic, 4);
    data.append(static_cast<char>(0)); // RecordTag::Value
    data.append(encodeVInt(1));
    data.append('K');
    // VInt(0) = single byte 0x00
    data.append(encodeVInt(0));
    return data;
}

// Pad a payload to at least 8 bytes so isSerializedFormat() can read its
// magic comparison. isSerializedFormat() reads 8 bytes from the file.
QByteArray padTo8(const QByteArray& payload)
{
    if (payload.size() >= 8)
        return payload;
    QByteArray padded = payload;
    padded.append(8 - padded.size(), '\0');
    return padded;
}

void writeSettingsFile(const QString& path, const QByteArray& content)
{
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(content);
    f.close();
}

} // namespace

class TestSettingsSerializer : public QObject
{
    Q_OBJECT

private slots:
    // A valid settings file should load and store values correctly.
    void testLoadValid();

    // An OOM payload (VInt 0x7FFFFFFF) must not crash or allocate huge memory.
    // The key is stored (its readStream succeeds), but the value is cleared
    // by the overflow guard, resulting in an empty QVariant.
    void testLoadOomPayload();

    // A VInt with shift overflow must not cause undefined behavior.
    // The guard clears the value bytearray before it's used.
    void testLoadShiftOverflowPayload();

    // A truncated stream must not crash. Stream-status checks detect EOF.
    void testLoadTruncatedPayload();

    // A zero-length VInt (num=0) must be rejected by the num<=0 guard.
    void testLoadZeroSizePayload();

    // Verify that isSerializedFormat correctly distinguishes binary from INI.
    void testIsSerializedFormat();
};

void TestSettingsSerializer::testLoadValid()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.path() + "/test_valid.ini";

    writeSettingsFile(filePath, padTo8(makeValidSettings("myKey", "myValue")));

    SettingsSerializer serializer(filePath);
    serializer.load();

    QCOMPARE(serializer.value("myKey").toString(), QString("myValue"));
}

void TestSettingsSerializer::testLoadOomPayload()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.path() + "/test_oom.ini";

    writeSettingsFile(filePath, padTo8(makeOomPayload()));

    // Must not crash or OOM. The overflow guard clears the value bytearray,
    // so the key is stored with an empty-string QVariant.
    SettingsSerializer serializer(filePath);
    serializer.load();

    // The key "K" exists but has an empty value because the guard cleared it.
    QCOMPARE(serializer.value("K").toString(), QString());
}

void TestSettingsSerializer::testLoadShiftOverflowPayload()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.path() + "/test_overflow.ini";

    writeSettingsFile(filePath, padTo8(makeShiftOverflowPayload()));

    SettingsSerializer serializer(filePath);
    serializer.load();

    QCOMPARE(serializer.value("K").toString(), QString());
}

void TestSettingsSerializer::testLoadTruncatedPayload()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.path() + "/test_truncated.ini";

    writeSettingsFile(filePath, padTo8(makeTruncatedPayload()));

    SettingsSerializer serializer(filePath);
    serializer.load();

    // Reaching here without crashing is the primary assertion.
    // The key may or may not be stored depending on how far parsing got.
    QVERIFY(true);
}

void TestSettingsSerializer::testLoadZeroSizePayload()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.path() + "/test_zerosize.ini";

    writeSettingsFile(filePath, padTo8(makeZeroSizePayload()));

    SettingsSerializer serializer(filePath);
    serializer.load();

    // The zero-size guard clears the value, leaving an empty string.
    QCOMPARE(serializer.value("K").toString(), QString());
}

void TestSettingsSerializer::testIsSerializedFormat()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // A file starting with the QTOX magic (padded to >=8 bytes) is serialized.
    const QString binPath = tempDir.path() + "/binary.ini";
    writeSettingsFile(binPath, padTo8(makeValidSettings("k", "v")));
    QVERIFY(SettingsSerializer::isSerializedFormat(binPath));

    // A plain-text INI file is not serialized format.
    const QString iniPath = tempDir.path() + "/plain.ini";
    QFile f(iniPath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("[General]\ncount=1\n");
    f.close();
    QVERIFY(!SettingsSerializer::isSerializedFormat(iniPath));
}

QTEST_GUILESS_MAIN(TestSettingsSerializer)
#include "settingsserializer_test.moc"