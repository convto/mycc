#include "mycc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid argument: incorrect arguments count\n");
    return 1;
  }

  // トークナイズして抽象構文木にパース
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ
  // 8byte変数26個分の領域を確保
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに値が残っているはず
    // スタックが溢れないように pop しておく
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果が rax に残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
