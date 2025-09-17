#!/usr/bin/env python3

import os
import sys

def set_cwd_to_repo_root_dir():
    script_path = os.path.abspath(__file__)
    script_dir = os.path.dirname(script_path) + "/../"
    os.chdir(script_dir)

set_cwd_to_repo_root_dir()
if len(sys.argv) > 1 and sys.argv[1] == "--fix" :
    os.system("python3 ./scripts/run-clang-format.py --style file -e **/ThirdParty/** -e **/build/** -r --in-place .")
else:
    os.system("python3 ./scripts/run-clang-format.py --style file -e **/ThirdParty/** -e **/build/** -r .")
