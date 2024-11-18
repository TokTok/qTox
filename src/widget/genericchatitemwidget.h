/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QFrame>
#include <QLabel>

class CroppingLabel;
class Style;

class GenericChatItemWidget : public QFrame
{
    Q_OBJECT
public:
    enum ItemType
    {
        ConferenceItem,
        FriendOfflineItem,
        FriendOnlineItem
    };

    GenericChatItemWidget(bool compact_, Style& style, QWidget* parent = nullptr);

    bool isCompact() const;
    void setCompact(bool compact_);

    QString getName() const;

    void searchName(const QString& searchString, bool hideAll);

    Q_PROPERTY(bool compact READ isCompact WRITE setCompact)

public slots:
    virtual void reloadTheme() {}

protected:
    CroppingLabel* nameLabel;
    QLabel statusPic;

private:
    bool compact;
};
