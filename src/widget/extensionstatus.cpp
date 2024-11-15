/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019-2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "extensionstatus.h"

#include <QIcon>

ExtensionStatus::ExtensionStatus(QWidget* parent)
    : QLabel(parent)
{
    // Initialize with 0 extensions
    onExtensionSetUpdate(ExtensionSet());
}

void ExtensionStatus::onExtensionSetUpdate(ExtensionSet extensionSet)
{
    QString iconName;
    QString hoverText;
    if (extensionSet.all()) {
        iconName = ":/img/status/extensions_available.svg";
        hoverText = tr("All extensions supported");
    } else if (extensionSet.none()) {
        iconName = ":/img/status/extensions_unavailable.svg";
        hoverText = tr("No extensions supported");
    } else {
        iconName = ":/img/status/extensions_partial.svg";
        hoverText = tr("Not all extensions supported");
    }

    hoverText += "\n";
    hoverText += tr("Multipart Messages: ");
    hoverText += extensionSet[ExtensionType::messages] ? "✔" : "❌";

    auto pixmap = QIcon(iconName).pixmap(QSize(16, 16));

    setPixmap(pixmap);
    setToolTip(hoverText);
}
