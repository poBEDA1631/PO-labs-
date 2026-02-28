#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <thread>
#include <atomic>


using namespace std;
static atomic<int> result(0);


void xor_sum_calc(int start, int end, const vector<int>& data) {
	int local_sum = 0;
	for (int i = start; i < end; i++) {
		if (data[i] % 2 != 0)
		{
			local_sum = local_sum ^ data[i];
		}
	}

	int old_val, new_val;
	do {
		old_val = result.load();
		new_val = old_val ^ local_sum;
	} while (!result.compare_exchange_weak(old_val, new_val));
}

int main() {
	srand(time(0));
	int N = 10;
	vector<int> data(N);

	int num_thread = 10;
	vector<thread> threads;

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
	for (int i = 0; i < num_thread; i++) {
		int start_section = i * N / num_thread;
		int end_section = (i + 1) * N / num_thread;
		threads.emplace_back(xor_sum_calc, start_section, end_section, ref(data));
	}

	for (auto& t : threads) {
		t.join();
	}
	auto end = chrono::high_resolution_clock::now();

	chrono::duration<double, milli> elapsed = end - start;

	cout << "Dimension of matrix: " << N << endl;
	cout << "Warm-up sum: " << warmUpSum << endl;
	cout << "Time:" << elapsed.count() << "ms" << endl;
	cout << "Result:" << result << endl;

	return 0;
}