/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "pixmapcache.h"

QPixmap PixmapCache::get(const QString& filename, QSize size)
{
    auto itr = cache.find(filename);

    if (itr == cache.end()) {
        QIcon icon;
        icon.addFile(filename);

        cache.insert(filename, icon);
        return icon.pixmap(size);
    }

    return itr.value().pixmap(size);
}

/**
 * @brief Returns the singleton instance.
 */
PixmapCache& PixmapCache::getInstance()
{
    static PixmapCache instance;
    return instance;
}
