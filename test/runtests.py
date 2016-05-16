#!/usr/bin/env python
#-------------------------------------------------------------------------------------------------------
# Copyright (C) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
#-------------------------------------------------------------------------------------------------------

from __future__ import print_function
import sys
import os
import subprocess as SP
import argparse
import xml.etree.ElementTree as ET
import re

# handle command line args
parser = argparse.ArgumentParser(
    description='ChakraCore *nix Test Script',
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog='''\
Samples:

test all folders:
    runtests.py

test only Array:
    runtests.py Array

test a single file:
    runtests.py Basics/hello.js
''')

parser.add_argument('folders', metavar='folder', nargs='*',
                    help='folder subset to run tests')
parser.add_argument('-b', '--binary', metavar='bin', help='ch full path')
parser.add_argument('-d', '--debug', action='store_true',
                    help='use debug build');
parser.add_argument('-t', '--test', action='store_true', help='use test build')
parser.add_argument('--include-slow', action='store_true',
                    help='include slow tests')
parser.add_argument('--only-slow', action='store_true',
                    help='run only slow tests')
parser.add_argument('--nightly', action='store_true',
                    help='run as nightly tests')
parser.add_argument('--x86', action='store_true', help='use x86 build')
parser.add_argument('--x64', action='store_true', help='use x64 build')
args = parser.parse_args()


test_root = os.path.dirname(os.path.realpath(__file__))
repo_root = os.path.dirname(test_root)

# arch: x86, x64
arch = 'x86' if args.x86 else ('x64' if args.x64 else None)
if arch == None:
    arch = os.environ.get('_BuildArch', 'x86')

# flavor: debug, test, release
type_flavor = {'chk':'debug', 'test':'test', 'fre':'release'}
flavor = 'debug' if args.debug else ('test' if args.test else None)
if flavor == None:
    flavor = type_flavor[os.environ.get('_BuildType', 'fre')]

# binary: full ch path
binary = args.binary
if binary == None:
    if sys.platform == 'win32':
        binary = 'Build/VcBuild/bin/{}_{}/ch.exe'.format(arch, flavor)
    else:
        binary = 'BuildLinux/ch'
    binary = os.path.join(repo_root, binary)
if not os.path.isfile(binary):
    print('{} not found. Did you run ./build.sh already?'.format(binary))
    sys.exit(1)

# global tags/not_tags
tags = set()
not_tags = set(['fail'])
if args.only_slow:
    tags.add('Slow')
elif not args.include_slow:
    not_tags.add('Slow')

not_tags.add('exclude_nightly' if args.nightly else 'nightly')


# records pass_count/fail_count
class PassFailCount(object):
    def __init__(self):
        self.pass_count = 0
        self.fail_count = 0

    def __str__(self):
        return 'passed {}, failed {}'.format(self.pass_count, self.fail_count)

# records total and individual folder's pass_count/fail_count
class TestResult(PassFailCount):
    def __init__(self):
        super(self.__class__, self).__init__()
        self.folders = {}

    def _get_folder_result(self, folder):
        r = self.folders.get(folder)
        if not r:
            r = PassFailCount()
            self.folders[folder] = r
        return r

    def log(self, filename=None, folder=None, fail=False):
        if not folder:
            folder = os.path.basename(os.path.dirname(filename))
        r = self._get_folder_result(folder)
        if fail:
            r.fail_count += 1
            self.fail_count += 1
        else:
            r.pass_count += 1
            self.pass_count += 1


