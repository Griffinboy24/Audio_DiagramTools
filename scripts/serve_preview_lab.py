#!/usr/bin/env python3
"""Serve the repository root for preview-lab project loading."""

from __future__ import annotations

import argparse
import functools
import http.server
import pathlib


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", nargs="?", type=int, default=8066)
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[1]
    handler = functools.partial(
        http.server.SimpleHTTPRequestHandler,
        directory=str(root),
    )
    server = http.server.ThreadingHTTPServer(("127.0.0.1", args.port), handler)
    url = (
        f"http://127.0.0.1:{args.port}/preview_lab/"
        "hise-published-topic.html?project=../articles/"
        "hise-audio-file-to-speaker-demo/article.json&tools=1"
    )
    print(f"Serving {root}")
    print(url)
    server.serve_forever()


if __name__ == "__main__":
    main()
