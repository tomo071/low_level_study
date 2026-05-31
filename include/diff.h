/*
 * diff.h — テキストファイル差分ツールの共通型と API
 *
 * 2 つのファイルを行単位で読み込み、LCS（最長共通部分列）で
 * 「削除・追加・一致」を判定し、結果をファイルに書き出す。
 */
#ifndef DIFF_H
#define DIFF_H

#include <stddef.h>

/* ----------------------------------------------------------
 * 行リスト: ファイルの内容を「行の配列」として保持する
 * ---------------------------------------------------------- */
typedef struct {
    char **lines;   /* 各行の文字列（ヒープ上に個別確保） */
    size_t count;   /* 現在の行数 */
    size_t capacity; /* 配列の確保容量（count <= capacity） */
} LineList;

/* ----------------------------------------------------------
 * 差分操作の種類
 * ---------------------------------------------------------- */
typedef enum {
    DIFF_OP_EQUAL,  /* 旧・新どちらにも同じ行が存在 */
    DIFF_OP_DELETE, /* 旧ファイルにのみ存在（削除された行） */
    DIFF_OP_INSERT  /* 新ファイルにのみ存在（追加された行） */
} DiffOpType;

/* 1 件の差分操作（LCS のバックトラック結果） */
typedef struct {
    DiffOpType type;
    size_t old_line_no; /* 旧ファイル側の行番号（1 始まり、該当なしは 0） */
    size_t new_line_no; /* 新ファイル側の行番号（1 始まり、該当なしは 0） */
    char *text;         /* その行の内容（DIFF_OP_EQUAL/DELETE は旧、INSERT は新） */
} DiffOp;

/* 差分操作の列（時系列順: ファイル先頭から末尾へ） */
typedef struct {
    DiffOp *ops;
    size_t count;
    size_t capacity;
} DiffResult;

/* --- ファイル読み込み (diff_reader.c) --- */
LineList *line_list_read_file(const char *path);
void line_list_free(LineList *list);

/* --- LCS 差分計算 (diff_lcs.c) --- */
DiffResult *diff_compute(const LineList *old_lines, const LineList *new_lines);
void diff_result_free(DiffResult *result);

/* --- 結果出力 (diff_writer.c) --- */
int diff_write_report(
    const char *output_path,
    const char *old_path,
    const char *new_path,
    const DiffResult *result
);

#endif /* DIFF_H */
