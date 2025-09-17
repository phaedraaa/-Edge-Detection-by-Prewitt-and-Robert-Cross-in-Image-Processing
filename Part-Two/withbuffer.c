#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define N 200 /* frame dimension for QCIF format */
#define M 200 /* frame dimension for QCIF format */
#define BUFFER_SIZE 16 /* Small buffer size */
#define filename "sunflower_200x200_444.yuv"
#define file_prewitt "sunflower_prewitt_output.yuv"
#define file_roberts "sunflower_roberts_output.yuv"

/* Prewitt Masks */
int Gx_Prewitt[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
int Gy_Prewitt[3][3] = {{1, 1, 1}, {0, 0, 0}, {-1, -1, -1}};

/* Roberts Cross Masks */
int Gx_Roberts[2][2] = {{1, 0}, {0, -1}};
int Gy_Roberts[2][2] = {{0, 1}, {-1, 0}};

/* Buffer for current_y data */
int buffer[BUFFER_SIZE][M];

#pragma arm section zidata="ram"
int current_y[N][M];
int current_u[N][M];
int current_v[N][M];
#pragma arm section

#pragma arm section zidata="cache"
int edge_prewitt[N][M];
int edge_roberts[N][M];
#pragma arm section

int j,i,row,x,y;

/* Read YUV frame */
void read()
{
  FILE *frame_c;
  if ((frame_c = fopen(filename, "rb")) == NULL)
  {
    printf("current frame doesn't exist\n");
    exit(-1);
  }


  for (j = 0; j < M; j++) 
  {
    for (i = 0; i < N; i++) 
    {
      current_y[i][j] = fgetc(frame_c);
    }
  }


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
    {
      current_u[i][j] = fgetc(frame_c);
    }
  }


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
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
  frame_yuv = fopen(file_prewitt, "wb");


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
    {
      fputc(edge_prewitt[i][j], frame_yuv);
    }
  }


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
    {
      fputc(current_u[i][j], frame_yuv);
    }
  }


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
    {
      fputc(current_v[i][j], frame_yuv);
    }
  }

  fclose(frame_yuv);
}

void write_roberts()
{
  FILE *frame_yuv;
  frame_yuv = fopen(file_roberts, "wb");


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
    {
      fputc(edge_roberts[i][j], frame_yuv);
    }
  }


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
    {
      fputc(current_u[i][j], frame_yuv);
    }
  }


  for (j = 0; j < M; j++)
  {
    for (i = 0; i < N; i++)
    {
      fputc(current_v[i][j], frame_yuv);
    }
  }

  fclose(frame_yuv);
}

/* Apply Prewitt Operator using Buffer */
void apply_prewitt_with_buffer()
{
    int gx, gy;

    for ( j = 1; j < M - 1; j += BUFFER_SIZE - 2)
    {
        /* Load BUFFER_SIZE rows into buffer */
        for ( row = 0; row < BUFFER_SIZE && (j + row - 1) < M; row++)
        {
            memcpy(buffer[row], current_y[j + row - 1], M * sizeof(int));
        }

        for ( row = 1; row < BUFFER_SIZE - 1 && (j + row) < M - 1; row++)
        {
            for ( i = 1; i < N - 1; i++)
            {
                gx = 0;
                gy = 0;

                for ( y = 0; y < 3; y++)
                {
                    for ( x = 0; x < 3; x++)
                    {
                        gx += buffer[row + x - 1][i + y - 1] * Gx_Prewitt[x][y];
                        gy += buffer[row + x - 1][i + y - 1] * Gy_Prewitt[x][y];
                    }
                }

                edge_prewitt[j + row][i] = (int)sqrt(gx * gx + gy * gy);

                if (edge_prewitt[j + row][i] > 255)
                    edge_prewitt[j + row][i] = 255;
                else if (edge_prewitt[j + row][i] < 0)
                    edge_prewitt[j + row][i] = 0;
            }
        }
    }
}

/* Apply Roberts Operator using Buffer */
void apply_roberts_with_buffer()
{
    int gx, gy;

    for ( j = 0; j < M - 1; j += BUFFER_SIZE - 1)
    {
        /* Load BUFFER_SIZE rows into buffer */
        for ( row = 0; row < BUFFER_SIZE && (j + row) < M; row++)
        {
            memcpy(buffer[row], current_y[j + row], M * sizeof(int));
        }

        for ( row = 0; row < BUFFER_SIZE - 1 && (j + row) < M - 1; row++)
        {
            for ( i = 0; i < N - 1; i++)
            {
                gx = 0;
                gy = 0;

                for ( y = 0; y < 2; y++)
                {
                    for ( x = 0; x < 2; x++)
                    {
                        gx += buffer[row + x][i + y] * Gx_Roberts[x][y];
                        gy += buffer[row + x][i + y] * Gy_Roberts[x][y];
                    }
                }

                edge_roberts[j + row][i] = (int)sqrt(gx * gx + gy * gy);

                if (edge_roberts[j + row][i] > 255)
                    edge_roberts[j + row][i] = 255;
                else if (edge_roberts[j + row][i] < 0)
                    edge_roberts[j + row][i] = 0;
            }
        }
    }
}

int main()
{
    /* Read input image */
    read();

    /* Apply Prewitt and Roberts with Buffer */
    apply_prewitt_with_buffer();
    apply_roberts_with_buffer();

    /* Write results */
    write_prewitt();
    write_roberts();

    return 0;
}
