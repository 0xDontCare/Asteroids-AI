/**
 * @file xLinear.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Basic linear algebra library for C.
 * @version 0.3
 * @date 22.12.2023.
 *
 * @copyright All rights reserved (c) 2023
 *
 */

#ifndef XLINEAR_H
#define XLINEAR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Matrix structure.
 *
 */
typedef struct
{
    int rows;      // number of rows
    int cols;      // number of columns
    float **data;  // data matrix
} xMatrix;

// constructor/destructor
// ----------------------------------------------------------------------------------------------
xMatrix *xMatrix_new(int rows, int cols);
void xMatrix_free(xMatrix *mat);

// instantiating functions
// ----------------------------------------------------------------------------------------------
xMatrix *xMatrix_add(xMatrix *mat1, xMatrix *mat2);
xMatrix *xMatrix_sub(xMatrix *mat1, xMatrix *mat2);
xMatrix *xMatrix_dot(xMatrix *mat1, xMatrix *mat2);
xMatrix *xMatrix_slice(xMatrix *mat, int row_start, int row_end, int col_start, int col_end);
xMatrix *xMatrix_identity(int dim);
xMatrix *xMatrix_row(xMatrix *mat, int rowIndex);
xMatrix *xMatrix_col(xMatrix *mat, int colIndex);

// non-instantiating functions
// ----------------------------------------------------------------------------------------------
void xMatrix_transpose(xMatrix *mat);
void xMatrix_scale(xMatrix *mat, float scale);
void xMatrix_rowAdd(xMatrix *mat, int row1, int row2, float scale);
void xMatrix_colAdd(xMatrix *mat, int col1, int col2, float scale);
void xMatrix_rowScale(xMatrix *mat, int rowIndex, float scale);
void xMatrix_colScale(xMatrix *mat, int colIndex, float scale);
float xMatrix_get(xMatrix *mat, int rowIndex, int colIndex);
void xMatrix_set(xMatrix *mat, int rowIndex, int colIndex, float value);

// utility functions
// ----------------------------------------------------------------------------------------------
xMatrix *xMatrix_flatten(xMatrix *mat, int axis);
xMatrix *xMatrix_unwrap(xMatrix *mat, int rows, int cols, int axis);


#ifdef __cplusplus
}
#endif

#endif  // XLINEAR_H
