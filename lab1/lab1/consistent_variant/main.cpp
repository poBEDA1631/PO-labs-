#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>

using namespace std;

int main() {
	srand(time(0));
	int N = 5;
	vector<vector<int>> quadratic_matrix(N, vector<int>(N, 0));


	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++)
		{
			quadratic_matrix[i][j] = rand() % 10 + 1;
			cout << quadratic_matrix[i][j] << "\t";
		}
		cout << endl;
	}

	for (int i = 0; i < N; i++) {
		int maxInRow = *max_element(quadratic_matrix[i].begin(), quadratic_matrix[i].end());
		quadratic_matrix[i][i] = maxInRow;
	}

	cout << "\nMatrix after mod: " << endl;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (i == j) {
				cout << "[" << quadratic_matrix[i][j] << "]\t";
			}
			else {
				cout << quadratic_matrix[i][j] << "\t";
			}
		}
		cout << endl;
	}
}