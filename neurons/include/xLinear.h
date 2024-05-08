/**
 * @file xLinear.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Basic linear algebra library for C.
 * @version 0.5
 * @date 05.03.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 */

#ifndef XLINEAR_H
#define XLINEAR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t rows;  // number of rows
    uint32_t cols;  // number of columns
    float *data;    // matrix data
} xMatrix;

// constructor/destructor
// ----------------------------------------------------------------------------------------------

/**
 * @brief Create new empty matrix.
 *
 * @param rows Number of rows.
 * @param cols Number of columns.
 * @return Pointer to allocated matrix. NULL if allocation fails or invalid
 * dimensions.
 */
xMatrix *xMatrix_new(uint32_t rows, uint32_t cols);

/**
 * @brief Free matrix from memory.
 *
 * @param mat Pointer to matrix.
 *
 * @note Make sure to remove dangling pointers to freed matrix.
 */
void xMatrix_free(xMatrix *mat);

// instantiating functions (allocates memory for returned values)
// ----------------------------------------------------------------------------------------------

/**
 * @brief Create new matrix from selected range in another matrix.
 *
 * @param mat Pointer to matrix.
 * @param row_start Starting index of slice row (inclusive).
 * @param row_end Ending index of slice row (exclusive, i.e. not included in slice).
 * @param col_start Starting index of slice column (inclusive).
 * @param col_end Ending index of slice column (exclusive, i.e. not included in slice).
 * @return Pointer to matrix slice. NULL if allocation fails or invalid indices.
 */
xMatrix *xMatrix_slice(xMatrix *mat, uint32_t row_start, uint32_t row_end, uint32_t col_start, uint32_t col_end);

/**
 * @brief Create identity matrix of given dimension.
 *
 * @param dim Dimension of identity matrix.
 * @return Pointer to identity matrix. NULL if allocation fails or invalid
 * dimension.
 */
xMatrix *xMatrix_identity(uint32_t dim);

/**
 * @brief Extract row from matrix into new matrix.
 *
 * @param mat Pointer to matrix.
 * @param rowIndex Index of row to extract.
 * @return Pointer to row matrix. NULL if allocation fails or invalid index.
 */
xMatrix *xMatrix_row(xMatrix *mat, uint32_t rowIndex);

/**
 * @brief Extract column from matrix into new matrix.
 *
 * @param mat Pointer to matrix.
 * @param colIndex Index of column to extract.
 * @return Pointer to column matrix. NULL if allocation fails or invalid index.
 */
xMatrix *xMatrix_col(xMatrix *mat, uint32_t colIndex);

// non-instantiating functions (does not allocate memory for returned values)
// ----------------------------------------------------------------------------------------------

/**
 * @brief Add two matrices and store result in third matrix.
 *
 * @param res Pointer to result matrix.
 * @param mat1 Pointer to first matrix.
 * @param mat2 Pointer to second matrix.
 *
 * @note Result matrix must be of same size as input matrices.
 *
 * @note Does nothing if any matrix is NULL or dimensions are invalid.
 */
void xMatrix_add(xMatrix *res, xMatrix *mat1, xMatrix *mat2);

/**
 * @brief Subtract two matrices and store result in third matrix.
 *
 * @param res Pointer to result matrix.
 * @param mat1 Pointer to first matrix.
 * @param mat2 Pointer to second matrix.
 *
 * @note Result matrix must be of same size as input matrices.
 *
 * @note Does nothing if any matrix is NULL or dimensions are invalid.
 */
void xMatrix_sub(xMatrix *res, xMatrix *mat1, xMatrix *mat2);

/**
 * @brief Multiply two matrices and store result in third matrix.
 *
 * @param res Pointer to result matrix.
 * @param mat1 Pointer to first matrix.
 * @param mat2 Pointer to second matrix.
 *
 * @note Result matrix must be of size (mat1->rows x mat2->cols).
 *
 * @warning Result matrix can not have same pointer as input matrices (due to in-place operations).
 *
 * @note Does nothing if any matrix is NULL or dimensions are invalid.
 */
void xMatrix_dot(xMatrix *res, xMatrix *mat1, xMatrix *mat2);

/**
 * @brief Transpose matrix in-place.
 *
 * @param mat Pointer to matrix.
 *
 * @note Does nothing if matrix is NULL.
 */
void xMatrix_transpose(xMatrix *mat);

/**
 * @brief Scale matrix by scalar in-place.
 *
 * @param mat Pointer to matrix.
 * @param scale Scalar value to scale matrix by.
 *
 * @note Does nothing if matrix is NULL.
 */
void xMatrix_scale(xMatrix *mat, float scale);

/**
 * @brief Add two matrix rows and store result in first row.
 *
 * @param mat Pointer to matrix.
 * @param row1 Index of first row.
 * @param row2 Index of second row.
 * @param scale Scalar value to scale second row by before adding to first row (does not affect second row values).
 *
 * @note Does nothing if index is out of bounds or matrix is NULL.
 */
void xMatrix_rowAdd(xMatrix *mat, uint32_t row1, uint32_t row2, float scale);

/**
 * @brief Add two matrix columns and store result in first column.
 *
 * @param mat Pointer to matrix.
 * @param col1 Index of first column.
 * @param col2 Index of second column.
 * @param scale Scalar value to scale second column by before adding to first column (does not affect second column values).
 *
 * @note Does nothing if index is out of bounds or matrix is NULL.
 */
void xMatrix_colAdd(xMatrix *mat, uint32_t col1, uint32_t col2, float scale);

/**
 * @brief Scale matrix row by scalar in-place.
 *
 * @param mat Pointer to matrix.
 * @param rowIndex Index of row.
 * @param scale Scalar value to scale row by.
 *
 * @note Does nothing if index is out of bounds or matrix is NULL.
 */
void xMatrix_rowScale(xMatrix *mat, uint32_t rowIndex, float scale);

/**
 * @brief Scale matrix column by scalar in-place.
 *
 * @param mat Pointer to matrix.
 * @param colIndex Index of column.
 * @param scale Scalar value to scale column by.
 *
 * @note Does nothing if index is out of bounds or matrix is NULL.
 */
void xMatrix_colScale(xMatrix *mat, uint32_t colIndex, float scale);

/**
 * @brief Get value from matrix at specified index.
 *
 * @param mat Pointer to matrix.
 * @param rowIndex Row index (0-based).
 * @param colIndex Column index (0-based).
 * @return Value at specified index. 0 if index is out of bounds or matrix is NULL.
 */
float xMatrix_get(xMatrix *mat, uint32_t rowIndex, uint32_t colIndex);

/**
 * @brief Set value in matrix at specified index.
 *
 * @param mat Pointer to matrix.
 * @param rowIndex Row index (0-based).
 * @param colIndex Column index (0-based).
 * @param value Value to set at specified index.
 *
 * @note Does nothing if index is out of bounds or matrix is NULL.
 */
void xMatrix_set(xMatrix *mat, uint32_t rowIndex, uint32_t colIndex, float value);

#ifdef __cplusplus
}
#endif

#endif  // XLINEAR_H
