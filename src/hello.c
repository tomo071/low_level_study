#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int pid = getpid();

    printf("私のPID（プロセスID）は %d です。\n", pid);
    printf("\n");
    printf("[手順1] 同じコンテナの別ターミナルで maps を確認:\n");
    printf("  ホスト側: docker compose exec dev bash\n");
    printf("  コンテナ内: cat /proc/%d/maps\n", pid);
    printf("  ※ WSLホストの /proc/%d ではない（PID名前空間が別）\n", pid);
    printf("\n");

    printf("一時停止中... Enterキーを押すと終了します。\n");
    getchar(); // 入力待ちにしてプログラムをわざと止める
}
