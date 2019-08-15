#include "mycc.h"

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
