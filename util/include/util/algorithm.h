/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <algorithm>
#include <iterator>
#include <map>
#include <vector>

template <typename K, typename V, typename F>
void forEachKey(const std::map<K, V>& map, F func)
{
    // Copy keys because func may modify the map.
    std::vector<K> keys;
    keys.reserve(map.size());
    std::transform(map.begin(), map.end(), std::back_inserter(keys),
                   [](const auto& pair) { return pair.first; });
    for (const auto key : keys) {
        func(key);
    }
}
