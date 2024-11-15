/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <Qt>

class QLayout;
class QVBoxLayout;
class GenericChatItemWidget;

class GenericChatItemLayout
{
public:
    GenericChatItemLayout();
    GenericChatItemLayout(const GenericChatItemLayout& layout) = delete;
    ~GenericChatItemLayout();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    void addSortedWidget(GenericChatItemWidget* widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
#else
    void addSortedWidget(GenericChatItemWidget* widget, int stretch = 0, Qt::Alignment alignment = nullptr);
#endif
    int indexOfSortedWidget(GenericChatItemWidget* widget) const;
    bool existsSortedWidget(GenericChatItemWidget* widget) const;
    void removeSortedWidget(GenericChatItemWidget* widget);
    void search(const QString& searchString, bool hideAll = false);

    QLayout* getLayout() const;

private:
    int indexOfClosestSortedWidget(GenericChatItemWidget* widget) const;
    QVBoxLayout* layout;
};
