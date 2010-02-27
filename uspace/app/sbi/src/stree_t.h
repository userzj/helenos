/*
 * Copyright (c) 2010 Jiri Svoboda
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STREE_T_H_
#define STREE_T_H_

#include "list_t.h"

/*
 * Arithmetic expressions
 */

struct stree_expr;

/** Identifier */
typedef struct {
	int sid;
} stree_ident_t;

/** Name reference */
typedef struct {
	stree_ident_t *name;
} stree_nameref_t;

/** Reference to currently active object. */
typedef struct {
} stree_self_ref_t;

typedef struct {
	int value;
} stree_lit_int_t;

/** Reference literal (there is only one: @c nil). */
typedef struct {
} stree_lit_ref_t;

typedef struct {
	char *value;
} stree_lit_string_t;

typedef enum {
	ltc_int,
	ltc_ref,
	ltc_string
} literal_class_t;

/** Literal */
typedef struct {
	literal_class_t ltc;
	union {
		stree_lit_int_t lit_int;
		stree_lit_ref_t lit_ref;
		stree_lit_string_t lit_string;
	} u;
} stree_literal_t;

/** Binary operation class */
typedef enum {
	bo_period,
	bo_slash,
	bo_sbr,
	bo_equal,
	bo_notequal,
	bo_lt,
	bo_gt,
	bo_lt_equal,
	bo_gt_equal,
	bo_plus
} binop_class_t;

/** Unary operation class */
typedef enum {
	uo_plus
} unop_class_t;

/** Binary operation */
typedef struct {
	/** Binary operation class */
	binop_class_t bc;

	/** Arguments */
	struct stree_expr *arg1, *arg2;
} stree_binop_t;

/** Unary operation */
typedef struct {
	/** Operation class */
	unop_class_t oc;

	/** Argument */
	struct stree_expr *arg;
} stree_unop_t;

/** New operation */
typedef struct {
	/** Type of object to construct. */
	struct stree_texpr *texpr;
} stree_new_t;

/** Member access operation */
typedef struct {
	/** Argument */
	struct stree_expr *arg;
	/** Name of member being accessed. */
	stree_ident_t *member_name;
} stree_access_t;

/** Function call operation */
typedef struct {
	/** Function */
	struct stree_expr *fun;

	/** Arguments */
	list_t args; /* of stree_expr_t */
} stree_call_t;

typedef enum {
	ac_set,
	ac_increase
} assign_class_t;

/** Assignment */
typedef struct {
	assign_class_t ac;
	struct stree_expr *dest, *src;
} stree_assign_t;

/** Arithmetic expression class */
typedef enum {
	ec_nameref,
	ec_literal,
	ec_self_ref,
	ec_binop,
	ec_unop,
	ec_new,
	ec_access,
	ec_call,
	ec_assign
} expr_class_t;

/** Arithmetic expression */
typedef struct stree_expr {
	expr_class_t ec;

	union {
		stree_nameref_t *nameref;
		stree_literal_t *literal;
		stree_self_ref_t *self_ref;
		stree_binop_t *binop;
		stree_unop_t *unop;
		stree_new_t *new_op;
		stree_access_t *access;
		stree_call_t *call;
		stree_assign_t *assign;
	} u;
} stree_expr_t;

/*
 * Type expressions
 */

struct stree_texpr;

/** Type literal class */
typedef enum {
	tlc_int,
	tlc_string
} tliteral_class_t;

/** Type literal */
typedef struct {
	tliteral_class_t tlc;
} stree_tliteral_t;

/** Type name reference */
typedef struct {
	stree_ident_t *name;
} stree_tnameref_t;

/** Type member access operation */
typedef struct {
	/** Argument */
	struct stree_texpr *arg;
	/** Name of member being accessed. */
	stree_ident_t *member_name;
} stree_taccess_t;

/** Type application operation */
typedef struct {
	/** Arguments */
	struct stree_texpr *gtype, *targ;
} stree_tapply_t;

/** Type expression class */
typedef enum {
	tc_tliteral,
	tc_tnameref,
	tc_taccess,
	tc_tapply
} texpr_class_t;

/** Arithmetic expression */
typedef struct stree_texpr {
	texpr_class_t tc;

	union {
		stree_tliteral_t *tliteral;
		stree_tnameref_t *tnameref;
		stree_taccess_t *taccess;
		stree_tapply_t *tapply;
	} u;
} stree_texpr_t;

/*
 * Statements, class members and module members.
 */

/** Statement block */
typedef struct {
	/** List of statements in the block */
	list_t stats; /* of stree_stat_t */
} stree_block_t;

