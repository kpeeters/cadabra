
%{
#include <string>
#include <iostream>
#include <stack>
#include "storage.hh"

// Prototypes to keep the compiler happy
 void yyerror (const char *error);
 int  yylex ();

// Since yacc does not allow us to store iterators,
// we store pointers. Needs to workaround the 
// fact that tree_node is protected in tree.
  exptree                          exp;
%}

%token PROPERTY ASSIGN PROD COMMA UNEQUALS EQUALS PROD SUM LCURLY RCURLY LROUND RROUND LSQUARE RSQUARE SEQUENCE UNDERSCORE CARET SEMICOLON RIGHTARROW CONDITIONAL FACTORIAL NUMBER TEXNAME COMMAND NAME WHITE

%left SUM  MIN
%left PROD DIV
%left POW

%nonassoc LOWER_THAN_PARENT_REL
%nonassoc CARET
%nonassoc UNDERSCORE
%nonassoc LCURLY
%nonassoc LSQUARE
%nonassoc LROUND

%union {
		tree_node_<str_node> *tnode;
		std::string          *str;
}

%type <str>   label NUMBER NAME TEXNAME 
%type <tnode> node product sum comma nodelistspaces argumentlist indexsuper indexsub argument

%%

expr: product SEMICOLON { exp.append_child(exp.begin(), exptree::iterator($1));
                          exp.print_recursive_treeform(std::cout, exp.begin()); }
    | sum SEMICOLON     { exp.append_child(exp.begin(), exptree::iterator($1));
                          exp.print_recursive_treeform(std::cout, exp.begin()); }
    | error             { }
    ;

product: node               { $$=$1; }
       | product PROD node  { if(*($1->data.name)=="\\prod") {
                                 exp.append_child(exptree::iterator($1), exptree::iterator($3));
                                 $$=$1;
                                 }
                              else {
                                 $$=exp.insert_after(exp.begin(), str_node("\\prod")).node;
                                 exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                 exp.append_child(exptree::iterator($$), exptree::iterator($3));
                                 }
                            }
       | product WHITE node  { if(*($1->data.name)=="\\prod") {
                                 exp.append_child(exptree::iterator($1), exptree::iterator($3));
                                 $$=$1;
                                 }
                              else {
                                 $$=exp.insert_after(exp.begin(), str_node("\\prod")).node;
                                 exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                 exp.append_child(exptree::iterator($$), exptree::iterator($3));
                                 }
                            }
;


sum    : product SUM product {  $$=exp.insert_after(exp.begin(), str_node("\\sum")).node;
                                exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));
                             }
       | sum SUM product     {  $$=$1;
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));  
                             }
       | product SUM sum     {  $$=$3;
                                exp.prepend_child(exptree::iterator($$), exptree::iterator($1));  
                             }
       | sum SUM sum         {  $$=$1;
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));  
                             }
; 

comma  : product COMMA product {  $$=exp.insert_after(exp.begin(), str_node("\\comma")).node;
                                exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));
                             }
       | sum COMMA product {  $$=exp.insert_after(exp.begin(), str_node("\\comma")).node;
                                exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));
                             }
       | product COMMA sum {  $$=exp.insert_after(exp.begin(), str_node("\\comma")).node;
                                exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));
                             }
       | sum COMMA sum {  $$=exp.insert_after(exp.begin(), str_node("\\comma")).node;
                                exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));
                             }
       | product COMMA comma   {  $$=$3;
                                exp.prepend_child(exptree::iterator($$), exptree::iterator($1));  
                             }
       | sum COMMA comma   {  $$=$3;
                                exp.prepend_child(exptree::iterator($$), exptree::iterator($1));  
                             }
       | comma COMMA product   {  $$=$1;
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));  
                             }
       | comma COMMA sum   {  $$=$1;
                                exp.append_child(exptree::iterator($$), exptree::iterator($3));  
                             }
; 

