#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace std;

void fill_diagonal(int start_row, int end_row, vector<vector<int>>& quadratic_matrix) {
	for (int i = start_row; i < end_row; i++) {
		auto maxInRow = *max_element(quadratic_matrix[i].begin(), quadratic_matrix[i].end());
		quadratic_matrix[i][i] = maxInRow;
	}
}

int main() {
	srand(time(0));
	int N = 100;

	vector<vector<int>> quadratic_matrix(N, vector<int>(N, 0));
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++)
		{
			quadratic_matrix[i][j] = rand() % 10 + 1;
		}
	}

	int num_thread = 5;
	vector<thread> threads;

	volatile long long warmUpSum = 0;
	for (int i = 0; i < N; i++) {
		warmUpSum += quadratic_matrix[i][i];
	}

	auto start = chrono::high_resolution_clock::now();

	for (int i = 0; i < num_thread; i++) {
		int start_row = i * N / num_thread;
		int end_row = (i + 1) * N / num_thread;
		threads.emplace_back(fill_diagonal, start_row, end_row, ref(quadratic_matrix));
	}

	for (auto& t : threads) {
		t.join();
	}

	auto end = chrono::high_resolution_clock::now();

	chrono::duration<double, milli> elapsed = end - start;

	long long diagonalSum = 0;
	for (int i = 0; i < N; i++) {
		diagonalSum += quadratic_matrix[i][i];
	}

	cout << "Dimension of matrix: " << N << " x " << N << endl;
	cout << "Warm-up sum: " << warmUpSum << endl;
	cout << "Time:" << elapsed.count() << "ms" << endl;
	cout << "Sum of main diagonal: " << diagonalSum << endl;

	return 0;
}