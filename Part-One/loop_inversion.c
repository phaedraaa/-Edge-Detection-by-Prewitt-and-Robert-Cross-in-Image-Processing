#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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

int i, j;

/* Read YUV frame */
void read()
{
  FILE *frame_c;
  if ((frame_c = fopen(filename, "rb")) == NULL)
  {
    printf("current frame doesn't exist\n");
    exit(-1);
  }

  for (i = 0; i < N; i++)
  {
    for (j = 0; j < M; j++)
    {
      current_y[i][j] = fgetc(frame_c);
    }
  }
  for (i = 0; i < N; i++)
  {
    for (j = 0; j < M; j++)
    {
      current_u[i][j] = fgetc(frame_c);
    }
  }
  for (i = 0; i < N; i++)
  {
    for (j = 0; j < M; j++)
    {
      current_v[i][j] = fgetc(frame_c);
    }
  }
  fclose(frame_c);
}

/* Write Prewitt result */
void write_prewitt()
{
  FILE *frame_yuv;
  frame_yuv=fopen(file_prewitt,"wb");

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      fputc(edge_prewitt[i][j],frame_yuv);
    }
  }
  
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      fputc(current_u[i][j],frame_yuv);
    }
  }

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      fputc(current_v[i][j],frame_yuv);
    }
  }
  fclose(frame_yuv);
}

/* Write Roberts result */
void write_roberts()
{
  FILE *frame_yuv;
  frame_yuv = fopen(file_roberts, "wb");

  for (i = 0; i < N; i++)
  {
    for (j = 0; j < M; j++)
    {
      fputc(edge_roberts[i][j], frame_yuv);
    }
  }
  
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      fputc(current_u[i][j],frame_yuv);
    }
  }

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      fputc(current_v[i][j],frame_yuv);
    }
  }
  
  fclose(frame_yuv);
}

void apply_prewitt()
{
  int x, y, gx, gy;

  i = 1;
  while (i < N - 1) 
  {
    j = 1;
    while (j < M - 1) 
    {
      gx = 0;
      gy = 0;

      for (x = 0; x < 3; x++) 
      {
        for (y = 0; y < 3; y++)
        {
          gx += current_y[i + x - 1][j + y - 1] * Gx_Prewitt[x][y];
          gy += current_y[i + x - 1][j + y - 1] * Gy_Prewitt[x][y];
        }
      }

      edge_prewitt[i][j] = (int)sqrt(gx * gx + gy * gy);

      if (edge_prewitt[i][j] > 255)
        edge_prewitt[i][j] = 255;
      else if (edge_prewitt[i][j] < 0)
        edge_prewitt[i][j] = 0;

      j++; 
    }
    i++; 
  }
}


void apply_roberts()
{
  int x, y, gx, gy;

  i = 0;
  while (i < N - 1) 
  {
    j = 0;
    while (j < M - 1) 
    {
      gx = 0;
      gy = 0;

      for (x = 0; x < 2; x++) 
      {
        for (y = 0; y < 2; y++)
        {
          gx += current_y[i + x][j + y] * Gx_Roberts[x][y];
          gy += current_y[i + x][j + y] * Gy_Roberts[x][y];
        }
      }

      edge_roberts[i][j] = (int)sqrt(gx * gx + gy * gy);

      if (edge_roberts[i][j] > 255)
        edge_roberts[i][j] = 255;
      else if (edge_roberts[i][j] < 0)
        edge_roberts[i][j] = 0;

      j++; 
    }
    i++; 
  }
}






int main()
{
  read();
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      printf("%d ",current_y[i][j]);
    }
	printf("\n");
  }
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      printf("%d ",current_u[i][j]);
    }
	printf("\n");
  }
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      printf("%d ",current_v[i][j]);
    }
	printf("\n");
  }
  /* Apply Prewitt and Roberts Cross Operators */
  apply_prewitt();
  apply_roberts();

  /* Write results */
  write_prewitt();
  write_roberts();

  return 0;
}