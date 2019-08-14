#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED,  // 記号
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
Token *token;

// 入力プログラム
char *user_input;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 現在のトークンが期待している記号のときには、トークンを一つ読み進めて
// 真を返す。それ以外の場合は偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      // token->len の字数を比較
      // 一致していれば 0(false) を返す
      // 一致していなければ正または負の整数(true) を返す
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 現在のトークンが期待している記号のときには、トークンを一つ読みすすめる。
// それ以外の場合にはエラーを報告する
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "not a '%c'", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを一つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM) error_at(token->str, "not a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成して cur につなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// 2文字以上の予約語とのマッチャー
char *reserved_chars(char *p) {
  static char *ops[] = {"==", "!=", "<=", ">="};
  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++) {
    if (memcmp(p, ops[i], strlen(ops[i])) == 0) return ops[i];
  }
  return NULL;
}

// 入力文字列 p をトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    // 二文字の演算子
    char *op = reserved_chars(p);
    if (op != NULL) {
      int len = strlen(op);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }

    // 一文字の演算子
    if (*p == '<' || *p == '>' || *p == '+' || *p == '-' || *p == '*' ||
        *p == '/' || *p == '(' || *p == ')') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 1);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "can not tokenize");
  }

  new_token(TK_EOF, cur, p, 1);
  return head.next;
}

// 抽象構文木のノードの種類
typedef enum {
  ND_EQ,   // =
  ND_NE,   // !=
  ND_LT,   // <, >
  ND_LE,   // <=, >=
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_NUM,  // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind;  // ノードの型
  Node *lhs;      // 左辺
  Node *rhs;      // 右辺
  int val;        // kind が ND_NUM のときのみ使う
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// num ノードは終端なので子ノードを持つ必要はない
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *equality();
Node *expr() { Node *node = equality(); }

Node *relational();
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *add();
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *mul();
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *unary();
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *term();
Node *unary() {
  if (consume("+")) return term();
  if (consume("-")) return new_node(ND_SUB, new_node_num(0), term());
  return term();
}

Node *term() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("	push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("	pop rdi\n");
  printf("	pop rax\n");

  switch (node->kind) {
    case ND_EQ:
      printf("	cmp rax, rdi\n");
      printf("	sete al\n");
      printf("	movzb rax, al\n");
      break;
    case ND_NE:
      printf("	cmp rax, rdi\n");
      printf("	setne al\n");
      printf("	movzb rax, al\n");
      break;
    case ND_LT:
      printf("	cmp rax, rdi\n");
      printf("	setl al\n");
      printf("	movzb rax, al\n");
      break;
    case ND_LE:
      printf("	cmp rax, rdi\n");
      printf("	setle al\n");
      printf("	movzb rax, al\n");
      break;
    case ND_ADD:
      printf("	add rax, rdi\n");
      break;
    case ND_SUB:
      printf("	sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("	imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("	cqo\n");
      printf("	idiv rdi\n");
      break;
  }

  printf("	push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid argument: incorrect arguments count\n");
    return 1;
  }

  // トークナイズして抽象構文木にパース
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を降りながらコード生成
  gen(node);

  // スタックトップに式全体が残っているはずなので
  // それを rax にロードし関数の返り値とする
  printf("	pop rax\n");
  printf("	ret\n");
  return 0;
}
