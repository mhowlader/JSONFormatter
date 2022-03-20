#include <stdlib.h>
#include <stdio.h>
#include <limits.h>


#include "argo.h"
#include "global.h"
#include "debug.h"


int read_top_level = 0;

int valid_char_after_basic(int c) {
    if (argo_is_whitespace(c) || c==ARGO_RBRACE || c== ARGO_COMMA || c==ARGO_RBRACK) {
        return 1;
    }
    return 0;
}


int iterate_white(FILE *f) {
    int iter_readbyte;
    while (1) {
        iter_readbyte = fgetc(f);
        if (!argo_is_whitespace(iter_readbyte)) {
            ungetc(iter_readbyte,f);
            break;
        }
    }
    //printf("char after iterate white: %c\n",iter_readbyte);
    return 0;
}

//0 if successful, -1 if failure
int argo_read_basic(ARGO_VALUE *v,FILE *f) {
    v->type = ARGO_BASIC_TYPE;
    int readbyte = fgetc(f);
    if (readbyte == 't') {
        if ((readbyte=fgetc(f))=='r') {
            if ((readbyte=fgetc(f))=='u') {
                if ((readbyte=fgetc(f))=='e') {
                    // if (valid_char_after_basic((readbyte=fgetc(f)))) {
                    //     ungetc(readbyte,f);
                    //     printf("%c",readbyte);
                    //     v->content.basic = ARGO_TRUE;
                    //     return 0;
                    // }
                    // return -1;  
                    v->content.basic=ARGO_TRUE;
                    return 0;
                }
                return -1;
            }
            return -1;
        }
        return -1;
    }
    else if (readbyte == 'f') {
        if ((readbyte=fgetc(f))=='a') {
            if ((readbyte=fgetc(f))=='l') {
                if ((readbyte=fgetc(f))=='s') {
                    if ((readbyte=fgetc(f))=='e') {
                        // if (valid_char_after_basic((readbyte=fgetc(f)))) {
                        //     ungetc(readbyte,f);
                        //     v->content.basic = ARGO_FALSE;
                        //     return 0;
                        // }
                        // return -1;
                        v->content.basic= ARGO_FALSE;
                        return 0;
                    }
                    return -1;
                }
                return -1;
            }
            return -1;
        }
        return -1;
    }
    else if (readbyte == 'n') {
        if ((readbyte=fgetc(f))=='u') {
            if ((readbyte=fgetc(f))=='l') {
                if ((readbyte=fgetc(f))=='l') {
                    // if (valid_char_after_basic((readbyte=fgetc(f)))) {
                    //     ungetc(readbyte,f);
                    //     v->content.basic = ARGO_NULL;
                    //     return 0;
                    // }
                    // return -1;
                    v->content.basic = ARGO_NULL;
                    return 0;
                }
                return -1;
            }
            return -1;
        }
        return -1;
    }
    return -1;
}



//assumption, whitespace eaten, not called on empty objects
int argo_read_members(ARGO_OBJECT *o,FILE *f) {
    //aalready eaten whitespace
    int membyte;
    while (1) {
        iterate_white(f);
        membyte = fgetc(f);
        if (membyte == ARGO_QUOTE) {//start of new member
            ungetc(membyte,f);

        }

        ARGO_STRING member_name;
        if (argo_read_string(&(member_name),f)) { //initialize and store string in argo_string
            return -1;
        }

        iterate_white(f);

        membyte = fgetc(f);
        if (membyte != ARGO_COLON) {
            fprintf(stderr,"Expecting ':' after member name\n");
            return -1;
        }


        ARGO_VALUE *mem_val = argo_read_value(f); //get element of object //ate white speace
        if (mem_val == NULL) {return -1;}

        mem_val->prev = o->member_list->prev; //set new element's previous to the sentinel's previous
        o->member_list->prev->next=mem_val; //set the previous last element's next to new element (if empty then set the sentinel to new)
        o->member_list->prev = mem_val; //set the sentinel's previous to new element
        mem_val->next = o->member_list; //set the new element's next to sentinel 
        mem_val->name = member_name;

        membyte = fgetc(f);
        if (membyte != ARGO_COMMA) { //right brace
            ungetc(membyte,f);
            return 0; //successful
        }
    }
}


