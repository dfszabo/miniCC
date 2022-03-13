import os
import re
import subprocess
from os import path
import sys
from dataclasses import dataclass
from dataclasses import field

save_temps = False

@dataclass
class Context:
    arch: str = ""
    function_declarations: list = field(default_factory=list)
    test_cases: list = field(default_factory=list)
    check_list: list = field(default_factory=list)
    check_not_list: list = field(default_factory=list)
    run_test: bool = False
    run_fail_test: bool = False
    compile_test: bool = False
    negative_test: bool = False
    extra_compile_flags: str = ""

def clean_up():
  if path.exists("test_main.c"):
    os.remove("test_main.c")
  if path.exists("test.s"):
    os.remove("test.s")
  if path.exists("test"):
    os.remove("test")

def check_file(file_name):
    context = Context()

    with open(file_name) as file:
        for line in file:
            m = re.search(r'(?:/{2}|#) *RUN: (.*)', line)
            if m:
                context.arch = m.group(1).lower()
                context.run_test = True

            m = re.search(r'(?:/{2}|#) *RUN-FAIL: (.*)', line)
            if m:
                context.arch = m.group(1).lower()
                context.run_fail_test = True

            m = re.search(r'(?:/{2}|#) *COMPILE-TEST', line)
            if m:
                context.compile_test = True

            m = re.search(r'(?:/{2}|#) *FUNC-DECL: (.*)', line)
            if m:
                context.function_declarations.append(m.group(1))

            m = re.search(r'(?:/{2}|#) *CHECK:\s*(.*)', line)
            if m:
                context.check_list.append(m.group(1))

            m = re.search(r'(?:/{2}|#) *CHECK-NOT:\s*(.*)', line)
            if m:
                context.check_not_list.append(m.group(1))

            m = re.search(r'(?:/{2}|#) *TEST-CASE: (.*) -> (.*)', line)
            if m:
                context.test_cases.append((m.group(1), m.group(2)))

            m = re.search(r'(?:/{2}|#) *COMPILE-FAIL', line)
            if m:
                context.negative_test = True

            m = re.search(r'(?:/{2}|#) *EXTRA-FLAGS: (.*)', line)
            if m:
                context.extra_compile_flags = m.group(1)

    return context


def execute_tests(file_name, context):
    run_command = "qemu-" + context.arch
    if (context.run_test or context.run_fail_test) and (context.arch == "" or len(context.function_declarations) == 0 or
                                                        len(context.test_cases) == 0):
      print("run test requires arch name, function declarations and test cases")
      return False

    if context.compile_test and len(context.check_list) == 0 and len(context.check_not_list) == 0:
      print("no CHECK were given")
      return False

    # create the full command to call the compiler
    command = ["../build/miniCC", file_name]
    if context.extra_compile_flags != "":
      command.append(context.extra_compile_flags)

    # call the compiler
    result = subprocess.run(command, capture_output=True, timeout=10)

    # if the compilation failed
    if result.returncode != 0:
      # if it was a negative test and the fail did not caused by an assertion -> test passed
      if context.negative_test and not re.search(r'(.*): Assertion(.*)', result.stderr.decode()):
        return True
      return False

    # if its a compile test then check the output
    if context.compile_test:
      had_checks = len(context.check_list) > 0
      had_check_nots = len(context.check_not_list) > 0

      for line in result.stdout.decode().splitlines():
        # if CHECKs were given, then check the output for them
        if had_checks:
          if line.find(context.check_list[0]) != -1:
            context.check_list.pop(0)

            # found all check -> success
            if len(context.check_list) == 0:
              return True

        # if there are CHECK-NOTs
        if had_check_nots:
          # then check each CHECK-NOT
          for check_not in context.check_not_list:
            # if found one in the output -> fail
            if line.find(check_not) != -1:
              print("Output contains '", check_not, "', but it should not")
              return False

      # reached this point and had CHECKs -> did not found all check
      # print the next check which was not found
      if had_checks:
        print("Have not found in the output: ", context.check_list[0])
        return False
      # reached this point with CHECK-NOTs -> success
      if had_check_nots:
        return True

    # RUN test case

    # create the assembly file
    text_file = open("test.s", "w")
    text_file.write(result.stdout.decode())
    text_file.close()

    # create the main C file which used for the testing
    test_main_c_template = "#include <stdio.h>\n\n"
    for func_decl in context.function_declarations:
        test_main_c_template += func_decl + ";\n\n"

    test_main_c_template += "int main() {"
    test_main_c_template += "  int res = $;"
    test_main_c_template += "  if (res != @) { "
    test_main_c_template += r'   printf("\nExpected: %d, Actual: %d\n", @, res);'
    test_main_c_template += "    return 1;"
    test_main_c_template += "  }"
    test_main_c_template += "  return 0;"
    test_main_c_template += "}"

    # iterate over all the test cases
    for case, expected_result in context.test_cases:
        current_test_main = test_main_c_template
        # substitute in the test case and the expected value in the template C code (look above for the C code)
        # example: // TEST-CASE: test(1) -> 0
        #   case: test(1), expected_result: 0
        current_test_main = current_test_main.replace("$", case)
        current_test_main = current_test_main.replace("@", expected_result)

        text_file = open("test_main.c", "w")
        text_file.write(current_test_main)
        text_file.close()

        # compile the C file with the generated assembly
        compile_ret = subprocess.run([context.arch + "-linux-gnu-gcc", "test_main.c", "test.s", "-o", "test", "-static", "-lm"]).returncode
        if compile_ret != 0:
            if not save_temps:
              clean_up()
            return False

        # run the generated object file with qemu
        ret_code = subprocess.run([run_command, "test"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT).returncode

        if ret_code != 0 and not context.run_fail_test:
            if not save_temps:
              clean_up()
            return False

    if not save_temps:
      clean_up()
    return True

# clean up files remaining from previous runs
clean_up()

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
  context = check_file(filepath)

  # if no test types were specified, then skip it
  if not context.run_fail_test and not context.run_test and not context.compile_test and not context.negative_test:
    continue

  success = execute_tests(filepath, context)

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

