#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */


//compare that the string is a certain flag, 0 if false, 1 if true
int flagCompare(char *arg,char c) {
    if (*arg !='-') {
        return 0;
    }
    arg++;
    if (*arg!=c) {
        return 0;
    }
    arg++;
    if (*arg=='\0') {
        return 1;
    }
    return 0;
}

//returns -1 if invalid int, otherwise returns the int
int nonNegInt(char *arg) {
    int val = 0;
    while (*arg!='\0') { //loop until end of string
        int digit = *arg-'0'; //get the digit value
        if (digit<0 || digit >9) { //if not an digit then error
            return -1;
        }
        val= val*10 + digit;
        arg++;
    }
    if (val>=0 && val < 256) { //make sure val is between 0 and 255
        return val;
    } //if not then invalid int
    return -1;
}

int validargs(int argc, char **argv) {
    if (argc==1) { //if arg count is 1 then no arguments so return -1
        return -1;
    }

    //for all arguments
    for (int i = 0;i<argc-1;i++) {
        argv++;
        if (i==0) { //first argument
            if (flagCompare(*argv,'h')) { //if help then return immediately
                global_options = HELP_OPTION;
                return 0;
            } else if (flagCompare(*argv,'v')) { //if v then set to v
                global_options = VALIDATE_OPTION;
            } else if (flagCompare(*argv,'c')) { //if h then set to h
                global_options = CANONICALIZE_OPTION;
            } else {
                return -1;
            }
        } else if (i==1) { //second argument can ONLY be pretty print
            if (global_options == CANONICALIZE_OPTION) { //if in canonicalize mode then check if next flag is -p
                if (flagCompare(*argv,'p')) {
                    global_options+=PRETTY_PRINT_OPTION;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else if (i==2) { //third argument can ONLY be INDENT
            if (global_options == CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) { //if -c and -p, then
                int nonNegRes = nonNegInt(*argv); //result of verifying the indent is a valid non negative int
                if (nonNegRes==-1) { //if the indent is not a valid non negative int then error
                    return -1;
                }
                global_options+=nonNegRes; //add the indent value to global_options
            } else { //if global options is not in -c and -p then erro
                return -1;
            }
        } else { //if more than 3 arguments then error, never more than 3 arguments
            return -1;
        }
    }
    if (global_options == CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) { //if -c -p and no INDENT then default is 4.
        global_options+=4;
    }
    return 0;

}


