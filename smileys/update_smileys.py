#!/usr/bin/env python3
import os
import sys
from xml.dom import minidom  # nosec


def load_smileys(path: str) -> dict[str, list[str]]:
    """Load smileys from emoticons.xml file.

    Args:
        path: Path to the emoticons.xml file.

    Returns:
        A dictionary where the keys are the filenames (without suffix) of the
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


def save_smileys(path: str, smileys: dict[str, list[str]]) -> None:
    """Save smileys to emoticons.xml file.

    Args:
        path: Path to the emoticons.xml file.
        smileys: The same format as the return value of load_smileys.
    """
    doc = minidom.Document()
    root = doc.createElement("messaging-emoticon-map")
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
    root.setAttribute("xsi:noNamespaceSchemaLocation",
                      "../messaging-emoticon-map.xsd")
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
    for emoji_str in sorted(filter_svgs(os.listdir(path)),
                            key=parse_emoji_sequence):
        emoji = emoji_to_string(parse_emoji_sequence(emoji_str))
        if (emoji_str not in smileys or len(smileys[emoji_str]) == 1
                and smileys[emoji_str][0] == emoji):
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


def sort_strings(smileys: dict[str, list[str]]) -> None:
    """Sort the strings in the smileys dict.

    We put the emoji string first.
    """
    for emoji_str, strings in smileys.items():
        emoji = emoji_to_string(parse_emoji_sequence(emoji_str))
        strings.sort(key=lambda s: prefer_emoji(emoji, s))


def main() -> None:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <smileypack>")
        sys.exit(1)
    smileys = load_smileys(os.path.join(sys.argv[1], "emoticons.xml"))
    add_missing_smileys(sys.argv[1], smileys)
    remove_missing_smileys(sys.argv[1], smileys)
    sort_strings(smileys)
    save_smileys(os.path.join(sys.argv[1], "emoticons.xml"), smileys)


if __name__ == "__main__":
    main()
