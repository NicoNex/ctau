#ifndef ITEM_H_
#define ITEM_H_

#include <stddef.h>

enum item_type {
	item_eof,
	item_error,
	item_null,

	item_ident,
	item_int,
	item_float,
	item_string,
	item_rawstring,

	item_assign,
	item_plus,
	item_minus,
	item_slash,
	item_asterisk,
	item_modulus,
	item_plus_assign,
	item_minus_assign,
	item_asterisk_assign,
	item_slash_assign,
	item_modulus_assign,
	item_bw_and_assign,
	item_bw_or_assign,
	item_bw_xor_assign,
	item_lshift_assign,
	item_rshift_assign,
	item_equals,
	item_not_equals,
	item_bang,
	item_lt,
	item_gt,
	item_lteq,
	item_gteq,
	item_and,
	item_or,
	item_bw_and,
	item_bw_not,
	item_bw_or,
	item_bw_xor,
	item_lshift,
	item_rshift,
	item_plusplus,
	item_minusminus,

	item_dot,
	item_comma,
	item_colon,
	item_semicolon,
	item_new_line,

	item_lparen,
	item_rparen,

	item_lbrace,
	item_rbrace,

	item_lbracket,
	item_rbracket,

	item_function,
	item_for,
	item_continue,
	item_break,
	item_if,
	item_else,
	item_true,
	item_false,
	item_return,
	item_import,
	item_tau
};

struct string {
	char *val;
	size_t len;
};

struct item {
	enum item_type type;
	struct string lit;
	int pos;
};

struct item new_item(struct string lit, enum item_type type, int pos);
enum item_type lookup_type(struct string s);
struct string slice_str(char *s, size_t len);
void print_item(struct item i);

#endif
