/*
  SPDX-License-Identifier: GPL-3.0

  Copyright (C) 2020-2025  Jevgenijs Protopopovs

  This file is part of Kefir project.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, version 3.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

importScripts('kefir.js');

let includeList = null;
let kefirWasm = null;
const implicitArgs = ['--target', 'x86_64-linux-musl', '-I', '/includes/extra'];

async function kefir(args, input) {
  if (!includeList) {
    includeList = await fetch("include.list")
      .then(res => res.text())
      .then(res => res.split('\n'))
      .then(res => res.filter(el => el.length > 0));
  }

  if (!kefirWasm) {
    kefirWasm = await fetch('kefir.wasm')
      .then(res => res.arrayBuffer());
  }

  let stdoutBuf = "";
  let stderrBuf = "";
  
  const kefir = await createKefirModule({
      preRun: function(kefir) {
        let input_index = 0;
        function stdin() {
          if (input_index < input.length) return input.charCodeAt(input_index++);
          else return null;
        }

        function stdout(code) {
          stdoutBuf += String.fromCharCode(code);
        }

        function stderr(code) {
          stderrBuf += String.fromCharCode(code);
        }

        function mkdir_p(dir) {
          for (let separator = dir.indexOf('/'); separator != -1; separator = dir.indexOf('/', separator + 1)) {
            const parentDir = dir.substring(0, separator);
            const parentDirInfo = kefir.FS.analyzePath(parentDir);
            if (!parentDirInfo.exists) {
              kefir.FS.mkdir(parentDir);
            }
          }
          const dirInfo = kefir.FS.analyzePath(dir);
          if (!dirInfo.exists) {
            kefir.FS.mkdir(dir);
          }
        }

        kefir.FS.init(stdin, stdout, stderr);
        for (let includeFile of includeList) {
          const basenameSep = includeFile.lastIndexOf('/');
          const dirname = includeFile.substring(0, basenameSep);
          const filename = includeFile.substring(basenameSep + 1);
          
          mkdir_p(`/includes/${dirname}`);
          kefir.FS.createLazyFile(`/includes/${dirname}`, filename, `includes/${dirname}/${filename}`, true, false);
        }

        kefir.ENV.KEFIR_RTINC="/includes/kefir";
        kefir.ENV.KEFIR_MUSL_INCLUDE="/includes/musl";
      },

      instantiateWasm: (info, receiveInstance) => {
        WebAssembly.instantiate(kefirWasm, info)
          .then(output => {
                  receiveInstance(output['instance']);
                },
                err => {
                  console.error(`wasm instantiation failed: ${err}`);
                });
          return {};
      }
  });
  
  const stringToC = s => {
      var size = kefir.lengthBytesUTF8(s) + 1;
      var ret = kefir._malloc(size);
      kefir.stringToUTF8Array(s, kefir.HEAP8, ret, size);
      return ret;
  }
  
  const run_with_args = args => {
      const cStrings = args.map(x => stringToC(x));

      const cStringSizeof = 4;
      const cArgs = kefir._malloc(cStrings.length * cStringSizeof);
      cStrings.forEach((x, i) => {
        kefir.setValue(cArgs + i * cStringSizeof, x, "i32");
      });

      const rc = kefir.ccall('kefir_run_with_args', 'number', ['number', 'number'], [cStrings.length, cArgs]);

      cStrings.forEach(cStr => {
        kefir._free(cStr);
      })

      kefir._free(cArgs);

      return rc;
  }
  
  const rc = run_with_args(['kefir', ...args]);
  return {
    rc,
    stdout: stdoutBuf,
    stderr: stderrBuf
  };
}

onmessage = async (msg) => {
  try {
    const [args, code] = msg.data;
    const result = await kefir([...implicitArgs, ...args], code);
    postMessage({ success: true, result });
  } catch (err) {
    postMessage({ success: false, error: err});
  }
};

