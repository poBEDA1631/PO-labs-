#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <chrono>

using namespace std;

int main() {
	srand(time(0));
	int N = 5;
	vector<vector<int>> quadratic_matrix(N, vector<int>(N, 0));


	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++)
		{
			quadratic_matrix[i][j] = rand() % 10 + 1;
		}
	}

	volatile long long warmUpSum = 0;
	for (int i = 0; i < N; i++) {
		warmUpSum += quadratic_matrix[i][i];
	}

	auto start = chrono::high_resolution_clock::now();

	for (int i = 0; i < N; i++) {
		int maxInRow = *max_element(quadratic_matrix[i].begin(), quadratic_matrix[i].end());
		quadratic_matrix[i][i] = maxInRow;
	}

	auto end = chrono::high_resolution_clock::now();

	chrono::duration<double, milli> elapsed = end - start;

	long long diagonalSum = 0;
	for (int i = 0; i < N; i++) {
		diagonalSum += quadratic_matrix[i][i];
	}

	cout << "Dimension of matrix: " << N << " x " << N << endl;
	cout << "Warm-up sum: " << warmUpSum << " (Cache is ready!)" << endl;
	cout << "Time:" << elapsed.count() << "ms" << endl;
	cout << "Sum of main diagonal: " << diagonalSum << endl;

	return 0;
}