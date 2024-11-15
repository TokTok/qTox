/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2005-2014 by the Quassel Project <devel@quassel-irc.org>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/group.h"
#include "src/widget/tool/chattextedit.h"
#include <QMap>
#include <QString>

class TabCompleter : public QObject
{
    Q_OBJECT
public:
    TabCompleter(ChatTextEdit* msgEdit_, Group* group_);

public slots:
    void complete();
    void reset();

private:
    struct SortableString
    {
        explicit SortableString(const QString& n)
            : contents{n}
        {
        }
        bool operator<(const SortableString& other) const;
        QString contents;
    };

    ChatTextEdit* msgEdit;
    Group* group;
    bool enabled;
    const static QString nickSuffix;

    QMap<SortableString, QString> completionMap;
    QMap<SortableString, QString>::Iterator nextCompletion;
    int lastCompletionLength;

    void buildCompletionList();
};
