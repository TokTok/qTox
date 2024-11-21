/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#include "debugwidget.h"

#include "src/widget/contentlayout.h"
#include "src/widget/form/debug/debuglog.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

#include <QStyleFactory>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWindow>

#include <memory>

DebugWidget::DebugWidget(Paths& paths, Style& style, Widget* parent)
    : QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);

    bodyLayout = std::unique_ptr<QVBoxLayout>(new QVBoxLayout());

    debugWidgets = std::unique_ptr<QTabWidget>(new QTabWidget(this));
    debugWidgets->setTabPosition(QTabWidget::North);
    bodyLayout->addWidget(debugWidgets.get());

    std::unique_ptr<DebugLogForm> abtfrm(new DebugLogForm(paths, style));

    dbgForms = {{std::move(abtfrm)}};
    for (auto& dbgForm : dbgForms)
        debugWidgets->addTab(dbgForm.get(), dbgForm->getFormIcon(), dbgForm->getFormName());

    connect(debugWidgets.get(), &QTabWidget::currentChanged, this, &DebugWidget::onTabChanged);

    Translator::registerHandler(std::bind(&DebugWidget::retranslateUi, this), this);
}

DebugWidget::~DebugWidget()
{
    Translator::unregister(this);
}

void DebugWidget::setBodyHeadStyle(QString style)
{
    debugWidgets->setStyle(QStyleFactory::create(style));
}

bool DebugWidget::isShown() const
{
    if (debugWidgets->isVisible()) {
        debugWidgets->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void DebugWidget::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(debugWidgets.get());
    debugWidgets->show();
    onTabChanged(debugWidgets->currentIndex());
}

void DebugWidget::onTabChanged(int index)
{
    debugWidgets->setCurrentIndex(index);
}

void DebugWidget::retranslateUi()
{
    for (size_t i = 0; i < dbgForms.size(); ++i)
        debugWidgets->setTabText(i, dbgForms[i]->getFormName());
}
