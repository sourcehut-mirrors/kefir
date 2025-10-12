#!/usr/bin/env bash

set -e

./opc --technology opc --export svg --cell basic/mosfet
sha256sum openPCells.svg

cd examples/svg
./run.sh
sha256sum gilfoyle.gds
sha256sum pineapple.gds
sha256sum talentandsweat.gds