int argo_read_object(ARGO_OBJECT *o, FILE *f) {
    //printf("read o\n");
    o->member_list->next=o->member_list; //error over here
    o->member_list->prev=o->member_list; 

    //printf("read o\n");
    int obyte = fgetc(f);
    if (obyte == ARGO_LBRACE) {
        iterate_white(f);
        obyte = fgetc(f);
        if (obyte==ARGO_RBRACE) { //reads left_brace
            return 0;
        }
        else if (obyte!=EOF) { //expects
            ungetc(obyte,f);
            if (argo_read_members(o,f)) { //if error in argo_read_members
                return -1;
            }
        }
        else {
            fprintf(stderr,"Error: Reached End of File before closing object\n");
            return -1;
        }


        obyte = fgetc(f);
        //make sure to fget(c)
        if (obyte == ARGO_RBRACE) { 
            return 0;
        }
        else if (obyte == EOF) {
            fprintf(stderr,"Error: reached end of file before before closing object\n");
            return -1;
        }
        else {
            fprintf(stderr,"Expecting '}' to close object'\n");
            return -1;
        }
    }
    else {
        fprintf(stderr,"Expecting '{' to start object\n");
        return -1;
    }
    return -1;
}

//elements of an array
int argo_read_elements(ARGO_ARRAY *a,FILE *f) {
    //aalready eaten whitespace
    int elebyte;
    while (1) {
        //iterate_white(f);

        ARGO_VALUE *ele_val = argo_read_value(f); //get element of object //ate white speace
        if (ele_val == NULL) {return -1;}

        ele_val->prev = a->element_list->prev; //set new element's previous to the sentinel's previous
        a->element_list->prev->next=ele_val; //set the previous last element's next to new element (if empty then set the sentinel to new)
        a->element_list->prev = ele_val; //set the sentinel's previous to new element
        ele_val->next = a->element_list; //set the new element's next to sentinel

        elebyte = fgetc(f);
        if (elebyte != ARGO_COMMA) { //right brace
            ungetc(elebyte,f);
            return 0; //successful
        }
    }
}

