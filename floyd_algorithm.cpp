#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <mpi.h>

std::pair<int, int *> get_data(int argc, char **argv);

static bool PRINT_GRAPH = false;
static bool PRINT_TIME = false;

int main(int argc, char **argv) {
    int pid, pcount, n = 0;
    float time;
    int *adj_matrix = nullptr;
    const int INF = INT_MAX;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &pcount);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    if (pid == 0) {
        std::tie(n, adj_matrix) = get_data(argc, argv);
        time = MPI_Wtime();
    }
    int *rcount = new int[pcount];
    int *offsets = new int[pcount];
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
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
        adj_matrix = new int[n * n];
    }
    int* buffer = new int[n * n];
    for (int k = 0; k < n; k++) {
        MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(adj_matrix, n * n, MPI_INT, 0, MPI_COMM_WORLD);
        if (pid == 0) {
            for (int i = row_start; i < row_end; i++) {
                for (int j = 0; j < n; j++) {
                    adj_matrix[i * n + j] = std::min(adj_matrix[i * n + j],
                                                     adj_matrix[i * n + k] + adj_matrix[k * n + j]);
                }
            }
            for (int p = 1; p < pcount; p++) {
                MPI_Status status;
                MPI_Recv(buffer, n * n, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
                int src = status.MPI_SOURCE;
                for (int i = 0; i < rcount[src]; i++) {
                    for (int j = 0; j < n; j++) {
                        adj_matrix[(offsets[src] + i) * n + j] = buffer[i * n + j];
                    }
                }
            }
        } else {
            for (int i = row_start; i < row_end; i++) {
                for (int j = 0; j < n; j++) {
                    adj_matrix[i * n + j] = std::min(adj_matrix[i * n + j],
                                                     adj_matrix[i * n + k] + adj_matrix[k * n + j]);
                }
            }
            MPI_Send(adj_matrix + row_start * n, rcount[pid] * n, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    if (pid == 0) {
        float end_time = MPI_Wtime() - time;
        if (PRINT_GRAPH) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    std::cout << adj_matrix[i * n + j] << " ";
                }
                std::cout << std::endl;
            }
        }
        if (PRINT_TIME) std::cout << "Time elapsed: " << end_time << std::endl;
    }
    delete[] buffer;
    delete[] adj_matrix;
    MPI_Finalize();
    return 0;
}

std::pair<int, int *> get_data(int argc, char **argv) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(1, 1000);
    int n;
    int* adj_matrix;
    std::string mode(argv[1]);
    if (mode == "-random") {
        n = std::atoi(argv[2]);
        adj_matrix = new int[n * n];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                adj_matrix[i * n + j] = distribution(generator);
            }
        }
    } else if (mode == "-file"){
        std::string filepath(argv[2]);
        std::ifstream file(filepath);
        file >> n;
        adj_matrix = new int[n * n];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                file >> adj_matrix[i * n + j];
            }
        }
    }
    for (int i = 3; i < argc; i++) {
        std::string extra(argv[i]);
        if (extra == "-print") {
            PRINT_GRAPH = true;
        } else if (extra == "-time") {
            PRINT_TIME = true;
        }
    }
    return {n, adj_matrix};
}