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

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
  }
  return 0;
}