int argo_read_array(ARGO_ARRAY *a, FILE *f) {
    //printf("read o\n");
    a->element_list->next=a->element_list; //error over here
    a->element_list->prev=a->element_list; 

    //printf("read o\n");
    int abyte = fgetc(f);
    if (abyte == ARGO_LBRACK) {
        iterate_white(f);
        abyte = fgetc(f);
        if (abyte==ARGO_RBRACK) { //reads left_brace
            return 0;
        }
        else if (abyte!=EOF) { //expects
            ungetc(abyte,f);
            if (argo_read_elements(a,f)) { //if error in argo_read_members
                return -1;
            }
        }
        else {
            fprintf(stderr,"Error: Reached End of File before closing array\n");
            return -1;
        }


        abyte = fgetc(f);
        //make sure to fget(c)
        if (abyte == ARGO_RBRACK) { 
            return 0;
        }
        else if (abyte == EOF) {
            fprintf(stderr,"Error: reached end of file before before closing array\n");
            return -1;
        }
        else {
            fprintf(stderr,"Expecting ']' to close array'\n");
            return -1;
        }
    }
    else {
        fprintf(stderr,"Expecting '[' to start array\n");
        return -1;
    }
    return -1;
}
/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */
ARGO_VALUE *argo_read_value(FILE *f) {
    iterate_white(f);

    if (argo_next_value>=NUM_ARGO_VALUES) {
        fprintf(stderr, "Exceeded Limit of ARGO_VALUE_STORAGE\n");
        return NULL;
    }
    ARGO_VALUE *new_argo_value = argo_value_storage + argo_next_value;
    argo_next_value++;
    int readbyte = fgetc(f);             
    if (readbyte==ARGO_QUOTE) {
        ungetc(ARGO_QUOTE,f);
        new_argo_value -> type = ARGO_STRING_TYPE;
        if (argo_read_string(&(new_argo_value->content.string),f)) { //if it returns -1 means error
            return NULL;
        }
    }
    else if (readbyte=='t' || readbyte == 'f' || readbyte == 'n') {
        ungetc(readbyte,f);
        new_argo_value -> type = ARGO_BASIC_TYPE;
        if (argo_read_basic(new_argo_value,f)) {

            fprintf(stderr,"Invalid ARGO_BASIC Value\n");
            return NULL;
        }
    }
    else if (readbyte=='-' || argo_is_digit(readbyte)) {
        ungetc(readbyte,f);
        new_argo_value -> type = ARGO_NUMBER_TYPE;
        if (argo_read_number(&(new_argo_value->content.number),f)) {
            fprintf(stderr,"Invalid Number\n");
            return NULL;
        }
        //printf("READNUMBER\n");
    }
    else if (readbyte == ARGO_LBRACE) {
        read_top_level++; // elements made after this are not top level
        //printf("lbrace\n");
        ungetc(readbyte,f);
        new_argo_value -> type = ARGO_OBJECT_TYPE;

        //initailzie sentinal
        if (argo_next_value>=NUM_ARGO_VALUES) {
            return NULL;
        }
        ARGO_VALUE *sentinel = argo_value_storage + argo_next_value;
        argo_next_value++;

        sentinel->type =ARGO_NO_TYPE;

        new_argo_value->content.object.member_list = sentinel;
        if (argo_read_object(&(new_argo_value->content.object),f)) {
            return NULL;
        }
        read_top_level--; //reset the top_level

    }
    else if(readbyte == ARGO_LBRACK) {
        read_top_level++; // elements made after this are not top level
        //printf("lbrace\n");
        ungetc(readbyte,f);
        new_argo_value -> type = ARGO_ARRAY_TYPE;

        //initailzie sentinal
        if (argo_next_value>=NUM_ARGO_VALUES) {
            return NULL;
        }
        ARGO_VALUE *sentinel = argo_value_storage + argo_next_value;
        argo_next_value++;

        sentinel->type =ARGO_NO_TYPE;

        new_argo_value->content.array.element_list = sentinel;
        if (argo_read_array(&(new_argo_value->content.array),f)) {
            return NULL;
        }
        read_top_level--; //reset the top_level
    }
    else if (readbyte==EOF) {
        fprintf(stderr,"Error: Reached End of File when expecting json element\n");
        return NULL;
    }
    else {
        fprintf(stderr,"Invalid character to start a JSON Value: %c\n",readbyte);
        return NULL;
    }

    iterate_white(f);

    // int num_get=fgetc(f);
    // printf("%c\n",num_get);
    // ungetc(num_get,f);

    if (read_top_level==0) {
        //readbyte = fgetc(f);
        //printf("\nchar after top level: %c\n",readbyte);
        //ungetc(readbyte,f);
        // if (iterate_white(f)) {
        //     return NULL;
        // }
        if (fgetc(f)!=EOF) { //after eating white space there should be nothing left
            fprintf(stderr,"Error: There should be no character after top-level JSON Element\n");
            return NULL;
        }
    }
    //printf("read finished argo val\n");
    return new_argo_value;
}

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_string(ARGO_STRING *s, FILE *f) {

    s->capacity = 0;
    s->length = 0;
    s->content = NULL;

    int strbyte = fgetc(f);
    if (strbyte == ARGO_QUOTE) {    
        while(1) {
            ARGO_CHAR readchar = fgetc(f); //get next byte
            if (readchar==ARGO_QUOTE) {
                return 0; //reached end of string, is valid
            }
            else if (readchar == EOF) {
                fprintf(stderr,"Reached End of File without closing quotes\n");
                return -1;
            }
            else if (readchar == ARGO_BSLASH) { //see a \, means escape character
                readchar = fgetc(f);
                if (readchar == EOF) {
                    fprintf(stderr,"End of File Error\n");
                    return -1;
                }
                else if(readchar == ARGO_QUOTE) {
                    argo_append_char(s,ARGO_QUOTE);
                }
                else if(readchar == ARGO_BSLASH) {
                    argo_append_char(s,ARGO_BSLASH);
                }
                else if(readchar == ARGO_FSLASH) {
                    argo_append_char(s,ARGO_FSLASH);
                }
                else if(readchar == ARGO_B) {
                    argo_append_char(s,ARGO_BS);
                }
                else if(readchar == ARGO_F) {
                    argo_append_char(s,ARGO_FF);
                }
                else if(readchar == ARGO_N) {
                    argo_append_char(s,ARGO_LF);
                }
                else if(readchar == ARGO_R) {
                    argo_append_char(s,ARGO_CR);
                }
                else if(readchar == ARGO_T) {
                    argo_append_char(s,ARGO_HT);
                }
                else if(readchar == ARGO_U) {
                    ARGO_CHAR hexNum = 0;
                    for (int i = 0;i<4;i++) {
                        readchar=fgetc(f);
                        if (readchar == EOF) {
                            fprintf(stderr,"End of File before full hexadecimal value\n");
                            return -1;
                        }
                        else if (argo_is_hex(readchar)) {
                            hexNum*=16;
                            if (argo_is_digit(readchar)) {
                                hexNum+=(readchar-'0');
                            }
                            else {//if letter
                                if (readchar<'a') {//if capital letter
                                    hexNum+=10+(readchar-'A');
                                }
                                else {
                                    hexNum+=10+(readchar-'a');
                                }
                            }
                        }
                        else {
                            fprintf(stderr,"Escape characters are not hexadecimal\n");
                            return -1;
                        }
                    }
                    argo_append_char(s,hexNum);
                }
                else {
                    fprintf(stderr,"Invalid escape sequence\n");
                    return -1;
                }
            }
            else if (argo_is_control(readchar)) {
                fprintf(stderr,"Error: Invalid Control character appeared in string\n");
                return -1;
            }
            else {
                argo_append_char(s,readchar);
            }
        }
    }
    else { //if first byte not a "
        fprintf(stderr,"Error: Expected \" to start string\n");
        return -1;
    }
}

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_number(ARGO_NUMBER *n, FILE *f) {
    ARGO_STRING *num_string = &(n->string_value);
    n-> string_value.capacity = 0;
    n-> string_value.length = 0;
    n-> string_value.content = NULL;
    n-> valid_float = 1;
    n-> valid_string = 1;
    n-> valid_int = 1;
    int num_first = fgetc(f);
    int neg = 0; //if negative

    if (num_first == '-') {
        argo_append_char(num_string,num_first);
        neg = 1;
        num_first = fgetc(f);
    }

    long num_total = 0;

    if (argo_is_digit(num_first) && (num_first !='0')) { //if one to nine digit
        argo_append_char(num_string,num_first); //append digit
        num_total = (num_first-'0'); //digit value
        //long long_max = sizeof(long);
        int num_byte;
        while (1) {
            num_byte = fgetc(f);
            if (argo_is_digit(num_byte)) {
                int curDig = num_byte - '0';
                if (num_total < (LONG_MAX - curDig)/10) {
                    num_total *=10;
                    num_total += curDig;
                    argo_append_char(num_string,num_byte);
                }
                else {
                    fprintf(stderr,"Integer exceeded Maximum Long\n");
                    return -1;
                }
            }
            else {
                ungetc(num_byte,f);
                break;
            }
        }
    }
    else if (num_first=='0') { //if first dig was 0
        argo_append_char(num_string,num_first);
    }
    else if (num_first==EOF) {
        fprintf(stderr,"No Number to Read\n");
        return -1;
    }
    else {
        fprintf(stderr,"Invalid start to number\n");
        return -1;
    }

    //have iterated through all
    double num_float_total = (double) num_total;
    //printf("%f\n",num_float_total);

    int num_get = fgetc(f);
    if (num_get == '.') { //fraction
        n->valid_int = 0;
        n->valid_float = 1;

        argo_append_char(num_string,num_get);
        double divisor = 10;
        num_get = fgetc(f);
        //printf("%c",num_get);
        if (argo_is_digit(num_get)) { //check that character after deciaml point is digit
            argo_append_char(num_string,num_get);
            //double dig_float = (double) 
            num_float_total += ((num_get-'0')/divisor);
            divisor *=10;
            while (1) {
                num_get = fgetc(f);
                if (argo_is_digit(num_get)) {
                    argo_append_char(num_string,num_get);
                    int curDig = num_get - '0'; //int curdig

                    num_float_total += (curDig/divisor);
                    //printf("%e\n",num_float_total);
                    divisor*=10;
                }
                else {
                    ungetc(num_get,f);
                    break;
                }
            }
        }
        else {
            fprintf(stderr,"No digit after decimal point\n");
            return -1;
        }
        //printf("mark\n");

        //num_get = fgetc(f);
    }
    else {
        ungetc(num_get,f);
    }


    num_get=fgetc(f);
    if (num_get=='e' || num_get == 'E') {
        argo_append_char(num_string,num_get);
        n->valid_int = 0;
        n->valid_float = 1;
        int exponent = 0;
        int exp_sign = 1;

        num_get = fgetc(f);
        if (num_get == '-') {
            argo_append_char(num_string,num_get);
            exp_sign = -1;
            num_get = fgetc(f);
        }
        else if (num_get == '+') {
            argo_append_char(num_string,num_get);
            exp_sign = 1;
            num_get = fgetc(f);
        }

        if (argo_is_digit(num_get)) {
            argo_append_char(num_string,num_get);
            int curDig = (num_get-'0');
            exponent=curDig;
            while (1) {
                num_get = fgetc(f);
                if (argo_is_digit(num_get)) {
                    argo_append_char(num_string,num_get);
                    exponent*=10;
                    exponent+=(num_get-'0');
                }
                else {
                    ungetc(num_get,f);
                    break;
                }
            }
        }
        else {
            fprintf(stderr,"Exponent value not specified\n");
            return -1;
        }

        long ten_exp_val = 1; //full ten to the power value
        for (int i = 0;i<exponent;i++) {
            ten_exp_val *= 10;
        }

        if (exp_sign == 1) { //if exponent sign positive
            num_float_total *=ten_exp_val;
        }
        else {
            num_float_total /=ten_exp_val;
        }
    }
    else { //if not e or E then unget
        //printf("%c",num_get);
        ungetc(num_get,f);
    }

    //if negative
    if (neg) {
        num_float_total*=-1;
        if (num_total<LONG_MAX-1) {
            num_total *= -1;
        }
        else {
            fprintf(stderr,"Integer below minimum Long\n");
            return -1;
        }
    }


    if (n->valid_float) {
        n->float_value = num_float_total;
        //printf("%f\n",num_float_total);
    }
    if (n->valid_int) {
        n->int_value = num_total;
    }

    // num_get=fgetc(f);
    // printf("%c\n",num_get);
    // ungetc(num_get,f);

    return 0;
}

