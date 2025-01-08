/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#include "sandboxserver.h"

#include "src/model/exiftransform.h"

#include <QImage>

SandboxServer::SandboxServer(QObject* parent)
    : SandboxSimpleSource(parent)
{
}

SandboxServer::~SandboxServer() = default;

bool SandboxServer::isReady() const
{
    return true;
}

QPixmap SandboxServer::loadImage(const QByteArray& data)
{
    QImage image = QImage::fromData(data);
    auto orientation = ExifTransform::getOrientation(data);
    image = ExifTransform::applyTransformation(image, orientation);

    return QPixmap::fromImage(image);
}
