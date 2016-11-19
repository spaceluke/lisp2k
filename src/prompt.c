#include<stdio.h>
#include<stdlib.h>
#include<editline/readline.h>
#include<editline/history.h>

#include "mpc.h"
/*Error handling and thought leading 
  we handle errors up to schrodinbug level
  your bug exist and your bug doesnt exist,program runs
*/
typedef struct {
    int type;
    long num;
    int err;
}lval;
/*enumerate possible eval types*/
enum {LVAL_NUM,LVAL_ERR};
/*enumerate possible lval error types*/
enum {LERR_DIV_ZERO, LERR_BAD_NUM,LERR_BAD_OP};

lval lval_num(long x)
{
    lval v;
    v.type=LVAL_NUM;
    v.num=x;
    return v;
}

lval lval_err(int x)
{
    lval v;
    v.type=LVAL_ERR;
    v.num=x;
    return v;
}

/* print an lval */
void lval_print(lval v)
{
    switch(v.type)
    {
        /*case it's a number print it leave..*/
        case LVAL_NUM:printf("%li",v.num); break;
        /*case if it's an error type we print resp log*/
        case LVAL_ERR:
            /*check the error type*/
            if(v.err = LERR_DIV_ZERO){
                puts("Error: Division by Zero");
            }
           if(v.err = LERR_BAD_OP){
                puts("Error : Bad Operator"); 
           }
           if(v.err = LERR_BAD_NUM){
                puts("Error: Bad Number ");
            }    
        break;
    }
}
/*evaluate the operator against arithmetic operation*/
lval eval_op(lval x, char *op, lval y){
    
    if(x.type == LVAL_ERR) {return x;}
    if(y.type == LVAL_ERR) {return y;}
    
    if(strcmp(op, "+")==0){return lval_num(x.num+y.num);}
    if(strcmp(op, "-")==0){return lval_num(x.num-y.num);}
    if(strcmp(op, "*")==0){return lval_num(x.num*y.num);}
    if(strcmp(op, "/")==0){
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO) : lval_err(x.num/y.num) ;
        }

    if(strcmp(op, "%")==0){
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO) : lval_err(x.num % y.num);
        }
    return lval_err(LERR_BAD_OP);
}

/*evaluation of the regular expression*/

lval eval(mpc_ast_t* t)
{
    /* if tagged as numeric return to it directly*/
    if(strstr(t->tag, "number")){
        errno = 0;
        long x = strtol(t->contents,NULL,10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    /*the operator is always a second child in the tree*/
    char* op = t->children[1]->contents;

    /*we store the third child in x*/
    lval x = eval(t->children[2]);

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
            lval result = eval(r.output);
            lval_print(result);
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

