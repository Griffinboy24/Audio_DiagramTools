#!/usr/bin/env python3
"""Serve the repository root for preview-lab project loading."""

from __future__ import annotations

import argparse
import functools
import http.server
import pathlib
import threading
import webbrowser


def article_url(port: int, project: str, tools: bool) -> str:
    return (
        f"http://127.0.0.1:{port}/preview_lab/"
        "hise-published-topic.html?project=../articles/"
        f"{project}/article.json&tools={'1' if tools else '0'}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", nargs="?", type=int, default=8066)
    parser.add_argument("--project", default="hise-dsp-buffer")
    parser.add_argument("--tools", action="store_true")
    parser.add_argument("--open", action="store_true", dest="open_browser")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[1]
    handler = functools.partial(
        http.server.SimpleHTTPRequestHandler,
        directory=str(root),
    )
    server = http.server.ThreadingHTTPServer(("127.0.0.1", args.port), handler)
    current_post_url = f"http://127.0.0.1:{args.port}/preview.html"
    url = article_url(args.port, args.project, args.tools)
    print(f"Serving {root}")
    print(f"Current post: {current_post_url}")
    print(f"Lab article:  {url}")
    if args.open_browser:
        threading.Timer(0.25, lambda: webbrowser.open(current_post_url)).start()
    server.serve_forever()


if __name__ == "__main__":
    main()
