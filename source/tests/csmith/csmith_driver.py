#!/usr/bin/env python3
#
# SPDX-License-Identifier: GPL-3.0
#
# Copyright (C) 2020-2025  Jevgenijs Protopopovs
#
# This file is part of Kefir project.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import sys
import os
import argparse
import subprocess
import tempfile
import functools
import stat
import signal
import ctypes
import time
import multiprocessing as mp
from typing import Optional, Tuple
from dataclasses import dataclass
import traceback

class CSmith:
    def __init__(self, csmith_executable_path: str, seed: Optional[int]):
        self._csmith = csmith_executable_path
        self._seed = seed
    
    def __call__(self) -> str:
        argv = [self._csmith, '--no-packed-struct']
        if  self._seed is not None:
            argv.append('--seed')
            argv.append(str(self._seed))
        return subprocess.check_output(argv, shell=False).decode()

    @property
    def seed(self) -> Optional[int]:
        return self._seed

    @functools.cached_property
    def include_path(self) -> str:
        return os.path.realpath(
            os.path.join(os.path.dirname(self._csmith), '..', 'include')
        )

    def __str__(self):
        return self._csmith

class Kefir:
    def __init__(self, kefir_executable_path: str, csmith_include_path: str):
        self._kefir = kefir_executable_path
        self._csmith_include = csmith_include_path

    def __call__(self, tmpdir: str, code: str) -> bytes:
        with tempfile.NamedTemporaryFile(mode='r+b', dir=tmpdir) as out:
            subprocess.run([self._kefir,
                '-O1', '-fPIC',
                '-I', self._csmith_include,
                '-o', out.name, '-'],
                input=code.encode(), shell=False, check=True)
            return out.read()

    def __str__(self):
        return self._kefir

class CC:
    def __init__(self, cc_executable_path: str, csmith_include_path: str):
        self._cc = cc_executable_path
        self._csmith_include = csmith_include_path

    def __call__(self, tmpdir: str, code: str) -> bytes:
        with tempfile.NamedTemporaryFile(mode='r+b', dir=tmpdir) as out:
            with tempfile.NamedTemporaryFile(mode='w', suffix='.c', dir=tmpdir) as input:
                input.write(code)
                input.flush()
                subprocess.run([self._cc,
                    '-w', '-O2', '-fPIC',
                    '-I', self._csmith_include,
                    '-o', out.name, input.name],
                    shell=False, check=True)
            return out.read()

    def __str__(self) -> str:
        return self._cc

class ExecutableRunner:
    def __init__(self, executable: bytes):
        self._executable = executable

    def __call__(self, tmpdir: str, timeout: Optional[int] = None) -> Tuple[bool, str]:
        with tempfile.NamedTemporaryFile(mode='w+b', delete_on_close=False, dir=tmpdir) as exe:
            exe.write(self._executable)
            exe.close()
            os.chmod(exe.name, os.stat(exe.name).st_mode | stat.S_IEXEC)
            try:
                return True, subprocess.check_output([exe.name], shell=False, timeout=timeout).decode()
            except subprocess.TimeoutExpired:
                return False, None

@dataclass
class TestResult:
    worker_id: int
    test_index: int
    timestamp: int
    test_code: Optional[str]
    success: bool

