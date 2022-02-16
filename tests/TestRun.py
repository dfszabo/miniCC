import os
import re
import subprocess
from os import path
import sys

save_temps = False

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

            m = re.search(r'(?:/{2}|#) *TEST-CASE: (.*) -> (.*)', line)
            if m:
                test_cases.append((m.group(1), m.group(2)))

    return arch, function_declarations, test_cases


def execute_tests(file_name, arch, function_declarations, test_cases):
    run_command = "qemu-" + arch
    if arch == "":
        return False

    test_main_c_template = "#include <stdio.h>\n"
    for func_decl in function_declarations:
        test_main_c_template += func_decl + ";\n"

    test_main_c_template += "int main() {"
    test_main_c_template += "  int res = $;"
    test_main_c_template += r'  if (res != @) { printf("\nExpected: %d, Actual: %d\n", @, res); return 1;}'
    test_main_c_template += "  return 0; }"

    ret_code = subprocess.run(["../build/miniCC", file_name], stdout=subprocess.DEVNULL, timeout=10).returncode
    if ret_code != 0:
        return False

    test_asm = subprocess.check_output(["../build/miniCC", file_name]).decode("utf-8")

    text_file = open("test.s", "w")
    text_file.write(test_asm)
    text_file.close()

    for case, expected_result in test_cases:
        current_test_main = test_main_c_template
        current_test_main = current_test_main.replace("$", case)
        current_test_main = current_test_main.replace("@", expected_result)

        text_file = open("test_main.c", "w")
        text_file.write(current_test_main)
        text_file.close()

        compile_ret = subprocess.run([arch + "-linux-gnu-gcc", "test_main.c", "test.s", "-o", "test", "-static"]).returncode
        if compile_ret != 0:
            if not save_temps:
              if path.exists("test_main.c"):
                  os.remove("test_main.c")
              if path.exists("test.s"):
                  os.remove("test.s")
              if path.exists("test"):
                  os.remove("test")
            return False

        ret_code = subprocess.run([run_command, "test"]).returncode

        if ret_code != 0:
            if not save_temps:
              os.remove("test_main.c")
              os.remove("test.s")
              os.remove("test")
            return False

    if not save_temps:
      if path.exists("test_main.c"):
          os.remove("test_main.c")
      if path.exists("test.s"):
          os.remove("test.s")
      if path.exists("test"):
          os.remove("test")
    return True

# clean up files remaining from previous runs
if path.exists("test_main.c"):
  os.remove("test_main.c")
if path.exists("test.s"):
  os.remove("test.s")
if path.exists("test"):
  os.remove("test")

test_set = []
failed_tests = []
tests_count = 0
passed_tests_count = 0

# handle program arguments
for i, arg in enumerate(sys.argv):
  if i == 0:
    continue
  if arg == "-save-temps":
    save_temps = True
  else:
    # treat non flag arguments as test files
    test_set.append(arg)

# if no input files were given, then save all test file from the current directory
if len(test_set) == 0:
  for subdir, dirs, files in os.walk(os.getcwd()):
      for filename in files:
          filepath = subdir + os.sep + filename

          if filepath.endswith(".c") or filepath.endswith(".s"):
              test_set.append(filepath)

test_set.sort() # sort them alphabetically

# run the tests
for filepath in test_set:
  arch, function_declarations, test_cases = check_file(filepath)
  if arch == "":
      continue

  success = execute_tests(filepath, arch, function_declarations, test_cases)

  tests_count += 1
  if success:
      passed_tests_count += 1
      print("PASS :: ", filepath)
  else:
      failed_tests.append(filepath)
      print("FAIL :: ", filepath)

# print test results
print("\n--------", tests_count, "Test executed --------")
print("\t\t", passed_tests_count, " PASS")
print("\t\t", len(failed_tests), " FAIL\n")

for test_case in failed_tests:
    print(test_case)

