import subprocess
import os
def run(cmd):
    res = subprocess.check_call(cmd, shell=True)

if not os.path.exists("build"):
    os.mkdir("build")

os.chdir("build")
run("cmake ..")
run("make")
run("make test")
run("rm -rf gcov_out")
run("rm -f coverage.info")
run("lcov -b . --capture --directory .  --output-file coverage.info")
run("genhtml coverage.info --output-directory gcov_out")
pwd = os.getcwd()
html_path = pwd + "/gcov_out"
print(html_path)
run("rm -f /www")
cmd = "ln -s " + html_path + " /www"
run(cmd)


