#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを示すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind;  // トークンの型
  Token *next;     // 次の入力トークン
  int val;         // kind が TK_NUM の場合、その数値
  char *str;       // トークン文字列
  int len;         // トークンの長さ
};

// 現在のトークン
extern Token *token;

typedef struct LVar LVar;

// ローカル変数を表す型
// TODO: Var と VarList に分ける
struct LVar {
  LVar *next;  // 次の変数かNULL
  char *name;  // 名前の変数
  int len;     // 名前の長さ
  int offset;  // rbp からのオフセット
};

// ローカル変数
// TODO: グローバル変数も作る
extern LVar *locals;

// ローカル変数を探索する
LVar *find_lvar(Token *tok);

// 入力プログラム
extern char *user_input;

// 抽象構文木のノードの種類
typedef enum {
  ND_EQ,       // ==
  ND_NE,       // !=
  ND_LT,       // <, >
  ND_LE,       // <=, >=
  ND_ADD,      // +
  ND_SUB,      // -
  ND_MUL,      // *
  ND_DIV,      // /
  ND_ASSIGN,   // =
  ND_LVAR,     // ローカル変数
  ND_NUM,      // 整数
  ND_RETURN,   // return文
  ND_IF,       // if文
  ND_WHILE,    // while文
  ND_FOR,      // for文
  ND_BLOCK,    // ブロック
  ND_FUNCALL,  // 関数呼び出し
  ND_FUNCDEF,  // 関数定義
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind;  // ノードの型
  Node *next;     // 単一ノードへ連結するときに使う
  Node *lhs;      // 左辺
  Node *rhs;      // 右辺

  // controll statement
  Node *cond;  // 条件ノード
  Node *then;  // 実行ノード
  Node *els;   // if文のelse節の実行ノード
  Node *init;  // for文のループ文字宣言用のノード
  Node *inc;   // for文のループごとの加算用のノード

  // block statement
  Node *body;  // ブロックノード

  // func call
  char *funcname;  // 関数名
  Node *args;      // 引数

  int val;     // kind が ND_NUM のときのみ使う
  int offset;  // kind が ND_LVAR のときのみ使う
};

extern Node *code[];

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

// トークンの一致判定関数の宣言
bool consume(char *op);
Token *consume_ident();

// トークンの一致判定関数の宣言
// 不一致の場合には exit する
void expect(char *op);

// トークンが数値かどうかの判定する関数の宣言
// 一致すれば数値を返し、そうでなければ exit する
int expect_number();

bool at_eof();

Token *tokenize(char *p);

// 抽象構文木にパースする関数の宣言
Node *program();

// アセンブリ生成関数の宣言
void gen(Node *node);
