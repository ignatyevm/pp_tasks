#include <iostream>
#include <mpi.h>
#include <fstream>
#include <random>

static bool PRINT_MATRIX = false;
static bool PRINT_TIME = false;

typedef long long int lli;

lli* get_minor(int n, lli* matrix, int row, int col);
int determinant(int n, lli* matrix);
std::pair<int, lli*> get_data(int argc, char **argv);

int main(int argc, char** argv) {
    int pid, pcount, n = 0;
    float time;
    lli* matrix = nullptr;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &pcount);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    int has_close = 0;
    if (pid == 0) {
        std::tie(n, matrix) = get_data(argc, argv);
        time = MPI_Wtime();
        if (n == 1) {
            float end_time = MPI_Wtime() - time;
            std::cout << "det: " << matrix[0] << std::endl;
            if (PRINT_TIME) std::cout << "Time elapsed: " << end_time << std::endl;
            has_close = 1;
        }
    }
    MPI_Bcast(&has_close, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (has_close) {
        MPI_Finalize();
        return 0;
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int *rcount = new int[pcount];
    int *offsets = new int[pcount];
    int m = n / pcount;
    for (int i = 0; i < pcount; i++) {
        rcount[i] = m;
    }
    int r = n - m * pcount;
    for (int i = 0; i < pcount && r > 0; i++, r--) {
        rcount[i] += 1;
    }
    offsets[0] = 0;
    for (int i = 1; i < pcount; i++) {
        offsets[i] = offsets[i - 1] + rcount[i - 1];
    }
    int row_start = offsets[pid];
    int row_end;
    if (pid == pcount - 1) {
        row_end = n;
    } else {
        row_end = offsets[pid + 1];
    }
    if (pid != 0) {
        matrix = new lli[n * n];
    }
    MPI_Bcast(matrix, n * n, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
    int d = 0;
    if (pid == 0) {
        for (int i = row_start; i < row_end; i++) {
            lli* minor = get_minor(n, matrix, i, n - 1);
            lli det = matrix[i * n + n - 1] * determinant(n - 1, minor);
            if ((i + n - 1) % 2 == 1) {
                det *= -1;
            }
            d += det;
            delete[] minor;
        }
        lli* buffer = new lli[n];
        for (int i = 1; i < pcount; i++) {
            MPI_Status status;
            MPI_Recv(buffer, n, MPI_LONG_LONG_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            int src = status.MPI_SOURCE;
            for (int j = 0; j < rcount[src]; j++) {
                d += buffer[j];
            }
        }
        delete[] buffer;
    } else {
        lli* determinants = new lli[rcount[pid]]{0};
        int p = 0;
        for (int i = row_start; i < row_end; i++) {
            lli* minor = get_minor(n, matrix, i, n - 1);
            lli det = matrix[i * n + n - 1] * determinant(n - 1, minor);
            if ((i + n - 1) % 2 == 1) {
                det *= -1;
            }
            determinants[p++] = det;
            delete[] minor;
        }
        MPI_Send(determinants, rcount[pid], MPI_LONG_LONG_INT, 0, 0, MPI_COMM_WORLD);
        delete[] determinants;
    }
    if (pid == 0) {
        std::cout << "det = " << d << std::endl;
        float end_time = MPI_Wtime() - time;
        if (PRINT_MATRIX) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    std::cout << matrix[i * n + j] << " ";
                }
                std::cout << std::endl;
            }
        }
        if (PRINT_TIME) std::cout << "Time elapsed: " << end_time << std::endl;
    }
    delete[] offsets;
    delete[] rcount;
    delete[] matrix;
    MPI_Finalize();
    return 0;
}

std::pair<int, lli*> get_data(int argc, char **argv) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(1, 10);
    int n;
    lli* matrix;
    std::string mode(argv[1]);
    if (mode == "-random") {
        n = std::atoi(argv[2]);
        matrix = new lli[n * n];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                matrix[i * n + j] = distribution(generator);
            }
        }
    } else if (mode == "-file"){
        std::string filepath(argv[2]);
        std::ifstream file(filepath);
        file >> n;
        matrix = new lli[n * n];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                file >> matrix[i * n + j];
            }
        }
    }
    for (int i = 3; i < argc; i++) {
        std::string extra(argv[i]);
        if (extra == "-print") {
            PRINT_MATRIX = true;
        } else if (extra == "-time") {
            PRINT_TIME = true;
        }
    }
    return {n, matrix};
}

int determinant(int n, lli* matrix) {
    if (n == 1) {
        return matrix[0];
    }
    if (n == 2) {
        return matrix[0] * matrix[3] - matrix[1] * matrix[2];
    }
    int det = 0;
    for (int k = 0; k < n; k++) {
        lli* minor = get_minor(n, matrix, k, n - 1);
        int d = matrix[k * n + n - 1] * determinant(n - 1, minor);
        if ((k + n - 1) % 2 == 1) {
            d *= -1;
        }
        det += d;
    }
    return det;
}

lli* get_minor(int n, lli* matrix, int row, int col) {
    int p = 0;
    lli* minor = new lli[(n - 1) * (n - 1)];
    for (int i = 0; i < n; i++) {
        if (i == row) {
            continue;
        }
        for (int j = 0; j < n; j++) {
            if (j == col) {
                continue;
            }
            minor[p++] = matrix[i * n + j];
        }
    }
    return minor;
}