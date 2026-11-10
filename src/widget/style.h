/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QColor>
#include <QFont>
#include <QMap>
#include <QObject>

class QString;
class QWidget;
class Settings;

class Style : public QObject
{
    Q_OBJECT
public:
    enum class ColorPalette
    {
        TransferGood,
        TransferWait,
        TransferBad,
        TransferMiddle,
        MainText,
        NameActive,
        StatusActive,
        GroundExtra,
        GroundBase,
        Orange,
        Yellow,
        ThemeDark,
        ThemeMediumDark,
        ThemeMedium,
        ThemeLight,
        Action,
        Link,
        SearchHighlighted,
        SelectText,
    };

    enum class Font
    {
        ExtraBig,
        Big,
        BigBold,
        Medium,
        MediumBold,
        Small,
        SmallLight,
    };

    enum class MainTheme
    {
        Light,
        Dark,
    };
    Q_ENUM(MainTheme)

    static int defaultThemeColor(MainTheme theme);

    static QStringList getThemeColorNames();
    static QString getThemeFolder(int themeColor);
    static QString getThemeName();
    static QFont getFont(Font font);
    static void repolish(QWidget* w);
    void applyTheme();
    static QPixmap scaleSvgImage(const QString& path, uint32_t width, uint32_t height);

    Style() = default;
    QString getStylesheet(const QString& filename, const Settings& settings,
                          const QFont& baseFont = QFont());
    QString getStylesheet(const QString& filename, int themeColor, const QFont& baseFont = QFont());
    QString getImagePath(const QString& filename, const Settings& settings);
    QString getImagePath(const QString& filename, int themeColor);
    QColor getColor(ColorPalette entry);
    QString resolve(const QString& filename, int themeColor, const QFont& baseFont = QFont());
    void setThemeColor(int themeColor, int color);
    void setThemeColor(const QColor& color);
    void initPalette(int themeColor);
    void initDictColor();
    static QString getThemePath(int themeColor);

signals:
    void themeReload();

private:
    QMap<ColorPalette, QColor> palette;
    QMap<QString, QString> dictColor;
    QMap<QString, QString> dictFont;
    QMap<QString, QString> dictTheme;
    // stylesheet filename, font -> stylesheet
    // QString implicit sharing deduplicates stylesheets rather than constructing a new one each time
    std::map<std::pair<const QString, const QFont>, const QString> stylesheetsCache;
    QStringList existingImagesCache;
};
