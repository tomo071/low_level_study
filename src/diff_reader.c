/*
 * diff_reader.c — テキストファイルを行単位で読み込む
 *
 * getline() で 1 行ずつ読み、動的配列 LineList に格納する。
 * 改行文字 (\n) は除去して比較しやすくする。
 */
#define _POSIX_C_SOURCE 200809L

#include "diff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 配列が足りなければ 2 倍に拡張する */
static int line_list_grow(LineList *list)
{
    size_t new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
    char **new_lines = realloc(list->lines, new_cap * sizeof(char *));

    if (new_lines == NULL) {
        return -1;
    }

    list->lines = new_lines;
    list->capacity = new_cap;
    return 0;
}

/*
 * 末尾の改行 (\n) とキャリッジリターン (\r) を取り除く。
 * Windows 形式 (\r\n) と Unix 形式 (\n) の両方に対応。
 */
static void strip_trailing_newline(char *line)
{
    size_t len = strlen(line);

    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

/*
 * line_list_read_file — ファイル全体を行の配列として読み込む
 *
 * フロー:
 *   1. fopen でテキストモードで開く
 *   2. getline で 1 行ずつ読む（バッファは getline が自動拡張）
 *   3. 改行を除去して strdup でコピーし、LineList に追加
 *   4. EOF まで繰り返し、LineList を返す
 */
LineList *line_list_read_file(const char *path)
{
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        perror(path);
        return NULL;
    }

    LineList *list = calloc(1, sizeof(LineList));

    if (list == NULL) {
        fclose(fp);
        return NULL;
    }

    char *buf = NULL;
    size_t buf_cap = 0;
    ssize_t nread;

    /*
     * getline の第 2 引数 &buf_cap は「バッファサイズへのポインタ」。
     * 行が長い場合 getline が realloc してくれる。
     */
    while ((nread = getline(&buf, &buf_cap, fp)) != -1) {
        (void)nread;

        if (list->count == list->capacity && line_list_grow(list) != 0) {
            free(buf);
            line_list_free(list);
            fclose(fp);
            return NULL;
        }

        strip_trailing_newline(buf);

        list->lines[list->count] = strdup(buf);

        if (list->lines[list->count] == NULL) {
            free(buf);
            line_list_free(list);
            fclose(fp);
            return NULL;
        }

        list->count++;
    }

    free(buf);
    fclose(fp);
    return list;
}

/* LineList が保持する各行と配列本体を解放する */
void line_list_free(LineList *list)
{
    if (list == NULL) {
        return;
    }

    for (size_t i = 0; i < list->count; i++) {
        free(list->lines[i]);
    }

    free(list->lines);
    free(list);
}
