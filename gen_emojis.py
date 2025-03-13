# Generate a C++ array (Emoji[n] from src/emojis.hpp) from https://unicode.org/Public/emoji/14.0/emoji-test.txt and https://api.github.com/emojis
# TODO: Rewrite this to be not-messy

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

    for line in lines:
        x = line.split(";", maxsplit=1)
        codepoints = x[0].strip().lower()
        status, emoji = x[1].split("#", maxsplit=1)
        if status.strip() != "fully-qualified":
            continue

        name = clean_name(emoji.split(" ", maxsplit=3)[-1].lower().replace(" ", "_"))
        if name.startswith("flag:"):
            name = name[6:] + "_flag"

        name = "".join([c for c in name if c.isalnum() or c == "_"])
        name = remove_repeating(name, "_")

        emojis.append((codepoints.split(" "), name))

def fetch_from_github_api():
    global emojis

    res = requests.get("https://api.github.com/emojis")
    body = res.json()

    for alias, url in body.items():
        # Check if the emoji exists under the same name
        exists = False
        for e in emojis:
            if e[1] == alias:
                exists = True
        if exists:
            continue

        codepoints = url.split("/")[-1].split(".")[0].lower()
        if "-" in codepoints:
            # Search previous results to get full sequence (github doesn't include ZWJ). Requires fetch_from_unicode_org() to be called first
            codepoints = codepoints.split("-")
            for emoji in emojis:
                x = [c for c in emoji[0] if c != "200d" and c != "fe0f"]
                if x == codepoints:
                    codepoints = emoji[0]
                    break
        else:
            if not re.match(r'^[0-9a-f]+$', codepoints):
                continue
            codepoints = [codepoints]

        emojis.append((codepoints, alias))

fetch_from_unicode_org()
fetch_from_github_api()

print(f"const Emoji emojis[{len(emojis)}] = {{")
for emoji in emojis:
    print(f"\t{{ \"{emoji[1]}\", \"{"".join([format_codepoint(c) for c in emoji[0]])}\" }},")
print("};")

