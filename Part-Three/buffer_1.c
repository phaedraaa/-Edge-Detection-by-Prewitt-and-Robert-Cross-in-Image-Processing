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
int edge_prewitt[N][M];
int edge_roberts[N][M];
#pragma arm section

int i, j;

/* Buffers */
#pragma arm section zidata="cache_L2"
int buffer_current_line[M]; // Buffer for one line of current frame
#pragma arm section

#pragma arm section zidata="cache_L1"
int buffer_block[3][3]; // Buffer for a 3x3 block (Prewitt)
int buffer_block_roberts[2][2]; // Buffer for a 2x2 block (Roberts)
#pragma arm section

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

/* Write Roberts result */
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

/* Apply Prewitt Operator using buffers */
void apply_prewitt_with_buffers()
{
    int x, y, gx, gy;

    for (j = 1; j < M - 1; j++)
    {
        // Load one line of current_y into buffer_current_line
        for (i = 0; i < N; i++)
        {
            buffer_current_line[i] = current_y[i][j];
        }

        for (i = 1; i < N - 1; i++)
        {
            gx = 0;
            gy = 0;

            // Load 3x3 block into buffer_block
            for (y = 0; y < 3; y++)
            {
                for (x = 0; x < 3; x++)
                {
                    buffer_block[x][y] = current_y[i + x - 1][j + y - 1];
                }
            }

            for (y = 0; y < 3; y++)
            {
                for (x = 0; x < 3; x++)
                {
                    gx += buffer_block[x][y] * Gx_Prewitt[x][y];
                    gy += buffer_block[x][y] * Gy_Prewitt[x][y];
                }
            }

            edge_prewitt[i][j] = (int)sqrt(gx * gx + gy * gy);

            if (edge_prewitt[i][j] > 255)
                edge_prewitt[i][j] = 255;
            else if (edge_prewitt[i][j] < 0)
                edge_prewitt[i][j] = 0;
        }
    }
}

/* Apply Roberts Operator using buffers */
void apply_roberts_with_buffers()
{
    int x, y, gx, gy;

    for (j = 0; j < M - 1; j++)
    {
        // Load the current line into buffer_current_line
        for (i = 0; i < N; i++)
        {
            buffer_current_line[i] = current_y[j][i];
        }

        for (i = 0; i < N - 1; i++)
        {
            gx = 0;
            gy = 0;

            // Load 2x2 block into buffer_block_roberts
            for (y = 0; y < 2; y++)
            {
                for (x = 0; x < 2; x++)
                {
                    buffer_block_roberts[x][y] = current_y[i + x][j + y];
                }
            }

            for (y = 0; y < 2; y++)
            {
                for (x = 0; x < 2; x++)
                {
                    gx += buffer_block_roberts[x][y] * Gx_Roberts[x][y];
                    gy += buffer_block_roberts[x][y] * Gy_Roberts[x][y];
                }
            }

            edge_roberts[i][j] = (int)sqrt(gx * gx + gy * gy);

            if (edge_roberts[i][j] > 255)
                edge_roberts[i][j] = 255;
            else if (edge_roberts[i][j] < 0)
                edge_roberts[i][j] = 0;
        }
    }
}

int main()
{
    read();

    /* Apply Prewitt and Roberts Operators using buffers */
    apply_prewitt_with_buffers();
    apply_roberts_with_buffers();

    /* Write results */
    write_prewitt();
    write_roberts();

    return 0;
}


