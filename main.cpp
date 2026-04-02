#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
using namespace std;

bool almost_equal(double a, double b, double eps = 1e-9) {
    return fabs(a - b) < eps;
}

// 1. 矩阵列内积：平凡算法（按列访问）
void col_dot_naive(const vector<double>& A,
    const vector<double>& x,
    vector<double>& y,
    int n) {
    for (int j = 0; j < n; ++j) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            sum += A[i * n + j] * x[i];
        }
        y[j] = sum;
    }
}

// 2. 矩阵列内积：cache 优化算法（按行访问）
void col_dot_cache(const vector<double>& A,
    const vector<double>& x,
    vector<double>& y,
    int n) {
    fill(y.begin(), y.end(), 0.0);
    for (int i = 0; i < n; ++i) {
        double xi = x[i];
        const double* row = &A[i * n];
        for (int j = 0; j < n; ++j) {
            y[j] += row[j] * xi;
        }
    }
}

// 3. 数组求和：平凡链式累加
double sum_naive(const vector<double>& a) {
    double s = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        s += a[i];
    }
    return s;
}

// 4. 数组求和：4 路累加
double sum_4way(const vector<double>& a) {
    double s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
    size_t i = 0, n = a.size();

    for (; i + 3 < n; i += 4) {
        s0 += a[i];
        s1 += a[i + 1];
        s2 += a[i + 2];
        s3 += a[i + 3];
    }

    double s = s0 + s1 + s2 + s3;
    for (; i < n; ++i) {
        s += a[i];
    }
    return s;
}
#include <chrono>

double benchmark_matrix(void (*func)(const vector<double>&, const vector<double>&, vector<double>&, int),
    const vector<double>& A, const vector<double>& x, vector<double>& y,
    int n, int repeat) {

    auto start = chrono::high_resolution_clock::now();
    for (int t = 0; t < repeat; ++t) {
        func(A, x, y, n);
    }
    auto end = chrono::high_resolution_clock::now();

    return chrono::duration<double, milli>(end - start).count() / repeat;
}

double benchmark_sum(double (*func)(const vector<double>&), const vector<double>& a, int repeat) {
    volatile double ans = 0.0;

    auto start = chrono::high_resolution_clock::now();
    for (int t = 0; t < repeat; ++t) {
        ans = func(a);
    }
    auto end = chrono::high_resolution_clock::now();

    return chrono::duration<double, milli>(end - start).count() / repeat;
}

int main() {
    // 1. 小规模正确性测试
    {
        int n = 4;
        vector<double> A = {
            1, 2, 3, 4,
            5, 6, 7, 8,
            9,10,11,12,
            13,14,15,16
        };
        vector<double> x = { 1, 2, 3, 4 };
        vector<double> y1(n), y2(n);

        col_dot_naive(A, x, y1, n);
        col_dot_cache(A, x, y2, n);

        cout << "Matrix correctness check:" << endl;
        for (int i = 0; i < n; ++i) {
            cout << y1[i] << "  " << y2[i] << endl;
        }

        vector<double> arr = { 1,2,3,4,5,6,7,8 };
        double s1 = sum_naive(arr);
        double s2 = sum_4way(arr);

        cout << "sum_naive = " << s1 << endl;
        cout << "sum_4way  = " << s2 << endl;
    }

    // 2. 矩阵性能测试
    cout << "\nMatrix benchmark:\n";
    for (int n : {128, 256, 512, 1024}) {
        vector<double> A(n * n), x(n), y(n);

        for (int i = 0; i < n * n; ++i) A[i] = 1.0 + (i % 10);
        for (int i = 0; i < n; ++i) x[i] = 1.0 + (i % 7);

        int repeat = (n <= 256 ? 200 : (n <= 512 ? 50 : 10));

        double t1 = benchmark_matrix(col_dot_naive, A, x, y, n, repeat);
        double t2 = benchmark_matrix(col_dot_cache, A, x, y, n, repeat);

        cout << "n = " << n
            << "  naive = " << t1 << " ms"
            << "  cache_opt = " << t2 << " ms" << endl;
    }

    // 3. 求和性能测试
    cout << "\nSum benchmark:\n";
    for (int n : {100000, 1000000, 10000000}) {
        vector<double> a(n);
        for (int i = 0; i < n; ++i) a[i] = 1.0 + (i % 13);

        int repeat = (n <= 1000000 ? 500 : 50);

        double t1 = benchmark_sum(sum_naive, a, repeat);
        double t2 = benchmark_sum(sum_4way, a, repeat);

        cout << "n = " << n
            << "  naive = " << t1 << " ms"
            << "  sum_4way = " << t2 << " ms" << endl;
    }

    return 0;
}