class TestDriver:
    def __init__(self, csmith: CSmith, kefir: Kefir, cc: CC, timeout: int, parallel_jobs: int = 1):
        self._csmith = csmith
        self._kefir = kefir
        self._cc = cc
        self._timeout = timeout
        self._parallel_jobs = parallel_jobs
        self._pool = None

    def run(self, num_of_tests: int, tmpdir: str, outdir: str, save_all: bool) -> bool:
        if not os.path.isdir(outdir):
            os.mkdir(outdir)
        begin_time = int(time.time() * 1000)
        print('CSmith driver for Kefir', file=sys.stderr)
        print(f'\tcsmith={self._csmith}', file=sys.stderr)
        print(f'\tkefir={self._kefir}', file=sys.stderr)
        print(f'\tcc={self._cc}', file=sys.stderr)
        print(f'\tseed={self._csmith.seed}', file=sys.stderr)
        print(f'\ttimeout={self._timeout}', file=sys.stderr)
        print(f'\tparallel jobs={self._parallel_jobs}', file=sys.stderr)
        print(f'\ttests={num_of_tests}', file=sys.stderr)
        print(f'\ttmpdir={tmpdir}', file=sys.stderr)
        print(f'\toutdir={outdir}', file=sys.stderr)
        print(f'\tsave all={save_all}', file=sys.stderr)
        print(f'\ttimestamp={begin_time}', file=sys.stderr)

        workers = list()
        mgr = mp.Manager()
        results = mgr.Queue()
        tasks_counter = mgr.Value(ctypes.c_ulong, 0)
        tasks_counter_lock = mgr.Lock()
        for worker_id in range(self._parallel_jobs):
            workers.append(self._pool.apply_async(TestDriver._run_worker, [worker_id, tasks_counter, tasks_counter_lock, results, tmpdir, num_of_tests, self._timeout, self._csmith, self._kefir, self._cc]))
        
        num_of_results = 0
        num_failed = 0
        while num_of_results < num_of_tests:
            result = results.get()
            num_of_results += 1
            filename = f'{begin_time}_{result.test_index}.c' if result.success else f'{begin_time}_{result.test_index}_fail.c'
            if not result.success or save_all:
                with open(os.path.join(outdir, filename), 'w') as out:
                    out.write(result.test_code)
            if not result.success:
                num_failed += 1
            print(f'[{int(result.timestamp - begin_time) / 1000:12.3f}] total={num_of_results}; failed={num_failed}', file=sys.stderr)

        for worker in workers:
            worker.wait()

        return num_failed == 0

    @staticmethod
    def _init_worker():
        signal.signal(signal.SIGINT, signal.SIG_IGN)

    @staticmethod
    def _run_worker(worker_id: int, tasks_counter: mp.Value, tasks_counter_lock: mp.Lock, results: mp.Queue, tmpdir: str, num_of_tests: int, timeout: int, csmith: CSmith, kefir: Kefir, cc: CC):
        try:
            while True:
                with tasks_counter_lock:
                    if tasks_counter.value == num_of_tests:
                        break
                    else:
                        test_idx = tasks_counter.value
                        tasks_counter.value += 1
                results.put(TestDriver._run_test(worker_id, test_idx, tmpdir, timeout, csmith, kefir, cc))
        except:
            traceback.print_exc()
    
    @staticmethod
    def _run_test(worker_id: int, test_index: int, tmpdir: str, timeout: int, csmith: CSmith, kefir: Kefir, cc: CC):
        while True:
            test_code = None
            try:
                test_code = csmith()
                kefir_exe = ExecutableRunner(kefir(tmpdir, test_code))
                cc_exe = ExecutableRunner(cc(tmpdir, test_code))
                kefir_finished, kefir_result = kefir_exe(tmpdir, timeout)
                if not kefir_finished:
                    continue
                cc_finished, cc_result = cc_exe(tmpdir, timeout)
                if not cc_finished:
                    continue
                return TestResult(worker_id=worker_id, test_index=test_index, timestamp=time.time() * 1000, test_code=test_code, success=(kefir_result == cc_result))
            except:
                traceback.print_exc()
                return TestResult(worker_id=worker_id, test_index=test_index, timestamp=time.time() * 1000, test_code=test_code, success=False)
    
    def __enter__(self, *args, **kwargs):
        self._pool = mp.Pool(self._parallel_jobs, TestDriver._init_worker)
        return self

    def __exit__(self, *args, **kwargs):
        self._pool.close()
        self._pool.terminate()

args_parser = argparse.ArgumentParser(
    prog='CSmith driver for Kefir'
)
args_parser.add_argument('--csmith', type=str, required=True, help='Path to csmith executable')
args_parser.add_argument('--kefir', type=str, required=True, help='Path to kefir executable')
args_parser.add_argument('--cc', type=str, default='cc', help='Path to reference C compiler')
args_parser.add_argument('--timeout', type=int, default=5, help='Test timeout before retrying')
args_parser.add_argument('--tests', type=int, required=True, help='Number of tests')
args_parser.add_argument('--jobs', type=int, default=1, help='Number of parallel jobs')
args_parser.add_argument('--out', type=str, required=True, help='Directory for tests')
args_parser.add_argument('--seed', type=int, default=None, help='Test seed')
args_parser.add_argument('--save-all', action=argparse.BooleanOptionalAction, default=False, help='Save test code for successful runs too')

if __name__ == '__main__':
    args = args_parser.parse_args(sys.argv[1:])
    if args.seed is not None:
        if args.tests > 1:
            print('Cannot have more than one test with predefined seed', file=sys.stderr)
            sys.exit(-1)
    if not args.csmith:
        print('Expected valid path to csmith executable')
        sys.exit(-1)
    if not args.kefir:
        print('Expected valid path to kefir executable')
        sys.exit(-1)
    if not args.cc:
        print('Expected valid reference C compiler')
        sys.exit(-1)
    csmith = CSmith(args.csmith, args.seed)
    kefir = Kefir(args.kefir, csmith.include_path)
    cc = CC(args.cc, csmith.include_path)
    try:
        with tempfile.TemporaryDirectory() as tmpdir:
            with TestDriver(csmith, kefir, cc, args.timeout, args.jobs) as tests:
                if not tests.run(args.tests, tmpdir, args.out, args.save_all):
                    sys.exit(-1)
    except KeyboardInterrupt:
        print('Interruped', file=sys.stderr)
        sys.exit(-1)
    except SystemExit:
        raise
    except:
        traceback.print_exc()
        sys.exit(-1)