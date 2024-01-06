#include "xLinear.h"

#include <stdlib.h>

xMatrix *xMatrix_new(int rows, int cols) {
    xMatrix *mat = malloc(sizeof(xMatrix));

    // if we failed to allocate matrix
    if (mat == NULL) {
        return NULL;
    }

    mat->rows = rows;
    mat->cols = cols;
    mat->data = (float **)malloc(rows * sizeof(float *));

    // if we failed to allocate rows
    if (mat->data == NULL) {
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        mat->data[i] = (float *)calloc(cols, sizeof(float));  // automatically assigned to 0

        // if we failed to allocate columns in some row
        if (mat->data[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(mat->data[i]);
            }
            free(mat->data);
            free(mat);
            return NULL;
        }
    }

    return mat;
}

void xMatrix_free(xMatrix *mat) {
    for (int i = 0; i < mat->rows; i++) {
        free(mat->data[i]);
    }
    free(mat->data);
    free(mat);
    mat = NULL;
}

xMatrix *xMatrix_slice(xMatrix *mat, int row_start, int row_end, int col_start, int col_end) {
    // validate index bounds
    if (row_start < 0 || row_end >= mat->rows || row_start >= row_end || col_start < 0 || col_end >= mat->cols || col_start >= col_end) {
        return NULL;
    }

    xMatrix *retMatrix = xMatrix_new(row_end - row_start, col_end - col_start);
    // check if matrix is successfully allocated
    if (retMatrix == NULL) {
        return NULL;
    }

    for (int i = row_start; i < row_end; i++) {
        for (int j = col_start; j < col_end; j++) {
            retMatrix->data[i - row_start][j - col_start] = mat->data[i][j];
        }
    }

    return retMatrix;
}

xMatrix *xMatrix_identity(int dim) {
    xMatrix *mat = xMatrix_new(dim, dim);
    // check if matrix is successfully allocated
    if (mat == NULL) {
        return NULL;
    }

    for (int i = 0; i < dim; i++) {
        mat->data[i][i] = 1;
    }

    return mat;
}

xMatrix *xMatrix_row(xMatrix *mat, int rowIndex) {
    // validate index bounds
    if (rowIndex >= mat->rows) {
        return NULL;
    }

    xMatrix *row = xMatrix_new(1, mat->cols);
    // check if matrix is successfully allocated
    if (row == NULL) {
        return NULL;
    }

    for (int i = 0; i < mat->cols; i++) {
        row->data[0][i] = mat->data[rowIndex][i];
    }

    return row;
}

xMatrix *xMatrix_col(xMatrix *mat, int colIndex) {
    // validate index bounds
    if (colIndex >= mat->cols) {
        return NULL;
    }

    xMatrix *col = xMatrix_new(mat->rows, 1);
    // check if matrix is successfully allocated
    if (col == NULL) {
        return NULL;
    }

    for (int i = 0; i < mat->rows; i++) {
        col->data[i][0] = mat->data[i][colIndex];
    }

    return col;
}

void xMatrix_add(xMatrix *res, xMatrix *mat1, xMatrix *mat2) {
    // check matrix dimensions
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols || res->rows != mat1->rows || res->cols != mat1->cols) {
        return;
    }

    // check if matrix data is not NULL
    if (res->data == NULL || mat1->data == NULL || mat2->data == NULL) {
        return;
    }

    // calculate matrix sum
    for (int i = 0; i < res->rows; i++) {
        for (int j = 0; j < res->cols; j++) {
            res->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
        }
    }
}

void xMatrix_sub(xMatrix *res, xMatrix *mat1, xMatrix *mat2) {
    // check matrix dimensions
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols || res->rows != mat1->rows || res->cols != mat1->cols) {
        return;
    }

    // check if matrix data is not NULL
    if (res->data == NULL || mat1->data == NULL || mat2->data == NULL) {
        return;
    }

    // calculate matrix difference
    for (int i = 0; i < res->rows; i++) {
        for (int j = 0; j < res->cols; j++) {
            res->data[i][j] = mat1->data[i][j] - mat2->data[i][j];
        }
    }
}

void xMatrix_dot(xMatrix *res, xMatrix *mat1, xMatrix *mat2) {
    // check if res matrix is not any of the input matrices
    if (res == mat1 || res == mat2) {
        return;
    }

    // check if matrix dimensions are valid
    if (mat1->cols != mat2->rows || res->rows != mat1->rows || res->cols != mat2->cols) {
        return;
    }

    // check if matrix data is not NULL
    if (res->data == NULL || mat1->data == NULL || mat2->data == NULL) {
        return;
    }

    // calculate matrix product
    for (int i = 0; i < mat1->rows; i++) {
        for (int j = 0; j < mat2->cols; j++) {
            for (int k = 0; k < mat1->cols; k++) {
                res->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
            }
        }
    }
}