//return 0 if successful, -1 error
int argo_write_basic(ARGO_BASIC b, FILE *f) {
    if (b==ARGO_TRUE) {
        fputs(ARGO_TRUE_TOKEN,f);
    }
    else if (b==ARGO_FALSE) {
        fputs(ARGO_FALSE_TOKEN, f);
    }
    else if (b==ARGO_NULL) {
        fputs(ARGO_NULL_TOKEN,f);
    }
    else {
        fprintf(stderr,"Invalid ARGO_BASIC\n");
        return -1; //error
    }
    return 0;
}

//returns whether in pretty print mode
int pretty() {
    if (global_options>=CANONICALIZE_OPTION + PRETTY_PRINT_OPTION && global_options < VALIDATE_OPTION) {
        return 1;
    }
    return 0;
}



//returns 0 if no indent (not in pretty print mode)
int indent() {
    if (pretty()) {
        return global_options - CANONICALIZE_OPTION - PRETTY_PRINT_OPTION;
    }
    return 0;
}


void pretty_print_line(FILE *f) {
    fputc(ARGO_LF,f);
    for (int i = 0;i<indent_level*indent();i++) {
        fputc(ARGO_SPACE,f);
    }
}




int write_top_level = 0;

int argo_write_object(ARGO_OBJECT *o, FILE *f) {
    write_top_level++;
    fputc(ARGO_LBRACE,f);
    if (pretty()) { 
        indent_level++;
        pretty_print_line(f);
    }


    ARGO_VALUE* member = o->member_list;

    if (member->next->type != ARGO_NO_TYPE) { //if there is at least one element
        while (1) {
            member=member-> next;
            argo_write_string(&(member->name),f);
            fputc(ARGO_COLON,f);
            if (pretty()) {
                fputc(ARGO_SPACE,f);
            }
            if (argo_write_value(member,f)) {
                return -1; //error
            }
            if (member->next->type!= ARGO_NO_TYPE) { //if next element is not dummy pointer (reached end of object)
                fputc(ARGO_COMMA,f); 
                if (pretty()) { //if pretty, print new line
                    pretty_print_line(f);
                }
            }
            else { //if reached the last element
                if (pretty()) { //if pretty, print new line
                    indent_level--;
                    pretty_print_line(f);
                }

                break;
            }
        }
    }
    fputc(ARGO_RBRACE,f);
    write_top_level--;
    return 0;
}

