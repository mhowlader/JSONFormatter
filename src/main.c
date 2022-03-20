#include <stdio.h>
#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv)) {
        //printf("fail");
        USAGE(*argv, EXIT_FAILURE);
    }
    if(global_options == HELP_OPTION) {
        USAGE(*argv, EXIT_SUCCESS);
    }
    ARGO_VALUE *read_result = argo_read_value(stdin);

    if (global_options == VALIDATE_OPTION) { //if validate
        if (read_result==NULL) { //if read returns nuull
            //printf("\nfail\n");
            exit(EXIT_FAILURE);
        }
        //printf("\nsuccess\n");
        exit(EXIT_SUCCESS);
    }
    //returns 0 if succcessful
    if (read_result==NULL) {
        //printf("\nread fail\n");
        exit(EXIT_FAILURE);
    }

    indent_level = 0;
    //canonicalize mode
    if (argo_write_value(read_result,stdout)==0) {
        //fputc(ARGO_LF,stdout);
        //printf("\nwrite success\n");
        exit(EXIT_SUCCESS);
    }
    //printf("\nwrite failure\n");
    exit(EXIT_FAILURE);

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