void xMatrix_transpose(xMatrix *mat) {
    // check if matrix is not NULL
    if (mat == NULL) {
        return;
    }

    xMatrix *transposed = xMatrix_new(mat->cols, mat->rows);
    // check if matrix is successfully allocated
    if (transposed == NULL) {
        return;
    }

    // assign values to new matrix
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            transposed->data[j][i] = mat->data[i][j];
        }
    }

    // free old matrix and swap with new one
    xMatrix_free(mat);
    mat = transposed;
}

void xMatrix_scale(xMatrix *mat, float scale) {
    // check if matrix is not NULL
    if (mat == NULL) {
        return;
    }

    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            mat->data[i][j] *= scale;
        }
    }
}

void xMatrix_rowAdd(xMatrix *mat, int row1, int row2, float scale) {
    // check if matrix is not NULL
    if (mat == NULL) {
        return;
    }
    // validate index bounds
    if (row1 >= mat->rows || row2 >= mat->rows) {
        return;
    }

    for (int i = 0; i < mat->cols; i++) {
        mat->data[row1][i] += mat->data[row2][i] * scale;
    }
}

void xMatrix_colAdd(xMatrix *mat, int col1, int col2, float scale) {
    // check if matrix is not NULL
    if (mat == NULL) {
        return;
    }
    // validate index bounds
    if (col1 >= mat->cols || col2 >= mat->cols) {
        return;
    }

    for (int i = 0; i < mat->rows; i++) {
        mat->data[i][col1] += mat->data[i][col2] * scale;
    }
}

void xMatrix_rowScale(xMatrix *mat, int rowIndex, float scale) {
    // check if matrix is not NULL
    if (mat == NULL) {
        return;
    }
    // check index bounds
    if (rowIndex >= mat->rows) {
        return;
    }

    for (int i = 0; i < mat->cols; i++) {
        mat->data[rowIndex][i] *= scale;
    }
}

void xMatrix_colScale(xMatrix *mat, int colIndex, float scale) {
    // check if matrix is not NULL
    if (mat == NULL) {
        return;
    }
    // check index bounds
    if (colIndex >= mat->cols) {
        return;
    }

    for (int i = 0; i < mat->rows; i++) {
        mat->data[i][colIndex] *= scale;
    }
}

float xMatrix_get(xMatrix *mat, int rowIndex, int colIndex) {
    // check if matrix is NULL
    if (mat == NULL) {
        return 0.f;
    }
    // check index bounds
    if (rowIndex >= mat->rows || colIndex >= mat->cols) {
        return 0.f;
    }

    return mat->data[rowIndex][colIndex];
}

void xMatrix_set(xMatrix *mat, int rowIndex, int colIndex, float value) {
    // check if matrix is NULL
    if (mat == NULL) {
        return;
    }
    // check index bounds
    if (rowIndex >= mat->rows || colIndex >= mat->cols) {
        return;
    }

    mat->data[rowIndex][colIndex] = value;
}

xMatrix *xMatrix_flatten(xMatrix *mat, int axis) {
    // check if matrix is NULL
    if (mat == NULL) {
        return NULL;
    }

    xMatrix *retMatrix = NULL;
    // flatten along rows (axis = 0)
    if (axis == 0) {
        retMatrix = xMatrix_new(1, mat->rows * mat->cols);
        // check if matrix is successfully allocated
        if (retMatrix == NULL) {
            return NULL;
        }

        // assign values to new matrix
        for (int i = 0; i < mat->rows; i++) {
            for (int j = 0; j < mat->cols; j++) {
                retMatrix->data[0][i * mat->cols + j] = mat->data[i][j];
            }
        }
    }
    // flatten matrix along columns (axis != 0)
    else {
        retMatrix = xMatrix_new(mat->rows * mat->cols, 1);
        // check if matrix is successfully allocated
        if (retMatrix == NULL) {
            return NULL;
        }

        // assign values to new matrix
        for (int i = 0; i < mat->rows; i++) {
            for (int j = 0; j < mat->cols; j++) {
                retMatrix->data[i * mat->cols + j][0] = mat->data[i][j];
            }
        }
    }

    return retMatrix;
}

xMatrix *xMatrix_unwrap(xMatrix *mat, int rows, int cols, int axis) {
    // check if matrix is NULL
    if (mat == NULL) {
        return NULL;
    }

    xMatrix *retMatrix = NULL;
    // unwrap along rows (axis = 0)
    if (axis == 0) {
        // check if matrix can be unwrapped to given dimensions
        if (mat->cols != rows * cols) {
            return NULL;
        }

        retMatrix = xMatrix_new(rows, cols);
        // check if matrix is successfully allocated
        if (retMatrix == NULL) {
            return NULL;
        }

        // assign values to new matrix
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                retMatrix->data[i][j] = mat->data[0][i * cols + j];
            }
        }
    }
    // unwrap along columns (axis != 0)
    else {
        // check if matrix can be unwrapped to given dimensions
        if (mat->rows != rows * cols) {
            return NULL;
        }

        retMatrix = xMatrix_new(rows, cols);
        // check if matrix is successfully allocated
        if (retMatrix == NULL) {
            return NULL;
        }

        // assign values to new matrix
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                retMatrix->data[i][j] = mat->data[i * cols + j][0];
            }
        }
    }

    return retMatrix;
}