int argo_write_array(ARGO_ARRAY *a, FILE *f) {
    write_top_level++;
    fputc(ARGO_LBRACK,f); 
    if (pretty()) { //if pretty, print new line
        indent_level++;
        pretty_print_line(f);
    }

    ARGO_VALUE* member = a->element_list;
    if (member->next->type != ARGO_NO_TYPE) { //if there is at least one member of the list
        while (1) {
            member=member-> next;
            if (argo_write_value(member,f)) {       
                return -1; //error
            }
            if (member->next->type!= ARGO_NO_TYPE) {
                fputc(ARGO_COMMA,f); 
                if (pretty()) { //if pretty, print new line
                    pretty_print_line(f);
                }
            }
            else { //if it is equal
                if (pretty()) { //if pretty, print new line
                    indent_level--;
                    pretty_print_line(f);
                }
                break;
            }
        }
    }
    fputc(ARGO_RBRACK,f);
    write_top_level--;
    return 0;

}





/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.   
 */
int argo_write_value(ARGO_VALUE *v, FILE *f) {
    //printf("start write value\n");
    ARGO_VALUE_TYPE curType = v->type;
    if (curType == ARGO_OBJECT_TYPE) {
        if (argo_write_object(&(v->content.object),f)) {
            return -1;
        }   
        //printf("object");
    }
    else if (curType == ARGO_ARRAY_TYPE) {
        if (argo_write_array(&(v->content.array),f)) {
            return -1;
        }
        //printf("\narray\n");
    }
    else if (curType == ARGO_BASIC_TYPE) {
        if (argo_write_basic(v->content.basic,f)) {
            return -1;
        }
        //printf("basic");
    }
    else if (curType == ARGO_STRING_TYPE) {
        if (argo_write_string( &(v->content.string),f)) {
            return -1;
        }
        //printf("string");
    }
    else if (curType == ARGO_NUMBER_TYPE) {
        if (argo_write_number(&(v->content.number),f)) {
            return -1;
        }
    }
    else {
        fprintf(stderr,"Invalid or Missing Type for ARGO_VALUE\n");
        return -1;
    }

    if (pretty() && write_top_level==0) {
        fputc(ARGO_LF,f);
    }

    return 0;
}


