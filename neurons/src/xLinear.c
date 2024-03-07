#include "xLinear.h"

#include <stdint.h>
#include <stdlib.h>

xMatrix *xMatrix_new(uint32_t rows, uint32_t cols)
{
  // dimension checking
  if (rows == 0 || cols == 0 || rows > UINT32_MAX / cols || cols > UINT32_MAX / rows)
  {
    return NULL;
  }

  // matrix allocation
  xMatrix *mat = (xMatrix *)malloc(sizeof(xMatrix));
  if (mat == NULL)
  {
    return NULL;
  }

  // data allocation
  mat->data = (float *)calloc(rows * cols, sizeof(float));
  if (mat->data == NULL)
  {
    free(mat);
    return NULL;
  }

  // matrix initialization
  mat->rows = rows;
  mat->cols = cols;

  return mat;
}

void xMatrix_free(xMatrix *mat)
{
  free(mat->data);
  free(mat);
}

xMatrix *xMatrix_slice(xMatrix *mat, uint32_t row_start, uint32_t row_end, uint32_t col_start,
                       uint32_t col_end)
{
  // bound checking
  if (row_start >= row_end || col_start >= col_end)
  {
    return NULL;
  }

  // matrix allocation
  xMatrix *res = xMatrix_new(row_end - row_start, col_end - col_start);
  if (res == NULL)
  {
    return NULL;
  }

  // value copy
  for (uint32_t i = row_start; i < row_end; i++)
  {
    for (uint32_t j = col_start; j < col_end; j++)
    {
      xMatrix_set(res, i - row_start, j - col_start, xMatrix_get(mat, i, j));
    }
  }

  return res;
}

xMatrix *xMatrix_identity(uint32_t dim)
{
  // matrix allocation
  xMatrix *res = xMatrix_new(dim, dim);
  if (res == NULL)
  {
    return NULL;
  }

  // value initialization
  for (uint32_t i = 0; i < dim; i++)
  {
    xMatrix_set(res, i, i, 1.0f);
  }

  return res;
}

xMatrix *xMatrix_row(xMatrix *mat, uint32_t rowIndex)
{
  // matrix allocation
  xMatrix *res = xMatrix_new(1, mat->cols);
  if (res == NULL)
  {
    return NULL;
  }

  // value copy
  for (uint32_t i = 0; i < mat->cols; i++)
  {
    xMatrix_set(res, 0, i, xMatrix_get(mat, rowIndex, i));
  }

  return res;
}

xMatrix *xMatrix_col(xMatrix *mat, uint32_t colIndex)
{
  // matrix allocation
  xMatrix *res = xMatrix_new(mat->rows, 1);
  if (res == NULL)
  {
    return NULL;
  }

  // value copy
  for (uint32_t i = 0; i < mat->rows; i++)
  {
    xMatrix_set(res, i, 0, xMatrix_get(mat, i, colIndex));
  }

  return res;
}

void xMatrix_add(xMatrix *res, xMatrix *mat1, xMatrix *mat2)
{
  // pointer checking
  if (res == NULL || mat1 == NULL || mat2 == NULL)
  {
    return;
  }

  // dimension checking
  if (res->rows != mat1->rows || res->rows != mat2->rows || res->cols != mat1->cols ||
      res->cols != mat2->cols)
  {
    return;
  }

  // value addition
  for (uint32_t i = 0; i < res->rows; i++)
  {
    for (uint32_t j = 0; j < res->cols; j++)
    {
      xMatrix_set(res, i, j, xMatrix_get(mat1, i, j) + xMatrix_get(mat2, i, j));
    }
  }

  return;
}

void xMatrix_sub(xMatrix *res, xMatrix *mat1, xMatrix *mat2)
{
  // pointer checking
  if (res == NULL || mat1 == NULL || mat2 == NULL)
  {
    return;
  }

  // dimension checking
  if (res->rows != mat1->rows || res->rows != mat2->rows || res->cols != mat1->cols ||
      res->cols != mat2->cols)
  {
    return;
  }

  // value subtraction
  for (uint32_t i = 0; i < res->rows; i++)
  {
    for (uint32_t j = 0; j < res->cols; j++)
    {
      xMatrix_set(res, i, j, xMatrix_get(mat1, i, j) - xMatrix_get(mat2, i, j));
    }
  }

  return;
}

