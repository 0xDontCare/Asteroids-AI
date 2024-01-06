/**
 * @file xLinear.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Basic linear algebra library for C.
 * @version 0.4
 * @date 06.01.2024.
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

xMatrix *xMatrix_slice(xMatrix *mat, int row_start, int row_end, int col_start, int col_end);  // create new matrix containing slice of original matrix
xMatrix *xMatrix_identity(int dim);                                                            // create new identity matrix
xMatrix *xMatrix_row(xMatrix *mat, int rowIndex);                                              // create new matrix containing single row of original matrix
xMatrix *xMatrix_col(xMatrix *mat, int colIndex);                                              // create new matrix containing single column of original matrix

// non-instantiating functions
// ----------------------------------------------------------------------------------------------

void xMatrix_add(xMatrix *res, xMatrix *mat1, xMatrix *mat2);             // add two matrices and store result in `res`
void xMatrix_sub(xMatrix *res, xMatrix *mat1, xMatrix *mat2);             // subtract two matrices and store result in `res`
void xMatrix_dot(xMatrix *res, xMatrix *mat1, xMatrix *mat2);             // multiply two matrices and store result in `res`
void xMatrix_transpose(xMatrix *mat);                                     // transpose matrix
void xMatrix_scale(xMatrix *mat, float scale);                            // scale matrix by scalar value
void xMatrix_rowAdd(xMatrix *mat, int row1, int row2, float scale);       // add two rows of matrix and store result in `row1`
void xMatrix_colAdd(xMatrix *mat, int col1, int col2, float scale);       // add two columns of matrix and store result in `col1`
void xMatrix_rowScale(xMatrix *mat, int rowIndex, float scale);           // scale row of matrix by scalar value
void xMatrix_colScale(xMatrix *mat, int colIndex, float scale);           // scale column of matrix by scalar value
float xMatrix_get(xMatrix *mat, int rowIndex, int colIndex);              // get value from matrix
void xMatrix_set(xMatrix *mat, int rowIndex, int colIndex, float value);  // set value in matrix

// utility functions
// ----------------------------------------------------------------------------------------------
xMatrix *xMatrix_flatten(xMatrix *mat, int axis);                     // flatten matrix along specified axis
xMatrix *xMatrix_unwrap(xMatrix *mat, int rows, int cols, int axis);  // unwrap matrix along specified axis

#ifdef __cplusplus
}
#endif

#endif  // XLINEAR_H