void convertHexToChar(int c,FILE *f) {
    if (c<10) {
        fputc(c+'0',f);
    }
    else {
        c-=10;
        fputc(c+'a',f);
    }
}



void convertIntToHex(int c,FILE *f) {
    int q1 = c / 16;
    int r1 = c % 16;
    
    int q2 = q1 / 16;
    int r2 = q1 % 16;
    
    int q3 = q2 / 16;
    int r3 = q2 % 16;
    
    int r4 = q3 % 16;
    
    convertHexToChar(r4,f);
    convertHexToChar(r3,f);
    convertHexToChar(r2,f);
    convertHexToChar(r1,f);
}

/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_string(ARGO_STRING *s, FILE *f) {
    // TO BE IMPLEMENTED.
    fputc(ARGO_QUOTE,f);
    for (int i = 0; i<s->length;i++) {
        int c = *(s->content);
        if (c==ARGO_BS) {   
            fputc(ARGO_BSLASH,f);
            fputc('b',f);
        }
        else if (c==ARGO_FF) {
            fputc(ARGO_BSLASH,f);
            fputc('f',f);
        }
        else if (c==ARGO_LF) {
            fputc(ARGO_BSLASH,f);
            fputc('n',f);
        }
        else if (c==ARGO_CR) {
            fputc(ARGO_BSLASH,f);
            fputc('r',f);
        }
        else if (c==ARGO_HT) {
            fputc(ARGO_BSLASH,f);
            fputc('t',f);
        }
        else if (c==ARGO_BSLASH) {
            fputc(ARGO_BSLASH,f);
            fputc(ARGO_BSLASH,f);
        }
        else if (c==ARGO_QUOTE) {
            fputc(ARGO_BSLASH,f);
            fputc('"',f);
        }
        else if (c>0x001F && c<0x00FF) {
            fputc(*(s->content),f); //print out character as is
        }
        else if (c>0xFFFF) {
            fprintf(stderr,"ARGO_CHAR should not be greater than 0xFFFF\n");
            return -1;
        }
        else {
            fputc(ARGO_BSLASH,f);
            fputc('u',f);
            convertIntToHex(c,f);
        }
        s->content++;
    }
    fputc(ARGO_QUOTE,f);
    return 0;
}

