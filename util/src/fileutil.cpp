/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#include "util/fileutil.h"

#include <QDebug>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>

QStringList FileUtil::tail(const QString& filename, int count)
{
    if (count <= 0) {
        return {};
    }

    QFile file{filename};
    if (!file.exists()) {
        qDebug() << "File" << filename << "does not exist";
        return {};
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Unable to open file" << filename;
        return {};
    }

    // This could be made more efficient. We don't need to read the whole file,
    // but we use this function rarely, so it's not a priority. Clarity of the
    // code is more important here.
    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }

    if (lines.isEmpty()) {
        qDebug() << "File" << filename << "is empty";
        return {};
    }

    if (lines.size() <= count) {
        return lines;
    }

    return lines.mid(lines.size() - count);
}
