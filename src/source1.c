#include <source1.h>
#include <source2.h>
int yourlib_add(int x, int y) {
    printf("yourlib_add(%d, %d)\n", x, y);
    return internal_add(x, y);
}
void yourlib_test_print(void) {
    printf("printf called from YourLib\n");
}
