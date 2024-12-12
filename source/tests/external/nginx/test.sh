#!/usr/bin/env bash

NGINX_DIR="$1"
OUTPUT_FILE="$2"

set -e

kill_nginx () {
    "$NGINX_DIR/objs/nginx" -p "$NGINX_DIR" -c "conf/test.conf" -s quit
}

trap kill_nginx EXIT HUP INT QUIT PIPE TERM

rm -rf "$NGINX_DIR/logs"
mkdir "$NGINX_DIR/logs"

sed "s/listen[ ]*80;/listen 8081;/g" "$NGINX_DIR/conf/nginx.conf" > "$NGINX_DIR/conf/test.conf"

"$NGINX_DIR/objs/nginx" -p "$NGINX_DIR" -c "conf/test.conf"
sleep 1
wget -q -O - "http://localhost:8081" > "$OUTPUT_FILE"
