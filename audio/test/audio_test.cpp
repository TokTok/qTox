/*
    Copyright © 2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "audio/audio.h"

#include <memory>
#include <QtTest/QtTest>
#include <QtGlobal>

#include "audio/iaudiocontrol.h"
#include "audio/iaudiosettings.h"

class MockSettings : public QObject, public IAudioSettings
{
    Q_OBJECT
public:

    QString getInDev() const override { return ""; }
    void setInDev(const QString& deviceSpecifier) override {}

    bool getAudioInDevEnabled() const override { return false; }
    void setAudioInDevEnabled(bool enabled) override {}

    QString getOutDev() const override { return ""; }
    void setOutDev(const QString& deviceSpecifier) override {}

    bool getAudioOutDevEnabled() const override { return false; }
    void setAudioOutDevEnabled(bool enabled) override {}

    qreal getAudioInGainDecibel() const override { return 0.0; }
    void setAudioInGainDecibel(qreal dB) override {}

    qreal getAudioThreshold() const override { return 0.0; }
    void setAudioThreshold(qreal percent) override {}

    int getOutVolume() const override { return 0; }
    int getOutVolumeMin() const override { return 0; }
    int getOutVolumeMax() const override { return 0; }
    void setOutVolume(int volume) override {}

    int getAudioBitrate() const override { return 0; }
    void setAudioBitrate(int bitrate) override {}

    bool getEnableTestSound() const override { return false; }
    void setEnableTestSound(bool newValue) override {}

    SIGNAL_IMPL(MockSettings, inDevChanged, const QString& device);
    SIGNAL_IMPL(MockSettings, audioInDevEnabledChanged, bool enabled);

    SIGNAL_IMPL(MockSettings, outDevChanged, const QString& device);
    SIGNAL_IMPL(MockSettings, audioOutDevEnabledChanged, bool enabled);

    SIGNAL_IMPL(MockSettings, audioInGainDecibelChanged, qreal dB);
    SIGNAL_IMPL(MockSettings, audioThresholdChanged, qreal dB);
    SIGNAL_IMPL(MockSettings, outVolumeChanged, int volume);
    SIGNAL_IMPL(MockSettings, audioBitrateChanged, int bitrate);
    SIGNAL_IMPL(MockSettings, enableTestSoundChanged, bool newValue);
};

class TestAudio : public QObject
{
    Q_OBJECT
private slots:
    void some_test();
};


void TestAudio::some_test()
{
    MockSettings settings;

    auto test_core = Audio::makeAudio(settings);
}

QTEST_GUILESS_MAIN(TestAudio)
#include "audio_test.moc"
