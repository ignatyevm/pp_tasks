#include <iostream>
#include <vector>
#include <tuple>
#include <random>
#include <fstream>
#include <mpi.h>

void merge_sort(int* arr, int l, int r);
void merge(int* arr, int l, int m, int r);

std::vector<int> get_data(int argc, char** argv);
void print_arr(int* arr, int n);
void print_arr(int* arr, int n, int pid);

static bool PRINT_ARRAY = false;
static bool PRINT_TIME = false;

int main(int argc, char** argv) {
    int pid, pcount, n = 0, fake_count = 0;
    float time;
    int* arr = nullptr;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &pcount);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    if (pcount != pow(2, int(log2(pcount)))) {
        if (pid == 0) {
            std::cout << "Process count must be power of 2" << std::endl;
        }
        MPI_Finalize();
        return 0;
    }
    if (pid == 0) {
        std::vector<int> data = get_data(argc, argv);
        n = data.size();
        int p = int(pow(2, int(ceil(log2(n)))));
        fake_count = p - n;
        data.resize(p, INT_MAX);
        n = p;
        arr = new int[n];
        std::copy(data.begin(), data.end(), arr);
        time = MPI_Wtime();
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int m = n / pcount;
    int* local_arr = new int[m];
    MPI_Scatter(arr, m, MPI_INT, local_arr, m, MPI_INT, 0, MPI_COMM_WORLD);
    merge_sort(local_arr, 0, m);
    MPI_Gather(local_arr, m, MPI_INT, arr, m, MPI_INT, 0, MPI_COMM_WORLD);
    if (pid == 0) {
        int p = pcount / 2;
        int q = 2 * m;
        delete[] local_arr;
        local_arr = nullptr;
        while (p > 1) {
            for (int i = p; i < pcount; i++) {
                int act = 0;
                MPI_Send(&act, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
            for (int i = 1; i < p; i++) {
                int act = 1;
                MPI_Send(&act, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&q, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(arr + (i * q), q, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
            merge(arr, 0, q / 2, q);
            MPI_Status status;
            for (int i = 1; i < p; i++) {
                int* buffer = new int[q];
                MPI_Recv(buffer, q, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                for (int j = 0; j < q; j++) {
                    arr[i * q + j] = buffer[j];
                }
                delete[] buffer;
                buffer = nullptr;
            }
            p /= 2;
            q *= 2;
        }
        if (pcount > 1) {
            merge(arr, 0, n / 2, n);
            int act = 0;
            MPI_Send(&act, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            n -= fake_count;
            int *temp_arr = new int[n];
            for (int i = 0; i < n; i++) {
                temp_arr[i] = arr[i];
            }
            delete[] arr;
            arr = temp_arr;
        }
    } else {
        int act;
        MPI_Status status;
        MPI_Recv(&act, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        while (act != 0) {
            delete[] local_arr;
            local_arr = nullptr;
            int q;
            MPI_Recv(&q, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            local_arr = new int[q];
            MPI_Recv(local_arr, q, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            merge(local_arr, 0, q / 2, q);
            MPI_Send(local_arr, q, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Recv(&act, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }
    }
    if (pid == 0) {
        float end_time = MPI_Wtime() - time;
        if (PRINT_ARRAY) print_arr(arr, n);
        // std::cout << "Sorted correctly? " << (std::is_sorted(arr, arr + n) ? "Yes" : "No") << std::endl;
        if (PRINT_TIME) std::cout << "Time elapsed: " << end_time << std::endl;
    }
    MPI_Finalize();
    return 0;
}

void merge_sort(int* arr, int l, int r) {
    if (r - l == 1) {
        return;
    }
    int m = (l + r) / 2;
    merge_sort(arr, l, m);
    merge_sort(arr, m, r);
    merge(arr, l, m, r);
}

void merge(int* arr, int l, int m, int r) {
    int k1 = l;
    int k2 = m;
    int k = 0;
    int* arr_cpy = new int[r - l];
    while (k1 < m && k2 < r) {
        if (arr[k1] < arr[k2]) {
            arr_cpy[k++] = arr[k1++];
        } else {
            arr_cpy[k++] = arr[k2++];
        }
    }
    for (int i = k1; i < m; i++) {
        arr_cpy[k++] = arr[i];
    }
    for (int i = k2; i < r; i++) {
        arr_cpy[k++] = arr[i];
    }
    k = l;
    for (int i = 0; i < r - l; i++) {
        arr[k++] = arr_cpy[i];
    }
    delete[] arr_cpy;
}

void print_arr(int* arr, int n) {
    for (int j = 0; j < n; j++) {
        std::cout << arr[j] << " ";
    }
    std::cout << std::endl;
}

void print_arr(int* arr, int n, int pid) {
    std::cout << pid << ": ";
    print_arr(arr, n);
}

std::vector<int> get_data(int argc, char** argv) {
    std::vector<int> data;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::string mode(argv[1]);
    if (mode == "-random") {
        int n = std::atoi(argv[2]);
        std::uniform_int_distribution<> distribution(INT_MIN, INT_MAX);
        for (int i = 0; i < n; i++) {
            data.push_back(distribution(generator));
        }
    } else if (mode == "-file"){
        std::string filepath(argv[2]);
        std::ifstream file(filepath);
        int a;
        while (!file.eof()) {
            file >> a;
            data.push_back(a);
        }
    }
    for (int i = 3;  i < argc; i++) {
        std::string extra(argv[i]);
        if (extra == "-print") {
            PRINT_ARRAY = true;
        } else if (extra == "-time") {
            PRINT_TIME = true;
        }
    }
    return data;
}