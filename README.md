# ppl-assignment1

## Tokens
| Token | Name |
|---|---|
|{|COB| 
|}|CCB|
|[|SOB|
|]|SCB|
|()|RB|
|..|TO|
|;|SC|
|:|COL|
|=|ASSGN_OP|
|&&&|BOOL_AND|
|\|\|\||BOOL_OR|
|*, /|AR_OP1|
|+, -|AR_OP2|
|valid var name|VAR_NAME|
|valid static constt|STATIC_CNST|
|program, declare, list, <br>of,variables, array, size,<br> values, jagged, of,<r> integer, boolean, real| KEYWORD - identified<br> by their name|
|R1| is a keyword|

All tokens are terminals.
### Errors to be detected at tokenisation
* If VAR_NAME is valid or not (Can't start with digit and can't be >20 char long)
* If static constant is valid or not (can't be 23.45, etc)
* Need to throw error if no match with any token
***
## Grammar rules

<main_program> **-->** **program** **RB** **COB** <decl_stmts> <assgmt_stmts> **CCB**

<decl_stmts> **-->** <one_decl> <decl_stmts> | <one_decl>

<assgmt_stmts> **-->** <one_assgmt> <assgmt_stmts> | <one_assgmt>

<one_decl> **-->** <single_decl> | <decl_list>

<single_decl> **-->** <single_prim> | <single_rarr> | <single_jarr2d> | <single_jarr3d>

<decl_list> **-->** <list_prim> | <list_rarr> | <list_jarr2d> | <list_jarr3d>

<single_prim> **-->** **declare** **VAR_NAME** **COL** **integer** **SC** | **declare** **VAR_NAME** **COL** **boolean** **SC** | **declare** **VAR_NAME** **COL** **real** **SC**

<list_prim> **-->** **declare** **list**  **of** **variables** **VAR_NAME** **VAR_NAME** <var_list> **COL** **integer** **SC** | **declare** **list**  **of** **variables** **VAR_NAME** **VAR_NAME** <var_list> **COL** **boolean** **SC** | **declare** **list**  **of** **variables** **VAR_NAME** **VAR_NAME** <var_list> **COL** **real** **SC**

<var_list> **-->** **VAR_NAME** <var_list> | *epsilon* ***//var_list has 0 or more var names***

<single_rarr> **-->** **declare** **VAR_NAME** **COL** **array** <dim_rarr> **of** **integer** **SC**

<list_rarr> **-->**  **declare** **list** **of** **variables** **VAR_NAME** **VAR_NAME** <var_list> **COL** **array** <dim_rarr> **of** **integer** **SC**

<dim_rarr> **-->** <one_range> <dim_rarr> | <one_range>

<one_range> **-->** **SOB** <index_> **TO** <index_> **SCB** 

<single_jarr2d> **-->** **declare** **VAR_NAME** **COL** **jagged** **array** <dim_jarr2d> **of** **integer** **SC** <ranges_desc>

<list_jarr2d>  **-->** **declare** **list** **of** **variables** **VAR_NAME** **VAR_NAME** <var_list> **COL** **jagged** **array** <dim_jarr2d> **of** **integer** **SC** <ranges_desc>

<ranges_desc> **-->** <one_range_desc> <ranges_desc> | <one_range_desc>

<one_range_desc> **-->** **R1** **SOB** **STATIC_CNST** **SCB** **COL** **size** **STATIC_CNST** **COL** **values** **COB** <dim_values> <static_cnst_list> **CCB** 

<dim_values> **-->** <static_cnst_list> **SC** <dim_values> | *epsilon*

<dim_jarr2d> **-->** **SOB** **STATIC_CNST** **TO** **STATIC_CNST** **SCB** **SOB** **SCB** 

<single_jarr3d> **-->** **declare** **VAR_NAME** **COL** **jagged** **array** <dim_jarr3d> **of** **integer** **SC** <ranges_desc>

<list_jarr3d>  **-->** **declare** **list** **of** **variables** **VAR_NAME** **VAR_NAME** <var_list> **COL** **jagged** **array** <dim_jarr3d> **of** **integer** **SC** <ranges_desc>

<static_cnst_list> **-->** **STATIC_CNST** <static_cnst_list> | *epsilon*

<dim_jarr3d> **-->** **SOB** **STATIC_CNST** **TO** **STATIC_CNST** **SCB** **SOB** **SCB** **SOB** **SCB** 

<one_assgmt> **-->** <lhs_op> **ASSGN_OP** <bool_expr1> **SC** | <lhs_op> **ASSGN_OP** <arith_expr1> **SC** 

<arith_expr1> **-->** <arith_expr1> **AR_OP1** <arith_expr2> | <arith_expr2>

<arith_expr2> **-->** <arith_expr2> **AR_OP2** <rhs_op> | <rhs_op>

<rhs_op> **-->** <array_elem> | <index_>

<lhs_op> **-->** <array_elem> | **VAR_NAME**

<bool_expr1> **-->** <bool_expr1> **BOOL_AND** <bool_expr2> | <bool_expr2>

<bool_expr2> **-->** <bool_expr2> **BOOL_OR** <rhs_op> | <rhs_op>

<array_elem> **-->** **VAR_NAME** **SOB** <index_> <index_list> **SCB**

<index_> **-->** **VAR_NAME** | **STATIC_CNST**

<index_list> **-->** <index_> <index_list> | *epsilon*