/** Variable declaration */
typedef struct {
	stree_ident_t *name;
	stree_texpr_t *type;
} stree_vdecl_t;

/** If statement */
typedef struct {
	stree_expr_t *cond;
	stree_block_t *if_block;
	stree_block_t *else_block;
} stree_if_t;

/** While statement */
typedef struct {
	stree_expr_t *cond;
	stree_block_t *body;
} stree_while_t;

/** For statement */
typedef struct {
	stree_block_t *body;
} stree_for_t;

/** Raise statement */
typedef struct {
} stree_raise_t;

/** Return statement */
typedef struct {
	stree_expr_t *expr;
} stree_return_t;

/** Expression statement */
typedef struct {
	stree_expr_t *expr;
} stree_exps_t;

/** With-try-except-finally statement (WEF) */
typedef struct {
	stree_block_t *with_block;
	list_t except_blocks; /* of stree_block_t */
	stree_block_t *finally_block;
} stree_wef_t;

/** Statement class */
typedef enum {
	st_vdecl,
	st_if,
	st_while,
	st_for,
	st_raise,
	st_return,
	st_exps,
	st_wef
} stat_class_t;

/** Statement */
typedef struct {
	stat_class_t sc;

	union {
		stree_vdecl_t *vdecl_s;
		stree_if_t *if_s;
		stree_while_t *while_s;
		stree_for_t *for_s;
		stree_raise_t *raise_s;
		stree_return_t *return_s;
		stree_exps_t *exp_s;
		stree_wef_t *wef_s;
	} u;
} stree_stat_t;

/** Formal function parameter */
typedef struct {
	/* Argument name */
	stree_ident_t *name;

	/* Argument type */
	stree_texpr_t *type;
} stree_fun_arg_t;

/** Member function declaration */
typedef struct {
	/** Function name */
	stree_ident_t *name;

	/** Symbol */
	struct stree_symbol *symbol;

	/** Formal parameters */
	list_t args; /* of stree_fun_arg_t */

	/** Return type */
	stree_texpr_t *rtype;

	/** Function body */
	stree_block_t *body;
} stree_fun_t;

/** Member variable declaration */
typedef struct {
	stree_ident_t *name;
	struct stree_symbol *symbol;
	stree_texpr_t *type;
} stree_var_t;

/** Member property declaration */
typedef struct {
	stree_ident_t *name;
	struct stree_symbol *symbol;
	stree_texpr_t *type;
} stree_prop_t;

typedef enum {
	csimbr_csi,
	csimbr_fun,
	csimbr_var,
	csimbr_prop
} csimbr_class_t;

/** Class, struct or interface member */
typedef struct {
	csimbr_class_t cc;

	union {
		struct stree_csi *csi;
		stree_fun_t *fun;
		stree_var_t *var;
		stree_prop_t *prop;
	} u;
} stree_csimbr_t;

typedef enum {
	csi_class,
	csi_struct,
	csi_interface
} csi_class_t;

/** Class, struct or interface declaration */
typedef struct stree_csi {
	/** Which of class, struct or interface */
	csi_class_t cc;

	/** Name of this CSI */
	stree_ident_t *name;

	/** Symbol for this CSI */
	struct stree_symbol *symbol;

	/** Type expression referencing base CSI. */
	stree_texpr_t *base_csi_ref;

	/** Node state for ancr walks. */
	walk_state_t ancr_state;

	/** List of CSI members */
	list_t members; /* of stree_csimbr_t */
} stree_csi_t;

typedef enum {
	/* Class, struct or interface declaration */
	mc_csi
} modm_class_t;

/** Module member */
typedef struct {
	modm_class_t mc;
	union {
		stree_csi_t *csi;
	} u;
} stree_modm_t;

/** Module */
typedef struct stree_module {
	/** List of module members */
	list_t members; /* of stree_modm_t */
} stree_module_t;

typedef enum {
	sc_csi,
	sc_fun,
	sc_var,
	sc_prop
} symbol_class_t;

/** Symbol */
typedef struct stree_symbol {
	symbol_class_t sc;

	union {
		struct stree_csi *csi;
		stree_fun_t *fun;
		stree_var_t *var;
		stree_prop_t *prop;
	} u;

	/** Containing CSI (for all symbols) */
	stree_csi_t *outer_csi;

	/** Containing block (for block-level symbols) */
	stree_block_t *outer_block;
} stree_symbol_t;

/** Program */
typedef struct stree_program {
	/** The one and only module in the program */
	stree_module_t *module;
} stree_program_t;

#endif
