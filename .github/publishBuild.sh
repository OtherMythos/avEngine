#!/bin/bash
set -euo pipefail

JOB=$1
STATUS=$2
shift 2

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
COMMIT=$(git -C "$REPO_DIR" rev-parse HEAD)
STAMP=$(TZ=UTC git -C "$REPO_DIR" log -1 --format=%cd --date=format-local:%Y%m%d-%H%M%S)
BUILD_ID="$STAMP-${COMMIT:0:7}"

PYTHON=$(command -v python3 || command -v python)
IP=$("$PYTHON" -c 'import socket, sys; print(socket.gethostbyname(sys.argv[1]))' ftp.othermythos.com)
URL="ftp://ftp.hstgr.io/$BUILD_ID/"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
escape(){ printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g'; }
printf 'user = "%s:%s"\n' "$(escape "$BUILDS_FTP_USER")" "$(escape "$BUILDS_FTP_PASSWORD")" > "$TMP/curlrc"
chmod 600 "$TMP/curlrc"

upload(){
    curl -sS --ssl-reqd --retry 3 --ftp-create-dirs --resolve "ftp.hstgr.io:21:$IP" -K "$TMP/curlrc" -T "$1" "$URL$2"
}

case "$STATUS" in
    ok)
        : > "$TMP/marker"
        for arg in "$@"; do
            path=${arg%=*}
            name=${arg##*=}
            [ "$path" = "$arg" ] && name=$(basename "$arg")
            echo "Uploading $path as $BUILD_ID/$name"
            upload "$path" "$name"
            echo "$name" >> "$TMP/marker"
        done
        upload "$TMP/marker" "$JOB.done"
        ;;
    failed)
        echo "${GITHUB_SERVER_URL:-https://github.com}/${GITHUB_REPOSITORY:-}/actions/runs/${GITHUB_RUN_ID:-}" > "$TMP/marker"
        upload "$TMP/marker" "$JOB.failed"
        ;;
    *)
        echo "usage: $0 <job> ok [path[=name]]... | $0 <job> failed" >&2
        exit 2
        ;;
esac
echo "Published $JOB ($STATUS) to https://builds.othermythos.com/avEngine/ as $BUILD_ID"
