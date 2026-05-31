#!/usr/bin/env python3
import argparse
import json
import os
from xml.dom import minidom  # nosec


def load_smileys(path: str) -> dict[str, list[str]]:  # noqa: D213
    """Load smileys from emoticons.xml file.

    Args:
        path: Path to the emoticons.xml file.

    Returns
    -------
        smileys: A dictionary where the keys are the filenames (without suffix) of the
        smileys and the values are the strings that will be replaced by the
        file when sent in a message.

    """
    smileys: dict[str, list[str]] = {}
    dom = minidom.parse(path)  # nosec
    for emoticon in dom.getElementsByTagName("emoticon"):
        file = emoticon.getAttribute("file")
        smileys[file] = [
            string.firstChild.nodeValue
            for string in emoticon.getElementsByTagName("string")
            if isinstance(string.firstChild, minidom.Text)
        ]
    return smileys


def save_smileys(path: str, smileys: dict[str, list[str]]) -> None:  # noqa: D213,D407
    """Save smileys to emoticons.xml file.

    Args:
        path: Path to the emoticons.xml file.
        smileys: The same format as the return value of load_smileys.

    """
    doc = minidom.Document()
    root = doc.createElement("messaging-emoticon-map")
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
    root.setAttribute("xsi:noNamespaceSchemaLocation", "../messaging-emoticon-map.xsd")
    doc.appendChild(root)
    for file, strings in smileys.items():
        emoticon = doc.createElement("emoticon")
        emoticon.setAttribute("file", file)
        root.appendChild(emoticon)
        for string in strings:
            s = doc.createElement("string")
            s.appendChild(doc.createTextNode(string))
            emoticon.appendChild(s)
    with open(path, "wb") as f:
        f.write(doc.toprettyxml(indent="    ", encoding="utf-8"))


def filter_svgs(files: list[str]) -> list[str]:
    """Filter out non-svg files and return the filenames without the suffix."""
    return [file[:-4] for file in files if file.endswith(".svg")]


def parse_emoji_sequence(emoji_str: str) -> tuple[int, ...]:
    """Parse a hex codepoint sequence into a tuple of integers."""
    return tuple(int(c, 16) for c in emoji_str.split("-"))


def emoji_to_string(emoji: tuple[int, ...]) -> str:
    """Convert a tuple of codepoints to a string."""
    return "".join(chr(c) for c in emoji)


def add_missing_smileys(path: str, smileys: dict[str, list[str]]) -> None:
    """Add smileys that exist in the path but not in the smileys dict."""
    for emoji_str in sorted(filter_svgs(os.listdir(path)), key=parse_emoji_sequence):
        emoji = emoji_to_string(parse_emoji_sequence(emoji_str))
        if (
            emoji_str not in smileys
            or len(smileys[emoji_str]) == 1
            and smileys[emoji_str][0] == emoji
        ):
            smileys[emoji_str] = [emoji]
        if emoji not in smileys[emoji_str]:
            smileys[emoji_str].append(emoji)


def remove_missing_smileys(path: str, smileys: dict[str, list[str]]) -> None:
    """Remove smileys that exist in the smileys dict but not in the path."""
    svgs = set(filter_svgs(os.listdir(path)))
    for emoji_str in smileys.keys():
        if emoji_str not in svgs:
            del smileys[emoji_str]


def prefer_emoji(emoji: str, string: str) -> str:
    """Provide sorting key that puts the emoji first."""
    if string == emoji:
        return ""
    return string


def sort_strings(smileys: dict[str, list[str]]) -> None:  # noqa: D213
    """Sort the strings in the smileys dict.

    We put the emoji string first.
    """
    for emoji_str, strings in smileys.items():
        emoji = emoji_to_string(parse_emoji_sequence(emoji_str))
        strings.sort(key=lambda s: prefer_emoji(emoji, s))


def block_smiley_maybe(
    smileys: dict[str, list[str]],
    blocked_smileys_in_pack: dict[str, list[str]],
    block: str | None,
) -> bool:  # noqa: D213
    """Block smiley defined in block and return True if blocked smiley list was modified.

    Args
    ----
        smileys: The loaded set of smileys.
        blocked_smileys_in_pack: the list of currently blocked smileys.
        block: The smiley to be blocked.

    Returns
    -------
        True if blocked_smileys_in_pack was modified, False otherwise.

    """
    is_dirty = False
    if not block:
        return is_dirty
    block_name = None
    for name, strings in smileys.items():
        smiley_set = set(strings)
        if block in smiley_set:
            block_name = name
            block_list = blocked_smileys_in_pack.get(block_name, [])
            if block not in block_list:
                # Add blocked smiley.
                block_list.append(block)
                blocked_smileys_in_pack[block_name] = block_list
                is_dirty = True
            else:
                print(f'The smiley "{block}" is already blocked.')
            break
    if not block_name:
        is_blocked = False
        for name, strings in blocked_smileys_in_pack.items():
            if block in strings:
                print(f'The smiley "{block}" is already blocked.')
                break
        if is_blocked:
            print(f'The smiley to block "{block}" was not found.')
    return is_dirty


