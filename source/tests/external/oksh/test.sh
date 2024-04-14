set -e
set -o pipefail

TMP_DIR="$(mktemp -d)"

cleanup () {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT HUP INT QUIT PIPE TERM

mkdir -p "$TMP_DIR"/dir1/{dir2,dir3}/dir4
find "$TMP_DIR" -mindepth 1 -type d -exec touch "{}/.file" \;

cd "$TMP_DIR" && find .
if [ ! -e "$TMP_DIR/dir1/.file" ] || [ ! -e "$TMP_DIR/dir1/dir2/.file" ]; then
    echo "ERROR"
    exit 1
fi