# test variants:
#   interpreted: -maxInterpretCount:1 -maxSimpleJitRunCount:1 -bgjit-
#   dynapogo: -forceNative -off:simpleJit -bgJitDelay:0
class TestVariant(object):
    def __init__(self, name, compile_flags=[]):
        self.name = name
        self.compile_flags = [
            '-WERExceptionSupport', '-ExtendedErrorStackForTestHost'] \
            + compile_flags
        self.tags = tags.copy()
        self.not_tags = not_tags.union(
            ['{}_{}'.format(x, name) for x in 'fails','excludes'])
        self.test_result = TestResult()

    def should_test(self, test):
        tags = test.get('tags')
        if tags:
            tags = set(tags.split(','))
            if not tags.isdisjoint(self.not_tags):
                return False
            if self.tags and not tags.issubset(self.tags):
                return False
        return True

    def log_result(self, filename=None, folder=None, fail=False):
        self.test_result.log(filename, folder, fail)

    def show_failed(self, filename, output, exit_code, expected_output):
        print("\nFailed -> {}".format(filename))
        if expected_output == None:
            print("\nOutput:")
            print("----------------------------")
            print(output)
            print("----------------------------")
        else:
            lst_output = output.split('\n')
            lst_expected = expected_output.split('\n')
            ln = min(len(lst_output), len(lst_expected))
            for i in range(0, ln):
                if lst_output[i] != lst_expected[i]:
                    print("Output: (at line " + str(i) + ")")
                    print("----------------------------")
                    print(lst_output[i])
                    print("----------------------------")
                    print("Expected Output:")
                    print("----------------------------")
                    print(lst_expected[i])
                    print("----------------------------")
                    break

        print("exit code: {}".format(exit_code))
        self.log_result(filename, fail=True)

    def test_one(self, folder, test):
        js_file = os.path.join(folder, test['files'])
        js_output = ""

        working_path = os.path.dirname(js_file)
        file_name = os.path.basename(js_file)

        flags = test.get('compile-flags')
        flags = self.compile_flags + (flags.split() if flags else [])
        cmd = [binary] + flags + [file_name]
        p = SP.Popen(cmd, stdout=SP.PIPE, stderr=SP.STDOUT, cwd=working_path)
        js_output = p.communicate()[0].replace('\r','')
        exit_code = p.wait()

        if exit_code != 0:
            return self.show_failed(js_file, js_output, exit_code, None)

        if 'baseline' not in test:
            # output lines must be 'pass' or 'passed' or empty
            lines = (line.lower() for line in js_output.split('\n'))
            if any(line != '' and line != 'pass' and line != 'passed'
                    for line in lines):
                return self.show_failed(js_file, js_output, exit_code, None)
        else:
            baseline = test.get('baseline')
            if baseline:
                # perform baseline comparison
                baseline = os.path.join(working_path, baseline)
                expected_output = None
                with open(baseline, 'r') as bs_file:
                    baseline_output = bs_file.read()

                # Cleanup carriage return
                # todo: remove carriage return at the end of the line
                #       or better fix ch to output same on all platforms
                expected_output = re.sub('[\r]+\n', '\n', baseline_output)

                # todo: implement wild cards support
                if expected_output != js_output:
                    return self.show_failed(js_file, js_output, exit_code, expected_output)

        print("\tPassed -> " + os.path.basename(js_file))
        self.log_result(js_file)

    def run(self, tests):
        print('\n############# Starting {} variant #############'.format(
            self.name))
        if self.tags:
            print('  tags: {}'.format(self.tags))
        for x in self.not_tags:
            print('  exclude: {}'.format(x))
        print()

        tests = [x for x in tests if self.should_test(x[1])]
        for folder,test in tests:
            self.test_one(folder, test)

    def print_summary(self):
        print('\n######## Logs for {} variant ########'.format(self.name))
        for folder, result in sorted(self.test_result.folders.items()):
            print('{}: {}'.format(folder, result))
        print('Total: {}'.format(self.test_result))


def load_tests(folder, file):
    try:
        xmlpath = os.path.join(folder, 'rlexe.xml')
        xml = ET.parse(xmlpath).getroot()
    except IOError:
        return []

    def load_test(testXml):
        test = dict()
        for c in testXml.find('default'):
            test[c.tag] = c.text
        return test

    tests = [load_test(x) for x in xml]
    if file != None:
        tests = [x for x in tests if x['files'] == file]
        if len(tests) == 0 and is_jsfile(file):
            tests = [{'files':file}]
    return tests

def is_jsfile(path):
    return os.path.splitext(path)[1] == '.js'

def main():
    # By default run all tests
    if len(args.folders) == 0:
        files = (os.path.join(test_root, x) for x in os.listdir(test_root))
        args.folders = [f for f in sorted(files) if not os.path.isfile(f)]

    # load all tests
    tests = []
    for path in args.folders:
        if os.path.isfile(path):
            folder, file = os.path.dirname(path), os.path.basename(path)
        else:
            folder, file = path, None
        tests += ((folder,test) for test in load_tests(folder, file))

    # test variants
    variants = [
        TestVariant('interpreted', [
            '-maxInterpretCount:1', '-maxSimpleJitRunCount:1', '-bgjit-'])
    ]

    # run each variant
    for variant in variants:
        variant.run(tests)

    # print summary
    for variant in variants:
        variant.print_summary()

    print()
    failed = any(variant.test_result.fail_count > 0 for variant in variants)
    print('Success!' if not failed else 'Failed!')
    return 0

if __name__ == '__main__':
    sys.exit(main())
