/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "emoticonswidget.h"

#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/style.h"

#include <QFile>
#include <QGridLayout>
#include <QLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>

#include <cmath>

EmoticonsWidget::EmoticonsWidget(SmileyPack& smileyPack, Settings& settings, Style& style,
                                 QWidget* parent)
    : QMenu(parent)
{
    setStyleSheet(style.getStylesheet("emoticonWidget/emoticonWidget.qss", settings));
    setLayout(&layout);
    layout.addWidget(&stack);

    auto* pageButtonsContainer = new QWidget;
    auto* buttonLayout = new QHBoxLayout;
    pageButtonsContainer->setLayout(buttonLayout);

    layout.addWidget(pageButtonsContainer);

    const int maxCols = 8;
    const int maxRows = 8;
    const int itemsPerPage = maxRows * maxCols;

    const QList<QStringList>& emoticons = smileyPack.getEmoticons();
    const int itemCount = emoticons.size();
    const int pageCount = std::ceil(float(itemCount) / float(itemsPerPage));
    int currPage = 0;
    int currItem = 0;
    int row = 0;
    int col = 0;

    // respect configured emoticon size
    const int px = settings.getEmojiFontPointSize();
    const QSize size(px, px);

    // create pages
    buttonLayout->addStretch();
    for (int i = 0; i < pageCount; ++i) {
        auto* pageLayout = new QGridLayout;
        pageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
                            maxRows, 0);
        pageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0,
                            maxCols);

        auto* page = new QWidget;
        page->setLayout(pageLayout);
        stack.addWidget(page);

        // page buttons are only needed if there is more than 1 page
        if (pageCount > 1) {
            auto* pageButton = new QRadioButton;
            pageButton->setProperty("pageIndex", i);
            pageButton->setCursor(Qt::PointingHandCursor);
            pageButton->setChecked(i == 0);
            buttonLayout->addWidget(pageButton);

            connect(pageButton, &QRadioButton::clicked, this, &EmoticonsWidget::onPageButtonClicked);
        }
    }
    buttonLayout->addStretch();

    for (const QStringList& set : emoticons) {
        auto* button = new QPushButton;
        const std::shared_ptr<QIcon> icon = smileyPack.getAsIcon(set[0]);
        emoticonsIcons.append(icon);
        button->setIcon(icon->pixmap(size));
        button->setToolTip(set.join(" "));
        button->setProperty("sequence", set[0]);
        button->setCursor(Qt::PointingHandCursor);
        button->setFlat(true);
        button->setIconSize(size);
        button->setFixedSize(size);

        connect(button, &QPushButton::clicked, this, &EmoticonsWidget::onSmileyClicked);

        qobject_cast<QGridLayout*>(stack.widget(currPage)->layout())->addWidget(button, row, col);

        ++col;
        ++currItem;

        // next row
        if (col >= maxCols) {
            col = 0;
            ++row;
        }

        // next page
        if (currItem >= itemsPerPage) {
            row = 0;
            currItem = 0;
            ++currPage;
        }
    }

    // calculates sizeHint
    layout.activate();
}

void EmoticonsWidget::onSmileyClicked()
{
    // emit insert emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender != nullptr) {
        const QString sequence =
            sender->property("sequence").toString().replace("&lt;", "<").replace("&gt;", ">");
        emit insertEmoticon(sequence);
    }
}

void EmoticonsWidget::onPageButtonClicked()
{
    QWidget* sender = qobject_cast<QRadioButton*>(QObject::sender());
    if (sender != nullptr) {
        const int page = sender->property("pageIndex").toInt();
        stack.setCurrentIndex(page);
    }
}

QSize EmoticonsWidget::sizeHint() const
{
    return layout.sizeHint();
}

void EmoticonsWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (!rect().contains(ev->pos()))
        hide();
}

void EmoticonsWidget::mousePressEvent(QMouseEvent* event)
{
    std::ignore = event;
}

void EmoticonsWidget::wheelEvent(QWheelEvent* e)
{
    const bool vertical = qAbs(e->angleDelta().y()) >= qAbs(e->angleDelta().x());
    const int delta = vertical ? e->angleDelta().y() : e->angleDelta().x();

    if (vertical) {
        if (delta < 0) {
            stack.setCurrentIndex(stack.currentIndex() + 1);
        } else {
            stack.setCurrentIndex(stack.currentIndex() - 1);
        }
        emit PageButtonsUpdate();
    }
}

void EmoticonsWidget::PageButtonsUpdate()
{
    const QList<QRadioButton*> pageButtons = findChildren<QRadioButton*>(QString());
    foreach (QRadioButton* t_pageButton, pageButtons) {
        if (t_pageButton->property("pageIndex").toInt() == stack.currentIndex())
            t_pageButton->setChecked(true);
        else
            t_pageButton->setChecked(false);
    }
}

void EmoticonsWidget::keyPressEvent(QKeyEvent* e)
{
    std::ignore = e;
    hide();
}
