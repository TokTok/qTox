#include "debugobjecttree.h"
#include "ui_debugobjecttree.h"

#include "src/model/debug/debugobjecttreemodel.h"

DebugObjectTree::DebugObjectTree(Style& style, QWidget* parent)
    : GenericForm{QPixmap(":/img/settings/general.png"), style, parent}
    , ui_(std::make_unique<Ui::DebugObjectTree>())
    , model_(new DebugObjectTreeModel(this))
{
    ui_->setupUi(this);

    ui_->objectTree->setModel(model_);
    ui_->objectTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui_->btnReload, &QPushButton::clicked, model_, &DebugObjectTreeModel::reload);
}

DebugObjectTree::~DebugObjectTree() {}
