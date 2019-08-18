#include "mycc.h"

Token *token;
char *user_input;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

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
  if (token->kind != TK_RESERVED && token->kind != TK_RETURN ||
      strlen(op) != token->len ||
      // token->len の字数を比較
      // 一致していれば 0(false) を返す
      // 一致していなければ正または負の整数(true) を返す
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT) return NULL;
  Token *t = token;
  token = token->next;
  return t;
}

// 現在のトークンが期待している記号のときには、トークンを一つ読みすすめる。
// それ以外の場合にはエラーを報告する
void expect(char *op) {
  if (token->kind != TK_RESERVED && token->kind != TK_RETURN ||
      strlen(op) != token->len || memcmp(token->str, op, token->len))
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

// 変数を名前で検索する。見つからなかったらNULLを返す
LVar *find_lvar(Token *tok) {
  // 連結リスト LVar をすべて探索
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

// 2文字以上の予約語とのマッチャー
char *reserved_chars(char *p) {
  static char *ops[] = {"==", "!=", "<=", ">="};
  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++) {
    if (memcmp(p, ops[i], strlen(ops[i])) == 0) return ops[i];
  }
  return NULL;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
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

    // return文
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
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
        *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '=') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      char *q = p++;
      while ('a' <= *p && *p <= 'z') p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
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
