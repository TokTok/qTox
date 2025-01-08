/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "imagepreviewwidget.h"
#include "src/platform/sandbox.h"

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QScreen>
#include <QString>

namespace {
static const QStringList previewExtensions = {
    "gif",  // Graphics Interchange Format
    "jpeg", // Joint Photographic Experts Group
    "jpg",  // Joint Photographic Experts Group
    "png",  // Portable Network Graphics
    "qoi",  // Quite OK Image Format
    "svg",  // Scalable Vector Graphics
    "webp", // WebP
};
QPixmap pixmapFromFile(ImageLoader& imageLoader, const QString& filename)
{
    if (!previewExtensions.contains(QFileInfo(filename).suffix().toLower())) {
        return {};
    }

    QFile imageFile(filename);
    if (!imageFile.open(QIODevice::ReadOnly)) {
        return {};
    }

    return imageLoader.loadImage(imageFile.readAll());
}

QPixmap scaleCropIntoSquare(const QPixmap& source, const int targetSize)
{
    QPixmap result;

    // Make sure smaller-than-icon images (at least one dimension is smaller) will not be
    // upscaled
    if (source.width() < targetSize || source.height() < targetSize) {
        result = source;
    } else {
        result = source.scaled(targetSize, targetSize, Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);
    }

    // Then, image has to be cropped (if needed) so it will not overflow rectangle
    // Only one dimension will be bigger after Qt::KeepAspectRatioByExpanding
    if (result.width() > targetSize) {
        return result.copy((result.width() - targetSize) / 2, 0, targetSize, targetSize);
    }
    if (result.height() > targetSize) {
        return result.copy(0, (result.height() - targetSize) / 2, targetSize, targetSize);
    }

    // Picture was rectangle in the first place, no cropping
    return result;
}

QString getToolTipDisplayingImage(const QPixmap& image)
{
    // Show mouseover preview, but make sure it's not larger than 50% of the screen
    // width/height
    const QRect desktopSize = QGuiApplication::primaryScreen()->geometry();
    const int maxPreviewWidth{desktopSize.width() / 2};
    const int maxPreviewHeight{desktopSize.height() / 2};
    const QPixmap previewImage = [&image, maxPreviewWidth, maxPreviewHeight]() {
        if (image.width() > maxPreviewWidth || image.height() > maxPreviewHeight) {
            return image.scaled(maxPreviewWidth, maxPreviewHeight, Qt::KeepAspectRatio,
                                Qt::SmoothTransformation);
        }
        return image;
    }();

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    previewImage.save(&buffer, "PNG");
    buffer.close();

    return "<img src=data:image/png;base64," + QString::fromUtf8(imageData.toBase64()) + "/>";
}

} // namespace

ImagePreviewButton::~ImagePreviewButton() = default;

void ImagePreviewButton::initialize(const QPixmap& image)
{
    auto desiredSize = qMin(width(), height()); // Assume widget is a square
    desiredSize = qMax(desiredSize, 4) - 4;     // Leave some room for a border

    auto croppedImage = scaleCropIntoSquare(image, desiredSize);
    setIcon(QIcon(croppedImage));
    setIconSize(croppedImage.size());
    setToolTip(getToolTipDisplayingImage(image));
}

void ImagePreviewButton::setIconFromFile(ImageLoader& imageLoader, const QString& filename)
{
    initialize(pixmapFromFile(imageLoader, filename));
}

void ImagePreviewButton::setIconFromPixmap(const QPixmap& pixmap)
{
    initialize(pixmap);
}
