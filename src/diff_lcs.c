/*
 * diff_lcs.c — LCS（最長共通部分列）による行単位差分の計算
 *
 * アルゴリズム概要:
 *   1. 旧・新の行配列から DP テーブル dp[i][j] を構築
 *      dp[i][j] = 旧の先頭 i 行と新の先頭 j 行の LCS 長
 *   2. dp の右下角からバックトラックし、各行が
 *      「一致 / 削除 / 追加」のどれかを決定
 *   3. バックトラックは末尾→先頭なので、最後に逆順にして返す
 */
#include "diff.h"

#include <stdlib.h>
#include <string.h>

/* DiffResult に操作を 1 件追加（必要なら配列を拡張） */
static int diff_result_push(DiffResult *result, DiffOp op)
{
    if (result->count == result->capacity) {
        size_t new_cap = result->capacity == 0 ? 16 : result->capacity * 2;
        DiffOp *new_ops = realloc(result->ops, new_cap * sizeof(DiffOp));

        if (new_ops == NULL) {
            return -1;
        }

        result->ops = new_ops;
        result->capacity = new_cap;
    }

    result->ops[result->count++] = op;
    return 0;
}

/*
 * dp テーブルを (old_len+1) x (new_len+1) で確保する。
 * 1 次元配列に畳んで確保し、dp[i*(new_len+1)+j] でアクセス。
 */
static size_t *lcs_build_table(const LineList *old_lines, const LineList *new_lines)
{
    size_t old_len = old_lines->count;
    size_t new_len = new_lines->count;
    size_t cols = new_len + 1;
    size_t total = (old_len + 1) * cols;

    size_t *dp = calloc(total, sizeof(size_t));

    if (dp == NULL) {
        return NULL;
    }

    /*
     * 状態遷移:
     *   行が一致 → dp[i-1][j-1] + 1（LCS に 1 行追加）
     *   不一致   → max(dp[i-1][j], dp[i][j-1])（どちらかを捨てる）
     */
    for (size_t i = 1; i <= old_len; i++) {
        for (size_t j = 1; j <= new_len; j++) {
            size_t idx = i * cols + j;

            if (strcmp(old_lines->lines[i - 1], new_lines->lines[j - 1]) == 0) {
                dp[idx] = dp[(i - 1) * cols + (j - 1)] + 1;
            } else {
                size_t from_up = dp[(i - 1) * cols + j];
                size_t from_left = dp[i * cols + (j - 1)];

                dp[idx] = from_up >= from_left ? from_up : from_left;
            }
        }
    }

    return dp;
}

/*
 * dp テーブルを辿り、DiffOp の列を構築する（逆順）。
 * 返り値: 成功 0 / 失敗 -1
 */
static int lcs_backtrack(
    const LineList *old_lines,
    const LineList *new_lines,
    const size_t *dp,
    DiffResult *result
)
{
    size_t i = old_lines->count;
    size_t j = new_lines->count;
    size_t cols = new_lines->count + 1;

    while (i > 0 || j > 0) {
        DiffOp op;

        if (i > 0 && j > 0 &&
            strcmp(old_lines->lines[i - 1], new_lines->lines[j - 1]) == 0) {
            /*
             * 両方の末尾行が一致 → LCS の一部として「一致」と記録
             */
            op.type = DIFF_OP_EQUAL;
            op.old_line_no = i;
            op.new_line_no = j;
            op.text = old_lines->lines[i - 1];
            i--;
            j--;
        } else if (j > 0 &&
                   (i == 0 || dp[(i - 1) * cols + j] <= dp[i * cols + (j - 1)])) {
            /*
             * 新ファイル側の行を「追加」として採用
             * （左方向 dp[i][j-1] の方が LCS が長い、または旧が尽きた）
             */
            op.type = DIFF_OP_INSERT;
            op.old_line_no = 0;
            op.new_line_no = j;
            op.text = new_lines->lines[j - 1];
            j--;
        } else {
            /*
             * 旧ファイル側の行を「削除」として採用
             */
            op.type = DIFF_OP_DELETE;
            op.old_line_no = i;
            op.new_line_no = 0;
            op.text = old_lines->lines[i - 1];
            i--;
        }

        if (diff_result_push(result, op) != 0) {
            return -1;
        }
    }

    return 0;
}

/* 配列を先頭↔末尾で入れ替え（バックトラック結果を正順にする） */
static void diff_result_reverse(DiffResult *result)
{
    for (size_t left = 0, right = result->count; left < right; ) {
        right--;
        DiffOp tmp = result->ops[left];

        result->ops[left] = result->ops[right];
        result->ops[right] = tmp;
        left++;
    }
}

/*
 * diff_compute — 2 つの LineList から差分操作列を生成
 *
 * フロー:
 *   1. LCS の DP テーブルを構築
 *   2. 右下角からバックトラックして DiffOp を収集
 *   3. 逆順を直して DiffResult を返す
 */
DiffResult *diff_compute(const LineList *old_lines, const LineList *new_lines)
{
    if (old_lines == NULL || new_lines == NULL) {
        return NULL;
    }

    size_t *dp = lcs_build_table(old_lines, new_lines);

    if (dp == NULL) {
        return NULL;
    }

    DiffResult *result = calloc(1, sizeof(DiffResult));

    if (result == NULL) {
        free(dp);
        return NULL;
    }

    if (lcs_backtrack(old_lines, new_lines, dp, result) != 0) {
        free(dp);
        diff_result_free(result);
        return NULL;
    }

    free(dp);
    diff_result_reverse(result);
    return result;
}

void diff_result_free(DiffResult *result)
{
    if (result == NULL) {
        return;
    }

    free(result->ops);
    free(result);
}