//takes in a long int and prints all of its digits
void printValidInt(long givenInt,FILE *f) {
    if (givenInt==0) {
        fputc('0',f);\
        return;
    }
    int index = -1; //initialize array index
    if (givenInt<0) { //if negative;
        givenInt*=-1;
        fputc('-',f);
    }
    while (givenInt!=0) {
        index++;
        int r = givenInt%10; //remainder
        *(argo_digits+index) = r;
        givenInt /= 10;
    }
    for (int i=index;i>=0;i--) {
        fputc(*(argo_digits+i)+'0',f);
        argo_digits[i]=0;
    }
}

//given a float from argo_write_number, print out the float one character at a time
void printFloat(double inFloat, FILE *f) {
    if (inFloat<0) { //if a negative number, print -
        fputc('-',f); 
        inFloat*=-1; //make positive
    }
    fputc('0',f);
    fputc('.',f);

    if (inFloat==0) { //if 0.0 essentially
        fputc('0',f);
        return;
    }

    int exponent = 0; //exponent of decimal
    if (inFloat>=1) {
        while (inFloat>=1) {
            inFloat/=10; //keep on dividing by 10 until it's in the form 0.xxxxxx
            exponent++;
        }
    }
    else if (inFloat<0.1) { //if something like 0.002
        while (inFloat<0.1) {
            inFloat*=10; //keep on mulitpling by 10 until it gets in the form 0.0000
            exponent--;
        }
    }


    int dig;
    for (int i = 0;i<ARGO_PRECISION;i++) {
        inFloat*=10;
        dig = (int) inFloat; //get the integer part which is a digit
        fputc(dig+'0',f);
        inFloat-=dig; 
        if (inFloat==0) {
            break;
        }
    }
    if (exponent!=0) { 
        fputc('e',f); 
        if (exponent<0){
            fputc('-',f);
            exponent*=-1;
        }
        if (exponent>=10) {
            fputc('1',f);
            exponent-=10;
        }
        fputc(exponent+'0',f);
    }
}


//argo-write string specifically for string_value in argo number
int argo_write_stringNum(ARGO_STRING *s, FILE *f) {
    // TO BE IMPLEMENTED.
    for (int i = 0; i<s->length;i++) {
        int c = *(s->content);    
        if (c==ARGO_BS) {   
            fputc(ARGO_BSLASH,f);
            fputc('b',f);
        }
        else if (c==ARGO_FF) {
            fputc(ARGO_BSLASH,f);
            fputc('f',f);
        }
        else if (c==ARGO_LF) {
            fputc(ARGO_BSLASH,f);
            fputc('n',f);
        }
        else if (c==ARGO_CR) {
            fputc(ARGO_BSLASH,f);
            fputc('r',f);
        }
        else if (c==ARGO_HT) {
            fputc(ARGO_BSLASH,f);
            fputc('t',f);
        }
        else if (c==ARGO_BSLASH) {
            fputc(ARGO_BSLASH,f);
            fputc(ARGO_BSLASH,f);
        }
        else if (c==ARGO_QUOTE) {
            fputc(ARGO_BSLASH,f);
            fputc('"',f);
        }
        else if (c>0x001F && c<=0x00FF) {
            fputc(*(s->content),f); //print out character as is
        }
        else {
            fputc(ARGO_BSLASH,f);
            fputc('u',f);
            convertIntToHex(c,f);
        }
        s->content++;
    }
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_number(ARGO_NUMBER *n, FILE *f) {
    ARGO_NUMBER curNumber = *n;
    if (curNumber.valid_int) {
        if (curNumber.int_value>9999999999 || curNumber.int_value<-9999999999) {
            fprintf(stderr,"Integer exceeds 10 digits\n");
            return -1;
        }
        printValidInt(curNumber.int_value,f);
    }
    else if (curNumber.valid_float) {
        printFloat(curNumber.float_value,f);
        //printf("valid_float\n");
    }
    else if(curNumber.valid_string) {
        argo_write_stringNum(&(curNumber.string_value),f);
        //printf("valid_string\n");
    }
    else {
        fprintf(stderr,"Number is not a valid integer, float, nor string\n");
        return -1; //error
    }
    return 0;
}


