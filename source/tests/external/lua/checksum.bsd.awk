SUCCESS=0

$4 == expected_checksum {
    print filename " checksum check: ok"
    SUCCESS=1
    exit 0
}

END {
    if (SUCCESS != 1) {
        print filename " checksum check: failed"
        exit 1
    }
}