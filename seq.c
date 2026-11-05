#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

double** alloc_matrix(int n) {
    double** mat = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) mat[i] = (double*)malloc(n * sizeof(double));
    return mat;
}

void free_matrix(double** mat, int n) {
    for (int i = 0; i < n; i++) free(mat[i]);
    free(mat);
}

void fill_matrix(double** mat, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            mat[i][j] = (double)(rand() % 256);
}

void copy_matrix(double** src, double** dst, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            dst[i][j] = src[i][j];
}

void erode(double** src, double** dst, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double min_val = src[i][j];
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < n && nj >= 0 && nj < n) {
                        if (src[ni][nj] < min_val) min_val = src[ni][nj];
                    }
                }
            }
            dst[i][j] = min_val;
        }
    }
}

void dilate(double** src, double** dst, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double max_val = src[i][j];
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < n && nj >= 0 && nj < n) {
                        if (src[ni][nj] > max_val) max_val = src[ni][nj];
                    }
                }
            }
            dst[i][j] = max_val;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Использование: %s <N> <K>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int K = atoi(argv[2]);

    srand(42);

    double** Orig = alloc_matrix(N);
    double** BufE = alloc_matrix(N);
    double** BufD = alloc_matrix(N);
    double** TempE = alloc_matrix(N);
    double** TempD = alloc_matrix(N);

    fill_matrix(Orig, N);
    copy_matrix(Orig, BufE, N);
    copy_matrix(Orig, BufD, N);

    double start_time = get_time();

    for (int k = 0; k < K; k++) {
        erode(BufE, TempE, N);
        double** swapE = BufE; BufE = TempE; TempE = swapE;

        dilate(BufD, TempD, N);
        double** swapD = BufD; BufD = TempD; TempD = swapD;
    }

    double end_time = get_time();
    printf("Время выполнения: %f секунд\n", end_time - start_time);

    free_matrix(Orig, N);
    free_matrix(BufE, N);
    free_matrix(BufD, N);
    free_matrix(TempE, N);
    free_matrix(TempD, N);

    return 0;
}
