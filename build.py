#!/usr/bin/python3
import subprocess
import os
def run(cmd):
    res = subprocess.check_call(cmd, shell=True)
build_dir = "malice/build"
base_dir = os.getcwd()
if not os.path.exists(build_dir):
    os.mkdir(build_dir)

os.chdir(build_dir)
full_build_dir=os.getcwd()
run("cmake " + base_dir)
run("make")
run("make test")
run("rm -rf gcov_out")
run("rm -f coverage.info")
os.chdir(base_dir)
run("lcov -b ./malice --no-external  --capture --directory ./malice --output-file malice/build/coverage.info")
run("genhtml malice/build/coverage.info --output-directory malice/build/gcov_out")
html_path = full_build_dir + "/gcov_out"
print(html_path)
run("rm -f /www")
cmd = "ln -s " + html_path + " /www"
run(cmd)


