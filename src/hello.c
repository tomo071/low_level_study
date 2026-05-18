#include <stdio.h>
#include <unistd.h>

int main(void)
{
    // 1. 自分のプロセスID（PID）を取得して表示
    printf("私のPID（プロセスID）は %d です。\n", getpid());
    printf("確認コマンド: cat /proc/%d/maps\n\n", getpid());

    printf("一時停止中... Enterキーを押すと終了します。\n");
    getchar(); // 入力待ちにしてプログラムをわざと止める
}
