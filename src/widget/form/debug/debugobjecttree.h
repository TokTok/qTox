#ifndef DEBUGOBJECTTREE_H
#define DEBUGOBJECTTREE_H

#include "src/widget/form/settings/genericsettings.h"

#include <memory>

class DebugObjectTreeModel;
class Style;

namespace Ui {
class DebugObjectTree;
}

class DebugObjectTree : public GenericForm
{
    Q_OBJECT

public:
    explicit DebugObjectTree(Style& style, QWidget* parent = nullptr);
    ~DebugObjectTree();

    QString getFormName() final
    {
        // Not translated (for now). Debugging only.
        return QStringLiteral("Object Tree");
    }

private:
    std::unique_ptr<Ui::DebugObjectTree> ui_;
    DebugObjectTreeModel* model_;
};

#endif // DEBUGOBJECTTREE_H
