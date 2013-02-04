// test path tools

#include <string.h>
#include <stdio.h>

#include "utest.h"
#include "ah5.h"
#include "ah5_inmemory.h"


int tests_run = 0;

static char *test_create()
{
    hid_t file_id;
    file_id = AH5_create_inmemory_file("a_valid_file_name");
}

static char *all_tests()
{
    mu_run_test(test_create);
    return 0;
}

int main(int argc, char **argv)
{
    char *result = all_tests();
    if (result != 0)
    {
        printf("%s\n", result);
    }
    else
    {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}


