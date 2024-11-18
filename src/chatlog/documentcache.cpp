/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "documentcache.h"
#include "customtextdocument.h"

DocumentCache::DocumentCache(SmileyPack& smileyPack_, Settings& settings_)
    : smileyPack{smileyPack_}
    , settings{settings_}
{
}
DocumentCache::~DocumentCache()
{
    while (!documents.isEmpty())
        delete documents.pop();
}

QTextDocument* DocumentCache::pop()
{
    if (documents.empty())
        documents.push(new CustomTextDocument(smileyPack, settings));

    return documents.pop();
}

void DocumentCache::push(QTextDocument* doc)
{
    if (doc != nullptr) {
        doc->clear();
        documents.push(doc);
    }
}
