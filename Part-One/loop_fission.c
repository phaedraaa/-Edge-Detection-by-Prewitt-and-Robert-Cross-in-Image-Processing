#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define N 200 /* frame dimension for QCIF format */
#define M 200 /* frame dimension for QCIF format */
#define filename "sunflower_200x200_444.yuv"
#define file_prewitt "sunflower_prewitt_output.yuv"
#define file_roberts "sunflower_roberts_output.yuv"

/* Prewitt Masks */
int Gx_Prewitt[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
int Gy_Prewitt[3][3] = {{1, 1, 1}, {0, 0, 0}, {-1, -1, -1}};

/* Roberts Cross Masks */
int Gx_Roberts[2][2] = {{1, 0}, {0, -1}};
int Gy_Roberts[2][2] = {{0, 1}, {-1, 0}};

/* Frame data */
#pragma arm section zidata="ram"
int current_y[N][M];
int current_u[N][M];
int current_v[N][M];
#pragma arm section

int edge_prewitt[N][M];
int edge_roberts[N][M];

int i,j,x,y;

/* Read YUV frame */
void read()
{
  FILE *frame_c;
  if ((frame_c = fopen(filename, "rb")) == NULL)
  {
    printf("current frame doesn't exist\n");
    exit(-1);
  }

  // Loop Fission
  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      current_y[i][j] = fgetc(frame_c);

  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      current_u[i][j] = fgetc(frame_c);

  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      current_v[i][j] = fgetc(frame_c);

  fclose(frame_c);
}

/* Write Prewitt result */
void write_prewitt()
{
  FILE *frame_yuv;
  frame_yuv = fopen(file_prewitt, "wb");

  // Loop Fission
  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      fputc(edge_prewitt[i][j], frame_yuv);

  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      fputc(current_u[i][j], frame_yuv);

  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      fputc(current_v[i][j], frame_yuv);

  fclose(frame_yuv);
}

/* Write Roberts result */
void write_roberts()
{
  FILE *frame_yuv;
  frame_yuv = fopen(file_roberts, "wb");

  // Loop Fission: ??a????sµ?? e???af?? Y, U, V
  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      fputc(edge_roberts[i][j], frame_yuv);

  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      fputc(current_u[i][j], frame_yuv);

  for ( i = 0; i < N; i++)
    for ( j = 0; j < M; j++)
      fputc(current_v[i][j], frame_yuv);

  fclose(frame_yuv);
}

/* Apply Prewitt Operator */
void apply_prewitt()
{
  int gx_temp[N][M], gy_temp[N][M];

  // Loop Fission
  for ( i = 1; i < N - 1; i++)
    for ( j = 1; j < M - 1; j++)
    {
      int gx = 0;
      for ( x = 0; x < 3; x++)
        for ( y = 0; y < 3; y++)
          gx += current_y[i + x - 1][j + y - 1] * Gx_Prewitt[x][y];
      gx_temp[i][j] = gx;
    }

  // Loop Fission
  for ( i = 1; i < N - 1; i++)
    for ( j = 1; j < M - 1; j++)
    {
      int gy = 0;
      for ( x = 0; x < 3; x++)
        for ( y = 0; y < 3; y++)
          gy += current_y[i + x - 1][j + y - 1] * Gy_Prewitt[x][y];
      gy_temp[i][j] = gy;
    }

  // Loop Fission
  for ( i = 1; i < N - 1; i++)
    for ( j = 1; j < M - 1; j++)
    {
      edge_prewitt[i][j] = (int)sqrt(gx_temp[i][j] * gx_temp[i][j] + gy_temp[i][j] * gy_temp[i][j]);
      if (edge_prewitt[i][j] > 255)
        edge_prewitt[i][j] = 255;
      else if (edge_prewitt[i][j] < 0)
        edge_prewitt[i][j] = 0;
    }
}

/* Apply Roberts Operator */
void apply_roberts()
{
  int gx_temp[N][M], gy_temp[N][M];

  // Loop Fission
  for ( i = 0; i < N - 1; i++)
    for ( j = 0; j < M - 1; j++)
    {
      int gx = 0;
      for ( x = 0; x < 2; x++)
        for ( y = 0; y < 2; y++)
          gx += current_y[i + x][j + y] * Gx_Roberts[x][y];
      gx_temp[i][j] = gx;
    }

  // Loop Fission
  for ( i = 0; i < N - 1; i++)
    for ( j = 0; j < M - 1; j++)
    {
      int gy = 0;
      for ( x = 0; x < 2; x++)
        for ( y = 0; y < 2; y++)
          gy += current_y[i + x][j + y] * Gy_Roberts[x][y];
      gy_temp[i][j] = gy;
    }

  // Loop Fission
  for ( i = 0; i < N - 1; i++)
    for ( j = 0; j < M - 1; j++)
    {
      edge_roberts[i][j] = (int)sqrt(gx_temp[i][j] * gx_temp[i][j] + gy_temp[i][j] * gy_temp[i][j]);
      if (edge_roberts[i][j] > 255)
        edge_roberts[i][j] = 255;
      else if (edge_roberts[i][j] < 0)
        edge_roberts[i][j] = 0;
    }
}

int main()
{
  read();
  apply_prewitt();
  apply_roberts();
  write_prewitt();
  write_roberts();
  return 0;
}


