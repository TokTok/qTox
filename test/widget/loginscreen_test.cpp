#include "src/widget/loginscreen.h"

#include <QDebug>
#include <QProcessEnvironment>
#include <QTest>

#include "src/persistence/paths.h"
#include "src/widget/style.h"

class TestLoginScreen : public QObject
{
    Q_OBJECT
private slots:
    void testLoginScreen();
};

void TestLoginScreen::testLoginScreen()
{
    Paths paths;
    Style style;
    int themeColor = 0; // Default
    QString profileName = "test-user";
    LoginScreen loginScreen(paths, style, themeColor, profileName);

    const QPixmap actual = loginScreen.grab();
    const auto env = QProcessEnvironment::systemEnvironment();
    if (env.contains("BUILD_WORKSPACE_DIRECTORY")) {
        const QString actualPath = QStringLiteral("%1/%2")
                                       .arg(env.value("BUILD_WORKSPACE_DIRECTORY"))
                                       .arg("qtox/test/resources/images/loginscreen.png");
        qDebug() << "Saving actual image to" << actualPath;
        actual.save(actualPath);
    }
    const QPixmap expected = QPixmap(":/test/images/loginscreen.png");
    QCOMPARE(actual, expected);
}

QTEST_MAIN(TestLoginScreen)
#include "loginscreen_test.moc"
