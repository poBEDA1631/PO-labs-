#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <iomanip>
using namespace std;

int xor_sum_calc(const vector<int>& data, int N){
	int xor_sum = 0;
	for (int x : data) {
		if (x % 2 != 0)
		{
			xor_sum = xor_sum ^ x;
		}
	}
	return xor_sum;
}

int main() {
	srand(time(0));
	int N = 10;
	int result;
	vector<int> data(N);


	for (int i = 0; i < N; i++) {
			data[i] = rand() % 8;
	}

	cout << "--- data (first 10 elements) ---" << endl;
	for (int i = 0; i < (N > 10 ? 10 : N); i++) {
		cout << setw(4) << data[i];
	}
	cout << "-----------------------------" << endl;

	volatile long long warmUpSum = 0;
	for (int i = 0; i < N; i++) {
		warmUpSum += data[i];
	}

	auto start = chrono::high_resolution_clock::now();
	result = xor_sum_calc(data, N);
	auto end = chrono::high_resolution_clock::now();

	chrono::duration<double, milli> elapsed = end - start;

	cout << "Dimension of matrix: " << N << endl;
	cout << "Warm-up sum: " << warmUpSum << endl;
	cout << "Time:" << elapsed.count() << "ms" << endl;
	cout << "Result:" << result << endl;

	return 0;
}