/*
 * diff_writer.c — 差分結果をテキストファイルに書き出す
 *
 * 出力形式（学習用の独自フォーマット）:
 *   - ヘッダ: 比較対象ファイル名と統計
 *   - 本文: [削除] / [追加] / [一致] ごとに行番号と内容
 *   - 一致行は --quiet 相当で省略可能だが、ここでは変更のみ出力
 */
#include "diff.h"

#include <stdio.h>
#include <stdlib.h>

/* 操作種別を日本語ラベルに変換 */
static const char *op_label(DiffOpType type)
{
    switch (type) {
    case DIFF_OP_DELETE:
        return "削除";
    case DIFF_OP_INSERT:
        return "追加";
    case DIFF_OP_EQUAL:
        return "一致";
    default:
        return "?";
    }
}

/*
 * diff_write_report — 差分レポートを output_path に保存
 *
 * フロー:
 *   1. 出力ファイルを書き込みモードで開く
 *   2. ヘッダ（旧/新パス、行数、変更件数）を書く
 *   3. 削除・追加のみ列挙（一致行はサマリに含めず詳細から省略）
 *   4. 変更が 0 件ならその旨を明記
 */
int diff_write_report(
    const char *output_path,
    const char *old_path,
    const char *new_path,
    const DiffResult *result
)
{
    FILE *out = fopen(output_path, "w");

    if (out == NULL) {
        perror(output_path);
        return -1;
    }

    size_t delete_count = 0;
    size_t insert_count = 0;
    size_t equal_count = 0;

    for (size_t i = 0; i < result->count; i++) {
        switch (result->ops[i].type) {
        case DIFF_OP_DELETE:
            delete_count++;
            break;
        case DIFF_OP_INSERT:
            insert_count++;
            break;
        case DIFF_OP_EQUAL:
            equal_count++;
            break;
        }
    }

    fprintf(out, "=== 差分レポート ===\n");
    fprintf(out, "旧ファイル: %s\n", old_path);
    fprintf(out, "新ファイル: %s\n", new_path);
    fprintf(out, "\n");
    fprintf(out, "統計:\n");
    fprintf(out, "  削除: %zu 行\n", delete_count);
    fprintf(out, "  追加: %zu 行\n", insert_count);
    fprintf(out, "  一致: %zu 行\n", equal_count);
    fprintf(out, "\n");
    fprintf(out, "--- 変更箇所 ---\n");

    if (delete_count == 0 && insert_count == 0) {
        fprintf(out, "(変更なし — 2 ファイルは同一内容です)\n");
        fclose(out);
        return 0;
    }

    /*
     * 変更行のみ出力。行番号は旧/新それぞれ 1 始まり。
     * 削除のみ・追加のみの行は片方の行番号が 0 になる。
     */
    for (size_t i = 0; i < result->count; i++) {
        const DiffOp *op = &result->ops[i];

        if (op->type == DIFF_OP_EQUAL) {
            continue;
        }

        fprintf(out, "\n[%s]", op_label(op->type));

        if (op->old_line_no > 0) {
            fprintf(out, " 旧:%zu", op->old_line_no);
        }

        if (op->new_line_no > 0) {
            fprintf(out, " 新:%zu", op->new_line_no);
        }

        fprintf(out, "\n    %s\n", op->text);
    }

    fprintf(out, "\n");
    fclose(out);
    return 0;
}
