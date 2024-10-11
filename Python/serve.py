#!/usr/bin/env python

# Base on https://gist.github.com/iktakahiro/2c48962561ea724f1e9d
# Python3 http.server for Single Page Application

import urllib.parse
import http.server
import socketserver
import sys
import re
from pathlib import Path

pattern = re.compile(".png|.jpg|.jpeg|.js|.css|.ico|.gif|.svg", re.IGNORECASE)


class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header(
            "Cache-Control", "no-cache, no-store, must-revalidate")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        super().end_headers()

    def do_GET(self):
        url_parts = urllib.parse.urlparse(self.path)
        req_filepath = Path(url_parts.path.strip("/"))
        ext = req_filepath.suffix

        if not req_filepath.is_file() and not pattern.match(ext):
            self.path = "index.html"

        return http.server.SimpleHTTPRequestHandler.do_GET(self)


def help_and_exit():
    print("USAGE: serve.py [port]")
    exit(1)


def serve(port):
    address = ("0.0.0.0", port)

    print(f"Serving HTTP on {address[0]} port {
          address[1]} (http://{address[0]}:{address[1]}) ...")

    httpd = socketserver.TCPServer(address, Handler)

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        httpd.shutdown()


if __name__ == "__main__":
    argc = len(sys.argv)
    if argc > 2:
        help_and_exit()

    port = 8080

    if argc == 2:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print("ERROR: Invalid port number")
            help_and_exit()

    serve(port)
