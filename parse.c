#include "mycc.h"

Node *code[100];
LVar *locals;

// 利用している環境にたまたま strndup の実装がなかったので追加
char *strndup(const char *s, size_t n) {
  char *p = memchr(s, '\0', n);
  if (p != NULL) n = p - s;
  p = malloc(n + 1);
  if (p != NULL) {
    memcpy(p, s, n);
    p[n] = '\0';
  }
  return p;
}

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

Node *function();
Node *program() {
  int i = 0;
  while (!at_eof()) code[i++] = function();
  code[i] = NULL;
}

Node *param();
Node *stmt();
Node *function() {
  // 関数の形式かチェック
  Token *tok = consume_ident();
  if (!tok || !consume("(")) {
    error("not a function");
    return NULL;
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNCDEF;
  node->funcname = strndup(tok->str, tok->len);

  // 宣言部分のパース
  if (!consume(")")) {
    Node *head = param();
    Node *cur = head;
    while (consume(",")) {
      cur->next = param();
      cur = cur->next;
    }
    expect(")");
    node->args = head;
  }

  // ブロック部分のパース
  if (consume("{")) {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!consume("}")) {
      cur->next = stmt();
      if (cur->offset)
        cur->next->offset = 8;
      else
        cur->next->offset = cur->offset + 8;
      cur = cur->next;
    }

    node->body = head.next;
    return node;
  }
}

Node *param() {
  Token *tok = consume_ident();
  if (!tok) {
    error("not a parameter");
    return NULL;
  }
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  // 関数スコープなので find_lvar はしない
  // TODO: スコープ概念を追加
  // 引数をローカル変数に追加
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  if (locals)
    lvar->offset = locals->offset + 8;
  else
    lvar->offset = 8;
  node->offset = lvar->offset;
  locals = lvar;
  return node;
}

Node *expr();
Node *stmt() {
  Node *node;

  if (consume("{")) {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }

    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->body = head.next;
    return node;
  } else if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
    return node;
  } else if (consume("if")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else")) node->els = stmt();
    return node;
  } else if (consume("while")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  } else if (consume("for")) {
    // "for" "(" expr? ";" expr? ";" expr? ")" stmt
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    }
    node->then = stmt();
    return node;
  } else {
    node = expr();
    if (!consume(";"))
      error_at(token->str, "want ';' token. got %s", *token->str);
    return node;
  }
}

Node *assign();
Node *expr() {
  Node *node = assign();
  return node;
}

Node *equality();
Node *assign() {
  Node *node = equality();
  if (consume("=")) node = new_node(ND_ASSIGN, node, assign());
  return node;
}

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

  Token *tok = consume_ident();
  if (tok) {
    // 関数呼び出しを保存
    if (consume("(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_FUNCALL;
      node->funcname = strndup(tok->str, tok->len);

      // 引数を保存する node
      if (!consume(")")) {
        Node *head = assign();
        Node *cur = head;
        while (consume(",")) {
          cur->next = assign();
          cur = cur->next;
        }
        expect(")");
        node->args = head;
      }
      return node;
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals)
        lvar->offset = locals->offset + 8;
      else
        lvar->offset = 8;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }

  return new_node_num(expect_number());
}
