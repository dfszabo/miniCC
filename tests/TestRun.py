import os
import re
import subprocess
from os import path

workdir = "/home/david/work/mini-c-compiler-cpp/tests"


def check_file(file_name):
    arch = ""
    function_declarations = []
    test_cases = []
    with open(file_name) as file:
        for line in file:
            m = re.search(r'(?:/{2}|#) *RUN: (.*)', line)
            if m:
                arch = m.group(1).lower()

            m = re.search(r'(?:/{2}|#) *FUNC-DECL: (.*)', line)
            if m:
                function_declarations.append(m.group(1))

            m = re.search(r'(?:/{2}|#) *TEST-CASE: (.*) -> (\d+)', line)
            if m:
                test_cases.append((m.group(1), m.group(2)))

    if len(test_cases) == 0:
        return False, False

    run_command = "qemu-" + arch
    if arch == "":
        return False, True

    test_main_c_template = ""
    for func_decl in function_declarations:
        test_main_c_template += func_decl + ";\n"

    test_main_c_template += "int main() { return $; }"

    ret_code = subprocess.run(["../build/miniCC", file_name], stdout=subprocess.DEVNULL, timeout=10).returncode
    if ret_code != 0:
        return False, True

    test_asm = subprocess.check_output(["../build/miniCC", file_name]).decode("utf-8")

    text_file = open("test.s", "w")
    n = text_file.write(test_asm)
    text_file.close()

    for case, expected_result in test_cases:
        current_test_main = test_main_c_template
        current_test_main = current_test_main.replace("$", case + " == " + expected_result)

        text_file = open("test_main.c", "w")
        n = text_file.write(current_test_main)
        text_file.close()

        compile_ret = subprocess.run([arch + "-linux-gnu-gcc", "test_main.c", "test.s", "-o", "test", "-static"]).returncode
        if compile_ret != 0:
            if path.exists("test_main.c"):
                os.remove("test_main.c")
            if path.exists("test.s"):
                os.remove("test.s")
            if path.exists("test"):
                os.remove("test")
            return False, True

        ret_code = subprocess.run([run_command, "test"]).returncode

        if ret_code == 0:
            os.remove("test_main.c")
            os.remove("test.s")
            os.remove("test")
            return False, True

    if path.exists("test_main.c"):
        os.remove("test_main.c")
    if path.exists("test.s"):
        os.remove("test.s")
    if path.exists("test"):
        os.remove("test")
    return True, True


failed_tests = []
tests_count = 0
passed_tests_count = 0
for subdir, dirs, files in os.walk(workdir):
    for filename in files:
        filepath = subdir + os.sep + filename

        if filepath.endswith(".c") or filepath.endswith(".s"):
            success, has_cases = check_file(filepath)
            if not has_cases:
                continue

            tests_count += 1
            if success:
                passed_tests_count += 1
                print(filepath + "...PASS")
            else:
                failed_tests.append(filepath)
                print(filepath + "...FAIL")

print("\n--------", tests_count, "Test executed --------")
print("\t\t", passed_tests_count, "PASS")
print("\t\t", len(failed_tests), "FAIL\n")

for test_case in failed_tests:
    print(test_case)

