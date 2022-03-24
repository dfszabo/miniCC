// COMPILE-FAIL


struct A {
  struct B {
    int B_data;
  } B_member;
} A_obj;

int test() {
  A_obj.B_member.B_data = 23;
  return A_obj.B_member.B_data;
}
