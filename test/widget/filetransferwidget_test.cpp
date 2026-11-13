/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/chatlog/content/filetransferwidget.h"

#include "src/core/corefile.h"
#include "src/core/toxfile.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "src/widget/tool/imessageboxmanager.h"

#include <QDebug>
#include <QImage>
#include <QObject>
#include <QPixmap>
#include <QtTest/QtTest>

class MockMessageBoxManagerFT : public IMessageBoxManager
{
public:
    MockMessageBoxManagerFT();
    ~MockMessageBoxManagerFT() override;
    void showInfo(const QString& title, const QString& msg) override
    {
        std::ignore = title;
        std::ignore = msg;
    }
    void showWarning(const QString& title, const QString& msg) override
    {
        std::ignore = title;
        std::ignore = msg;
    }
    void showError(const QString& title, const QString& msg) override
    {
        std::ignore = title;
        std::ignore = msg;
    }
    bool askQuestion(const QString& title, const QString& msg, bool d = false, bool warning = true,
                     bool yesno = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = warning;
        std::ignore = yesno;
        return d;
    }
    bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                     const QString& button2, bool d = false, bool warning = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = button1;
        std::ignore = button2;
        std::ignore = warning;
        return d;
    }
    IMessageBoxManager::Answer askQuestion(const QString& title, const QString& msg,
                                           const QString& button1, const QString& button2,
                                           const QString& button3, bool d = false,
                                           bool warning = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = button1;
        std::ignore = button2;
        std::ignore = button3;
        std::ignore = warning;
        return d ? IMessageBoxManager::Answer::Accepted
                 : IMessageBoxManager::Answer::Rejected;
    }
    void confirmExecutableOpen(const QFileInfo& file) override
    {
        std::ignore = file;
    }
};

MockMessageBoxManagerFT::MockMessageBoxManagerFT() = default;
MockMessageBoxManagerFT::~MockMessageBoxManagerFT() = default;

class TestFileTransferWidget : public QObject
{
    Q_OBJECT
private slots:
    void testRenderingBounds();
};

void TestFileTransferWidget::testRenderingBounds()
{
    MockMessageBoxManagerFT messageBoxManager;
    Settings settings(messageBoxManager);
    Style style;
    QRecursiveMutex dummyMutex;
    CoreFile coreFile(nullptr, dummyMutex);

    // ToxFile(uint32_t fileNum_, uint32_t friendId_, QString fileName_, QString filePath_, uint64_t filesize, FileDirection direction)
    ToxFile file(0, 0, "test.txt", "test.txt", 100, ToxFile::SENDING);
    file.status = ToxFile::TRANSMITTING;

    FileTransferWidget widget(nullptr, coreFile, file, settings, style, messageBoxManager);

    // Set geometry with a non-zero origin. This ensures we test that the widget
    // paints itself using its own local coordinate system (rect()) rather than
    // its parent-relative coordinates (geometry()).
    widget.setGeometry(50, 50, 200, 50);

    // Force the widget to layout and be ready to paint
    widget.show();

    // Grab the rendered pixels of the widget into a QImage
    const QImage image = widget.grab().toImage();

    // Check a pixel near the top-left of the widget (10, 10).
    // If the widget incorrectly uses geometry() instead of rect() to draw its
    // background, it would draw the background starting at its parent-relative
    // offset (50, 50), leaving the local (10, 10) pixel completely transparent.
    // By verifying the pixel is opaque, we prove the background fills the widget
    // area starting from its local origin (0, 0).
    const QColor pixelColor = image.pixelColor(10, 10);

    QCOMPARE(pixelColor.alpha(), 255);
}

QTEST_MAIN(TestFileTransferWidget)
#include "filetransferwidget_test.moc"
