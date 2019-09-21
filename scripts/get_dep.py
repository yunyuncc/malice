#!/usr/bin/python3
import subprocess
def run(cmd):
    res = subprocess.check_call(cmd, shell=True)
    return res

run("apt install -y libspdlog-dev")
run("apt install -y doctest-dev")
