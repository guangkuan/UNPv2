#include <stdio.h>

int main()
{
    int array1[5] = {0, 1, 2, 3, 4};
    int* begin = array1, * end = &array1[3];
    printf("start with %d, stop with %d\n", *begin, *end);
    int* pa1 = array1;
    int i = 0;
    int* pointer1[5] = {};
    while (i < 5)
    {
        pointer1[i] = array1 + i;
        i += 1;
    }
    int* pointer2 = pointer1[0];
    int* pointer3 = &pointer1[4];
    printf("begin with %#x, end with %#x\n", pointer2, *pointer3);

    return 0;
}