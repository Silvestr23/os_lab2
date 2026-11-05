#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

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

typedef struct {
    double** src;
    double** dst;
    int n;
    int start_row;
    int end_row;
    int is_erode;
} ThreadData;

void* worker(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int n = data->n;
    
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < n; j++) {
            double val = data->src[i][j];
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < n && nj >= 0 && nj < n) {
                        if (data->is_erode) {
                            if (data->src[ni][nj] < val) val = data->src[ni][nj];
                        } else {
                            if (data->src[ni][nj] > val) val = data->src[ni][nj];
                        }
                    }
                }
            }
            data->dst[i][j] = val;
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("Использование: %s <N> <K> <num_threads>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    srand(42);

    double** Orig = alloc_matrix(N);
    double** BufE = alloc_matrix(N);
    double** BufD = alloc_matrix(N);
    double** TempE = alloc_matrix(N);
    double** TempD = alloc_matrix(N);

    fill_matrix(Orig, N);
    copy_matrix(Orig, BufE, N);
    copy_matrix(Orig, BufD, N);

    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    ThreadData* tdata = (ThreadData*)malloc(num_threads * sizeof(ThreadData));

    int rows_per_thread = N / num_threads;
    int extra = N % num_threads;

    double start_time = get_time();

    for (int k = 0; k < K; k++) {
        int current_row = 0;
        
        for (int t = 0; t < num_threads; t++) {
            int chunk = rows_per_thread + (t < extra ? 1 : 0);
            tdata[t].src = BufE;
            tdata[t].dst = TempE;
            tdata[t].n = N;
            tdata[t].start_row = current_row;
            tdata[t].end_row = current_row + chunk;
            tdata[t].is_erode = 1;
            current_row += chunk;
            pthread_create(&threads[t], NULL, worker, &tdata[t]);
        }
        for (int t = 0; t < num_threads; t++) pthread_join(threads[t], NULL);

        current_row = 0;
        for (int t = 0; t < num_threads; t++) {
            int chunk = rows_per_thread + (t < extra ? 1 : 0);
            tdata[t].src = BufD;
            tdata[t].dst = TempD;
            tdata[t].n = N;
            tdata[t].start_row = current_row;
            tdata[t].end_row = current_row + chunk;
            tdata[t].is_erode = 0;
            current_row += chunk;
            pthread_create(&threads[t], NULL, worker, &tdata[t]);
        }
        for (int t = 0; t < num_threads; t++) pthread_join(threads[t], NULL);

        double** swapE = BufE; BufE = TempE; TempE = swapE;
        double** swapD = BufD; BufD = TempD; TempD = swapD;
    }

    double end_time = get_time();
    printf("Время выполнения: %f секунд\n", end_time - start_time);

    free_matrix(Orig, N);
    free_matrix(BufE, N);
    free_matrix(BufD, N);
    free_matrix(TempE, N);
    free_matrix(TempD, N);
    free(threads);
    free(tdata);

    return 0;
}
