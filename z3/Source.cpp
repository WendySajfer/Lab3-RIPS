#include <iostream>
#include "windows.h"
#include <mutex>
#include <thread>
#include <future>
#include <vector>
#include <math.h>
#include <ctime>
#include <random>
#include <fstream>
#include <string>

using namespace std;

vector<vector<double>> matrix_A;
vector<vector<double>> matrix_B;

bool Check(int K, int n) {
	if (K <= 0 || K > 40)
		return false;
	if (n <= 0)
		return false;
	return true;
}
void Create_A_B(int n) {
	vector<vector<double>> matrix_A_;
	vector<vector<double>> matrix_B_;
	vector<double> str_matrix;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			str_matrix.push_back(rand() % 10);
		}
		matrix_A_.push_back(str_matrix);
		str_matrix.clear();
	}
	matrix_A = matrix_A_;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			str_matrix.push_back(rand() % 10);
		}
		matrix_B_.push_back(str_matrix);
		str_matrix.clear();
	}
	matrix_B = matrix_B_;
}

double function_composition(int i_index, int j_index, int n) //произедение i строки и j столбца
{
	double sum = 0;
	for (int i = 0; i < n; i++)
	{
		sum += matrix_A[i_index][i] * matrix_B[i][j_index];
	}
	return sum;
}

void Out_file1(vector<vector<double>> matirix_Result1, int n) {
	ofstream fout1;
	fout1.open("1_thread.txt", std::ofstream::out | std::ofstream::trunc);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			fout1 << matirix_Result1[i][j] << " ";
		}
		fout1 << endl;
	}
	fout1.close();
}

void Out_fileK(vector<vector<double>> matirix_ResultK, int n) {
	ofstream fout;
	fout.open("K_thread.txt", std::ofstream::out | std::ofstream::trunc);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			fout << matirix_ResultK[i][j] << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*Функция для асинхрона*/
void Task_Matrix_Composition(promise<vector<vector<double>>>&& matrix_prom, future<vector<vector<double>>>& matrix_future, int num_thread, int first_count, int n, int number_iteration)
{
	string Name = "Thread " + std::to_string(num_thread);
	static mutex m;
	vector<double> str_matrix;
	vector<vector<double>> matrix_C;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			str_matrix.push_back(0);
		}
		matrix_C.push_back(str_matrix);
		str_matrix.clear();
	}
	int buf_m = first_count % n, buf_n = (first_count - buf_m) / n;
	int buf_n_ = buf_n, buf_m_ = buf_m;
	for (int i = 0; i < number_iteration; i++)
	{
		//вычисление элементов произведения
		matrix_C[buf_n][buf_m] = function_composition(buf_n, buf_m, n);
		buf_m++;
		if (buf_m == n)
		{
			buf_m = 0;
			buf_n++;
		}
	}
	// Получить мьютекс.
	m.lock();
	vector<vector<double>> matrix_C_prom = matrix_future.get();
	matrix_prom = promise<vector<vector<double>>>();
	for (int i = 0; i < number_iteration; i++)
	{
		//заполнение элементами произведения в матрицу из promise
		matrix_C_prom[buf_n_][buf_m_] = matrix_C[buf_n_][buf_m_];
		buf_m_++;
		if (buf_m_ == n)
		{
			buf_m_ = 0;
			buf_n_++;
		}
	}
	//передача promise матрицы
	matrix_prom.set_value(matrix_C_prom);
	matrix_future = matrix_prom.get_future();
	// Освободить мьютекс.
	m.unlock();
}


int main() {
	setlocale(LC_ALL, "Rus");
	SetConsoleCP(1251);
	int task, exit = 1;
	int K, n;
	while (exit == 1) {
		cout << "1.Matrix_Composition" << endl << "2.Exit" << endl << "Choose a way:" << endl;
		cin.clear();
		cin >> task;
		switch (task)
		{
		case 1:
		{
			cout << "Input data:" << endl << "K = ";
			cin >> K;
			cout << "n = ";
			cin.clear();
			cin.ignore(1000, '\n');
			cin >> n;
		}
		if (Check(K, n)) {
			Create_A_B(n);
			int count_n = n * n, r_thread = count_n % K, K_min = (count_n - r_thread) / K;
			int count_first = 0;
			unsigned int start_time = 0, end_time = 0;
			vector<double> str_matrix;
			vector<vector<double>> matrix_C;
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					str_matrix.push_back(0);
				}
				matrix_C.push_back(str_matrix);
				str_matrix.clear();
			}
			promise<vector<vector<double>>> matrix_p;
			matrix_p.set_value(matrix_C);
			future<vector<vector<double>>> matrix_f = matrix_p.get_future();

			vector<thread> t_vector;

			start_time = clock();
			for (int i = 0; i < r_thread; i++)
			{
				t_vector.push_back(thread(Task_Matrix_Composition, move(matrix_p), ref(matrix_f) ,i + 1, count_first, n, K_min + 1));
				count_first += (K_min + 1);
			}
			for (int i = r_thread; i < K; i++)
			{
				t_vector.push_back(thread(Task_Matrix_Composition, move(matrix_p), ref(matrix_f), i + 1, count_first, n, K_min));
				count_first += K_min;
			}
			int k = 0;
			while (k < K) {
				t_vector[k].join();
				k++;
			}
			matrix_C = matrix_f.get();
			end_time = clock();
			cout << "Time of running: " << end_time - start_time << " ms." << endl;
			Out_fileK(matrix_C, n);
			thread t1;
			matrix_p = promise<vector<vector<double>>>();
			matrix_p.set_value(matrix_C);
			matrix_f = matrix_p.get_future();
			start_time = clock();
			t1 = thread(Task_Matrix_Composition, move(matrix_p), ref(matrix_f), 0, 0, n, n*n);
			t1.join();
			matrix_C = matrix_f.get();
			end_time = clock();
			cout << "Time of running1: " << end_time - start_time << " ms." << endl;
			Out_file1(matrix_C, n);
		}
		else cout << "Invalid number. Try again." << endl;
		cin.clear();
		break;
		case 2:
			exit = 0;
			break;
		default:
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "This task does not exist" << endl;
			break;
		}
	}
	system("pause");
	return 0;
}
/*
4
10

16
500
*/