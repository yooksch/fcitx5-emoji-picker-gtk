# Generate a C++ array (Emoji[n] from src/emojis.hpp) from https://unicode.org/Public/emoji/14.0/emoji-test.txt and https://api.github.com/emojis

import requests
import unicodedata
import re

emojis = []

def clean_name(s):
    return ''.join(c for c in unicodedata.normalize('NFKD', s) if not unicodedata.combining(c))

def remove_repeating(s, prevent_char):
    new = []
    for i, c in enumerate(s):
        if i == 0 or s[i - 1] != c or prevent_char != s[i]:
            new.append(c)
    return "".join(new)

def format_codepoint(c):
    return f"\\U{c.rjust(8, "0")}"

def fetch_from_unicode_org():
    global emojis

    res = requests.get("https://unicode.org/Public/emoji/14.0/emoji-test.txt")
    lines = [l for l in res.text.splitlines() if not l.startswith("#") and len(l) > 0]

    exp = re.compile(r"^([a-fA-F0-9 ]*[a-zA-Z0-9]+).*;.*?(\S+).*?#.+?(!?[E\d+\.\d+]+).+?(.+)") # Could probably optimize this more
    for line in lines:
        match = exp.match(line)
        if match is None:
            continue

        codepoints, status, _, name = match.groups()

        if status != "fully-qualified":
            continue

        name = clean_name(name.lower())
        name = "".join([c for c in name if c.isalnum() or c.isspace()])
        name = name.replace(" ", "_")

        emojis.append((codepoints.lower().split(" "), name))


def fetch_from_github_api():
    global emojis

    res = requests.get("https://api.github.com/emojis")
    body = res.json()

    exp = re.compile(r".+/([a-fA-F0-9\-]+).+")
    for alias, url in body.items():
        # Check if the emoji exists under the same name
        exists = False
        for e in emojis:
            if e[1] == alias:
                exists = True
        if exists:
            continue

        match = exp.match(url)
        if match is None:
            continue

        codepoints = match.group(1)
        if codepoints == "e": # A bunch of files are just named e.png
            continue

        if "-" in codepoints:
            # Search previous results to get full sequence (github doesn't include ZWJ or other control characters). Requires fetch_from_unicode_org() to be called first
            codepoints = codepoints.split("-")
            for emoji in emojis:
                x = [c for c in emoji[0] if c != "200d" and c != "fe0f"]
                if x == codepoints:
                    codepoints = emoji[0]
                    break
        else:
            codepoints = [codepoints]

        emojis.append((codepoints, alias))

fetch_from_unicode_org()
fetch_from_github_api()

print(f"const Emoji emojis[{len(emojis)}] = {{")
for emoji in emojis:
    print(f"\t{{ \"{emoji[1]}\", \"{"".join([format_codepoint(c) for c in emoji[0]])}\" }},")
print("};")