nodelistspaces: node                      { $$=exp.insert_after(exp.begin(), str_node("%tmpnodes")).node; 
                                            exp.append_child(exptree::iterator($$), exptree::iterator($1));
                                          }
              | nodelistspaces WHITE node { $$=$1;
                                            exp.append_child(exptree::iterator($$), exptree::iterator($3));
                                          }
              | nodelistspaces node { $$=$1;
                                      exp.append_child(exptree::iterator($$), exptree::iterator($2));
                                    }     
;

indexsuper: CARET LCURLY nodelistspaces RCURLY { 
                         $$=$3;
                         exptree::sibling_iterator sib=exptree::iterator($$).begin();
                         while(sib!=exptree::iterator($$).end()) {
                             sib->fl.parent_rel=str_node::p_super;
                             ++sib;
                             }
                       }
          | CARET node { $$=exp.insert_after(exp.begin(), str_node("%tmpnodes")).node;
                         exp.append_child(exptree::iterator($$), exptree::iterator($2))->fl.parent_rel=str_node::p_super;
                       }
;

indexsub  : UNDERSCORE LCURLY nodelistspaces RCURLY {
                         $$=$3;
                         exptree::sibling_iterator sib=exptree::iterator($$).begin();
                         while(sib!=exptree::iterator($$).end()) {
                             sib->fl.parent_rel=str_node::p_sub;
                             ++sib;
                             }
                         }
          | UNDERSCORE node { $$=exp.insert_after(exp.begin(), str_node("%tmpnodes")).node;
                              exp.append_child(exptree::iterator($$), exptree::iterator($2))->fl.parent_rel=str_node::p_sub;
                            }
;

argument: LCURLY node RCURLY     { $$=$2; $$->data.fl.bracket=str_node::b_none; }
        | LROUND node RROUND     { $$=$2; $$->data.fl.bracket=str_node::b_round; }
        | LSQUARE node RSQUARE   { $$=$2; $$->data.fl.bracket=str_node::b_square; }
        | LCURLY comma RCURLY    { $$=$2; $$->data.fl.bracket=str_node::b_none; }
        | LROUND comma RROUND    { $$=$2; $$->data.fl.bracket=str_node::b_none; }
        | LSQUARE comma RSQUARE  { $$=$2; $$->data.fl.bracket=str_node::b_none; }
;

argumentlist : indexsuper { $$=$1; }
             | indexsub   { $$=$1; }
             | argument   { $$=$1; }
             | argumentlist indexsuper { exp.reparent(exptree::iterator($1), exptree::iterator($2).begin(), 
                                                    exptree::iterator($2).end());
                                         $$=$1;
                                       }
             | argumentlist indexsub   { exp.reparent(exptree::iterator($1), exptree::iterator($2).begin(), 
                                                       exptree::iterator($2).end());
                                         $$=$1;
                                       }
             | argumentlist argument   { exptree::sibling_iterator si1,si2;
                                         si1=$2; si2=$2;
                                         ++si2;
                                         exp.reparent(exptree::iterator($1), si1, si2);
                                         $$=$1;
                                       }
;

label : NUMBER
      | NAME
      | TEXNAME
;

node  : label               %prec LOWER_THAN_PARENT_REL { $$=exp.insert_after(exp.begin(), *$1).node; }
      | label argumentlist     { $$=exp.insert_after(exp.begin(), *$1).node; 
                                 if(*$2->data.name=="%tmpnodes")
                                     exp.append_children(exptree::iterator($$), 
                                       exptree::iterator($2).begin(), exptree::iterator($2).end());
                                 else exp.append_child(exptree::iterator($$), exptree::iterator($2));
                               }
;

%%


std::ostream  *fake_txtout;

int main()
	{
	fake_txtout = &std::cout;

	exp.set_head(str_node("\\expression"));
	return yyparse();
	}

void yyerror(const char *error)
	{
	std::cout << error << std::endl;
	}