void xMatrix_dot(xMatrix *res, xMatrix *mat1, xMatrix *mat2)
{
  // pointer checking
  if (res == NULL || mat1 == NULL || mat2 == NULL || res == mat1 || res == mat2)
  {
    return;
  }

  // dimension checking
  if (res->rows != mat1->rows || res->cols != mat2->cols || mat1->cols != mat2->rows)
  {
    return;
  }

  // matrix multiplication
  for (uint32_t i = 0; i < res->rows; i++)
  {
    for (uint32_t j = 0; j < res->cols; j++)
    {
      float sum = 0.0f;
      for (uint32_t k = 0; k < mat1->cols; k++)
      {
        sum += xMatrix_get(mat1, i, k) * xMatrix_get(mat2, k, j);
      }
      xMatrix_set(res, i, j, sum);
    }
  }

  return;
}

void xMatrix_transpose(xMatrix *mat)
{
  // pointer checking
  if (mat == NULL)
  {
    return;
  }

  // temporary matrix allocation
  xMatrix *temp = xMatrix_new(mat->cols, mat->rows);
  if (temp == NULL)
  {
    return;
  }

  // value copy
  for (uint32_t i = 0; i < mat->rows; i++)
  {
    for (uint32_t j = 0; j < mat->cols; j++)
    {
      xMatrix_set(temp, j, i, xMatrix_get(mat, i, j));
    }
  }

  // matrix swap
  free(mat->data);
  mat->data = temp->data;
  mat->rows = temp->rows;
  mat->cols = temp->cols;
  free(temp);

  return;
}

void xMatrix_scale(xMatrix *mat, float scale)
{
  // pointer checking
  if (mat == NULL)
  {
    return;
  }

  // value scaling
  for (uint32_t i = 0; i < mat->rows; i++)
  {
    for (uint32_t j = 0; j < mat->cols; j++)
    {
      xMatrix_set(mat, i, j, xMatrix_get(mat, i, j) * scale);
    }
  }
}

void xMatrix_rowAdd(xMatrix *mat, uint32_t row1, uint32_t row2, float scale)
{
  // pointer checking
  if (mat == NULL)
  {
    return;
  }

  // index checking
  if (row1 >= mat->rows || row2 >= mat->rows)
  {
    return;
  }

  // value addition
  for (uint32_t i = 0; i < mat->cols; i++)
  {
    xMatrix_set(mat, row1, i, xMatrix_get(mat, row1, i) + xMatrix_get(mat, row2, i) * scale);
  }

  return;
}

void xMatrix_colAdd(xMatrix *mat, uint32_t col1, uint32_t col2, float scale)
{
  // pointer checking
  if (mat == NULL)
  {
    return;
  }

  // index checking
  if (col1 >= mat->cols || col2 >= mat->cols)
  {
    return;
  }

  // value addition
  for (uint32_t i = 0; i < mat->rows; i++)
  {
    xMatrix_set(mat, i, col1, xMatrix_get(mat, i, col1) + xMatrix_get(mat, i, col2) * scale);
  }

  return;
}

void xMatrix_rowScale(xMatrix *mat, uint32_t rowIndex, float scale)
{
  // pointer checking
  if (mat == NULL)
  {
    return;
  }

  // index checking
  if (rowIndex >= mat->rows)
  {
    return;
  }

  // value scaling
  for (uint32_t i = 0; i < mat->cols; i++)
  {
    xMatrix_set(mat, rowIndex, i, xMatrix_get(mat, rowIndex, i) * scale);
  }

  return;
}

void xMatrix_colScale(xMatrix *mat, uint32_t colIndex, float scale)
{
  // pointer checking
  if (mat == NULL)
  {
    return;
  }

  // index checking
  if (colIndex >= mat->cols)
  {
    return;
  }

  // value scaling
  for (uint32_t i = 0; i < mat->rows; i++)
  {
    xMatrix_set(mat, i, colIndex, xMatrix_get(mat, i, colIndex) * scale);
  }

  return;
}

float xMatrix_get(xMatrix *mat, uint32_t rowIndex, uint32_t colIndex)
{
  // pointer checking
  if (mat == NULL)
  {
    return 0.0f;
  }

  // index checking
  if (rowIndex >= mat->rows || colIndex >= mat->cols)
  {
    return 0.0f;
  }

  // value retrieval
  return mat->data[rowIndex * mat->cols + colIndex];
}

void xMatrix_set(xMatrix *mat, uint32_t rowIndex, uint32_t colIndex, float value)
{
  // pointer checking
  if (mat == NULL)
  {
    return;
  }

  // index checking
  if (rowIndex >= mat->rows || colIndex >= mat->cols)
  {
    return;
  }

  // value setting
  mat->data[rowIndex * mat->cols + colIndex] = value;

  return;
}