def unblock_smiley_maybe(
    smileys: dict[str, list[str]],
    blocked_smileys_in_pack: dict[str, list[str]],
    unblock: str | None,
) -> bool:  # noqa: D213
    """Unblock smiley defined in unblock and return True if blocked smiley list was modified.

    Args:
        smileys: The loaded set of smileys.
        blocked_smileys_in_pack: the list of currently blocked smileys.
        unblock: The smiley to be unblocked.

    Returns
    -------
        True if blocked_smileys_in_pack was modified, False otherwise.

    """
    is_dirty = False
    unblock_name = None
    if not unblock:
        return is_dirty
    for name, strings in blocked_smileys_in_pack.items():
        smiley_set = set(strings)
        if unblock in smiley_set:
            # Unblock smiley.
            unblock_name = name
            blocked_smileys_in_pack[unblock_name].remove(unblock)
            smiley_list = smileys.get(unblock_name, [])
            smiley_list.append(unblock)
            smileys[unblock_name] = list(set(smiley_list))
            is_dirty = True
            break
    if not unblock_name:
        print(f'The smiley to unblock "{unblock}" was not found in the blocklist.')
    return is_dirty


def load_and_update_blocklist(
    smileypack: str,
    smileys: dict[str, list[str]],
    block: str | None,
    unblock: str | None,
) -> dict[str, list[str]]:  # noqa: D213
    """Load the smiley block list, update and save it.

    We also add unblocked smileys to the smileys dictionary.

    Args:
        smileypack: The name of a smiley pack to update/load.
        smileys: The loaded set of smileys.
        block: The smiley to be blocked.
        unblock: The smiley to be unblocked.

    Returns
    -------
        The dictionary, containing where the keys are the filenames (without suffix) of the
        smileys and the values are strings that will be replaced by the
        file when sent in a message.

    """
    if block == unblock:
        raise ValueError("The smiley cannot be blocked and unblocked simultaneously.")
    block_list_file = os.path.join(os.path.dirname(__file__), "blocked_smileys.json")
    blocked_smileys: dict[str, dict[str, set[str]]] = {}
    if os.path.isfile(block_list_file):
        with open(block_list_file, "r") as f:
            blocked_smileys: dict[str, set[str]] = json.load(f)
    blocked_smileys_in_pack = blocked_smileys.get(smileypack, {})
    # Update the dictionary of blocked smileys if needed.
    is_dirty = block_smiley_maybe(smileys, blocked_smileys_in_pack, block)
    # Search for the smiley to unblock in the block list and remove it.
    is_dirty = (
        unblock_smiley_maybe(smileys, blocked_smileys_in_pack, unblock) or is_dirty
    )

    if is_dirty:
        sort_strings(blocked_smileys_in_pack)
        blocked_smileys[smileypack] = blocked_smileys_in_pack
        with open(block_list_file, "w") as f:
            json.dump(blocked_smileys, f, ensure_ascii=False, indent=2)
    return blocked_smileys.get(smileypack, {})


def remove_blocked_smileys(
    smileys: dict[str, list[str]], blocked_smileys: dict[str, list[str]]
) -> None:  # noqa: D213, D407
    """Remove all blocked_smileys from smileys.

    Args:
        smileys: The loaded set of smileys.
        blocked_smileys: The loaded smileys to be blocked.

    """
    for fname, strings in blocked_smileys.items():
        if fname in smileys:
            smileys[fname] = list(set(smileys[fname]).difference(set(strings)))


def main() -> None:
    parser = argparse.ArgumentParser(
        description="The script to parse and fix smileys directories."
    )
    parser.add_argument("smileypack", help="The folder with smileys.")
    parser.add_argument(
        "-b",
        "--block-smiley",
        help="The smiley to be added to blocklist.",
        required=False,
        default=None,
    )
    parser.add_argument(
        "-u",
        "--unblock-smiley",
        help="The smiley to be removed from blocklist.",
        required=False,
        default=None,
    )
    args = parser.parse_args()
    smileys = load_smileys(
        os.path.join(os.path.dirname(__file__), args.smileypack, "emoticons.xml")
    )
    add_missing_smileys(args.smileypack, smileys)
    remove_missing_smileys(args.smileypack, smileys)
    blocked_smileys = load_and_update_blocklist(
        args.smileypack, smileys, args.block_smiley, args.unblock_smiley
    )
    remove_blocked_smileys(smileys, blocked_smileys)
    sort_strings(smileys)
    save_smileys(os.path.join(args.smileypack, "emoticons.xml"), smileys)


if __name__ == "__main__":
    main()
