#include "mycc.h"

int labelseq = 1;
char *args[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
  printf("# gen_lval()\n");
  if (node->kind != ND_LVAR) error("left value is not variable");
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
  printf("# end gen_lval()\n");
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("# ND_NUM\n");
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      printf("# ND_LVAR\n");
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      printf("# ND_ASSIGN\n");
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_IF: {
      printf("# ND_IF\n");
      int seq = labelseq++;
      if (node->els) {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", seq);
        gen(node->then);
        printf("  jmp .Lend%d\n", seq);
        printf(".Lelse%d:\n", seq);
        gen(node->els);
        printf(".Lend%d:\n", seq);
      } else {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%d\n", seq);
        gen(node->then);
        printf(".Lend%d:\n", seq);
      }
      return;
    }
    case ND_WHILE: {
      printf("# ND_WHILE\n");
      int seq = labelseq++;
      printf(".Lbegin%d:\n", seq);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", seq);
      gen(node->then);
      printf("  jmp .Lbegin%d\n", seq);
      printf(".Lend%d:\n", seq);
      return;
    }
    case ND_FOR: {
      printf("# ND_FOR\n");
      int seq = labelseq++;
      gen(node->init);
      printf(".Lbegin%d:\n", seq);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", seq);
      gen(node->then);
      gen(node->inc);
      printf("  jmp .Lbegin%d\n", seq);
      printf(".Lend%d:\n", seq);
      return;
    }
    case ND_BLOCK:
      printf("# ND_BLOCK\n");
      for (Node *n = node->body; n; n = n->next) gen(n);
      return;
    case ND_FUNCALL: {
      printf("# ND_FUNCALL\n");
      int argcnt = 0;
      // 引数をすべて評価して RSP に積む
      for (Node *n = node->args; n; n = n->next) {
        gen(n);
        argcnt++;
      }

      // 引数の内容が RSP に積まれているので取り出してレジスタへ
      // See: https://github.com/hjl-tools/x86-psABI/wiki/X86-psABI
      for (int i = argcnt - 1; i >= 0; i--) printf("  pop %s\n", args[i]);

      // 処理を実行して結果をスタックに積む
      printf("  call %s\n", node->funcname);
      printf("  push rax\n");
      return;
    }
    case ND_FUNCDEF: {
      printf("# ND_FUNCDEF\n");
      printf(".global %s\n", node->funcname);
      printf("%s:\n", node->funcname);

      // プロローグ
      // TODO: 確保するメモリをちゃんと計測する
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");
      printf("  sub rsp, 208\n");

      // 引数ごとに rbp - offset 座標に置いておく
      int argidx = 0;
      for (Node *n = node->args; n; n = n->next) {
        // 引数を args 順のレジスタから受け取りスタックに積む
        printf("  mov [rbp - %d], %s\n", n->offset, args[argidx]);
        argidx++;
      }

      // bodyの処理を実行
      for (Node *n = node->body; n; n = n->next) gen(n);

      // エピローグ
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    }
    case ND_RETURN:
      printf("# ND_RETURN\n");
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_EQ:
      printf("# ND_EQ\n");
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("# ND_NE\n");
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("# ND_LT\n");
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("# ND_LE\n");
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_ADD:
      printf("# ND_ADD\n");
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("# ND_SUB\n");
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("# ND_MUL\n");
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("# ND_DIV\n");
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
  }

  printf("  push rax\n");
}
