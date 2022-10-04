#include <iostream>
#include "windows.h"
#include <mutex>
#include <thread>
#include <future>
#include <queue>
#include <math.h>
#include <ctime>
#include <string>

using namespace std;

double Sum = 0;

bool Check(int K, int n, int m, double a, double b, double c, double d) {
	if (K <= 0 || K > 40)
		return false;
	if (n <= 0)
		return false;
	if (m <= 0)
		return false;
	if (b <= a)
		return false;
	if (d <= c)
		return false;
	return true;
}

double function_integral(double x, double y) //функция f(x,y)
{
	return exp(x) * sin(y);
}

bool Task_Integral(int num_thread, int first_count, double n_length, double m_length, double a, int n, double c, int number_iteration)
{
	string Name = "Thread " + std::to_string(num_thread);
	int buf_n = first_count % n, buf_m = (first_count - buf_n) / n;
	double x0 = a + buf_n * n_length, y0 = c + buf_m * m_length;
	double S = m_length * n_length;
	double buf_Sum = 0;
	static mutex m;
	for (int i = 0; i < number_iteration; i++)
	{
		buf_n++;
		buf_Sum += function_integral(x0 + n_length / 2, y0 + m_length / 2) * S;
		if (buf_n < n)
		{
			x0 += n_length;
		}
		else
		{
			buf_n = 0;
			x0 = a;
			y0 += m_length;
		}
	}
	// Получить мьютекс.
	m.lock();
	//cout << Name << " buf_Sum = " << buf_Sum << endl;
	Sum += buf_Sum;
	// Освободить мьютекс.
	m.unlock();
	return true;
}


int main() {
	setlocale(LC_ALL, "Rus");
	SetConsoleCP(1251);
	int task, exit = 1;
	double a, b, c, d;
	int K, n, m;
	while (exit == 1) {
		cout << "1.Integral" << endl << "2.Exit" << endl << "Choose a way:" << endl;
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
			cout << "m = ";
			cin.clear();
			cin.ignore(1000, '\n');
			cin >> m;
			cout << "a = ";
			cin.clear();
			cin.ignore(1000, '\n');
			cin >> a;
			cout << "b = ";
			cin.clear();
			cin.ignore(1000, '\n');
			cin >> b;
			cout << "c = ";
			cin.clear();
			cin.ignore(1000, '\n');
			cin >> c;
			cout << "d = ";
			cin.clear();
			cin.ignore(1000, '\n');
			cin >> d;
		}
			if (Check(K, n, m, a, b, c, d)) {
				double ab = b - a, cd = d - c, n_length = ab / n, m_length = cd / m;
				int count_nm = n * m, r_thread = count_nm % K, K_min = (count_nm - r_thread) / K;
				int count_first = 0;
				unsigned int start_time = 0, end_time = 0;
				queue<future<bool>> queue_1;
				start_time = clock();
				for (int i = 0; i < r_thread; i++)
				{
					queue_1.push(async(Task_Integral, i + 1, count_first, n_length, m_length, a, n, c, K_min + 1));
					count_first += (K_min + 1);
				}
				for (int i = r_thread; i < K; i++)
				{
					queue_1.push(async(Task_Integral, i + 1, count_first, n_length, m_length, a, n, c, K_min));
					count_first += K_min;
				}
				while (!queue_1.empty()) {
					queue_1.front().wait();
					queue_1.pop();
				}
				end_time = clock();
				cout << "Time of running: " << end_time - start_time << " ms." << endl;
				cout << "Sum = " << Sum << endl;
				Sum = 0;
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
5
4
0
5
0
4

16
1000
1000
0
5
0
5
 */