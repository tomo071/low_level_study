/*
 * diff_main.c — 差分ツールのエントリポイント
 *
 * 使い方:
 *   ./build/diff_tool <旧ファイル> <新ファイル> -o <出力ファイル>
 *
 * 例:
 *   ./build/diff_tool test/old.txt test/new.txt -o build/changes.txt
 *
 * フロー:
 *   1. コマンドライン引数を解析
 *   2. 旧・新ファイルを LineList として読み込む
 *   3. LCS で差分を計算
 *   4. 結果を -o で指定したファイルに書き出す
 */
#include "diff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *prog)
{
    fprintf(stderr, "使い方: %s <旧ファイル> <新ファイル> -o <出力ファイル>\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "例:\n");
    fprintf(stderr, "  %s test/old.txt test/new.txt -o build/changes.txt\n", prog);
}

int main(int argc, char *argv[])
{
    /*
     * 引数: prog old new -o output → 最低 5 個必要
     */
    if (argc != 5 || strcmp(argv[3], "-o") != 0) {
        print_usage(argv[0]);
        return 1;
    }

    const char *old_path = argv[1];
    const char *new_path = argv[2];
    const char *output_path = argv[4];

    printf("=== diff_tool ===\n");
    printf("旧: %s\n", old_path);
    printf("新: %s\n", new_path);
    printf("出力: %s\n\n", output_path);

    /* ステップ 1: 両ファイルを行配列に読み込む */
    printf("[1] ファイル読み込み...\n");

    LineList *old_lines = line_list_read_file(old_path);

    if (old_lines == NULL) {
        fprintf(stderr, "エラー: 旧ファイルを読めません: %s\n", old_path);
        return 1;
    }

    LineList *new_lines = line_list_read_file(new_path);

    if (new_lines == NULL) {
        fprintf(stderr, "エラー: 新ファイルを読めません: %s\n", new_path);
        line_list_free(old_lines);
        return 1;
    }

    printf("    旧: %zu 行\n", old_lines->count);
    printf("    新: %zu 行\n\n", new_lines->count);

    /* ステップ 2: LCS により差分操作列を計算 */
    printf("[2] LCS 差分計算...\n");

    DiffResult *result = diff_compute(old_lines, new_lines);

    if (result == NULL) {
        fprintf(stderr, "エラー: 差分計算に失敗しました\n");
        line_list_free(old_lines);
        line_list_free(new_lines);
        return 1;
    }

    printf("    操作数: %zu 件\n\n", result->count);

    /* ステップ 3: レポートファイルに書き出し */
    printf("[3] レポート出力...\n");

    if (diff_write_report(output_path, old_path, new_path, result) != 0) {
        fprintf(stderr, "エラー: 出力ファイルを書けません: %s\n", output_path);
        diff_result_free(result);
        line_list_free(old_lines);
        line_list_free(new_lines);
        return 1;
    }

    printf("    完了: %s\n", output_path);

    diff_result_free(result);
    line_list_free(old_lines);
    line_list_free(new_lines);

    return 0;
}
