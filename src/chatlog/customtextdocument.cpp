/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "customtextdocument.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/style.h"

#include <QDebug>
#include <QIcon>
#include <QUrl>

CustomTextDocument::CustomTextDocument(SmileyPack& smileyPack_,
    Settings& settings_, QObject* parent)
    : QTextDocument(parent)
    , smileyPack(smileyPack_)
    , settings(settings_)
{
    setUndoRedoEnabled(false);
    setUseDesignMetrics(false);
}

QVariant CustomTextDocument::loadResource(int type, const QUrl& name)
{
    if (type == QTextDocument::ImageResource && name.scheme() == "key") {
        QSize size = QSize(settings.getEmojiFontPointSize(),
                           settings.getEmojiFontPointSize());
        QString fileName = QUrl::fromPercentEncoding(name.toEncoded()).mid(4).toHtmlEscaped();

        std::shared_ptr<QIcon> icon = smileyPack.getAsIcon(fileName);
        emoticonIcons.append(icon);
        return icon->pixmap(size);
    }

    return QTextDocument::loadResource(type, name);
}
