#include<stdio.h>
#include<stdlib.h>
#include<editline/readline.h>
#include<editline/history.h>

#include "mpc.h"

/*evaluate the operator against arithmetic operation*/
long eval_op(long x, char *op, long y){
    if(strcmp(op, "+")==0){return x+y;}
    if(strcmp(op, "-")==0){return x-y;}
    if(strcmp(op, "*")==0){return x*y;}
    if(strcmp(op, "/")==0){return x/y;}
    if(strcmp(op, "%")==0){return x%y;}

    return 0;
}

/*evaluation of the regular expression*/

long eval(mpc_ast_t* t)
{
    /* if tagged as numeric return to it directly*/
    if(strstr(t->tag, "number")){
        return atoi(t->contents);
    }

    /*the operator is always a second child in the tree*/
    char* op = t->children[1]->contents;

    /*we store the third child in x*/
    long x = eval(t->children[2]);

    /* iterate trough the remaining children*/
    int i = 3;
    while(strstr(t->children[i]->tag, "expr")){
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}


int main(int argc,char** argv)
{

    /*create a parser using mpc library*/
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lisp2k = mpc_new("lisp2k");

    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                   \
            number : /-?[0-9]+/ ;                           \      
            operator : '+' | '-' | '*' | '/' | '%' ;        \
            expr : <number> | '(' <operator> <expr>+ ')' ;  \
            lisp2k : /^/ <operator> <expr>+ /$/ ;           \
        ",
         Number, Operator, Expr, Lisp2k);
    /*Printing Version and User Information*/
    puts("Lisp2k version 0.0.1");
    puts("@author:quantumbiscuit");
    puts("Press CTRL+C to exit.");
    while(1)
    {
        /*pointer to input*/
        char *input = readline("lisp2k> ");
        /*add input to history*/
        add_history(input);

        /*parsing input*/
        mpc_result_t r;
        if(mpc_parse("<stdin>",input, Lisp2k, &r)){
            //if parsing is successful we print the output
            //mpc_ast_print(r.output);
            //mpc_ast_delete(r.output);
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);

        }else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        /*pls no mem leaks pls*/
        free(input);
    }
    mpc_cleanup(4,Number,Operator,Expr,Lisp2k);
}

