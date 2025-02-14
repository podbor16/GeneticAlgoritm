#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <cstring>
#include <iomanip>
#include <windows.h>

using namespace std;

// Функция для инициализации популяции
int** initialize_population(int pop_size, int num_vars, const int* bits_per_var, int* individual_lengths) {
    // Вычисляем длины бинарных строк для каждого индивида
    for (int i = 0; i < pop_size; ++i) {
        individual_lengths[i] = 0;
        for (int j = 0; j < num_vars; ++j) {
            individual_lengths[i] += bits_per_var[j];
        }
    }

    // Создаем двумерный массив для популяции
    int** population = new int*[pop_size];
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 1);

    for (int i = 0; i < pop_size; ++i) {
        population[i] = new int[individual_lengths[i]];
        int index = 0;
        for (int j = 0; j < num_vars; ++j) {
            for (int k = 0; k < bits_per_var[j]; ++k) {
                population[i][index++] = dis(gen);
            }
        }
    }

    return population;
}

// Преобразование Грей-кода в двоичный код
void gray_to_binary(const int* gray_code, int length, int* binary_code) {
    if (length > 0) {
        binary_code[0] = gray_code[0];
        for (int i = 1; i < length; ++i) {
            binary_code[i] = binary_code[i - 1] ^ gray_code[i];
        }
    }
}

// Функция для перевода бинарного кода в вещественные значения
double* binary_to_real(const int* binary_str, int total_length, const double* left, const double* right, const int* bits_per_var, int num_vars, bool gray_code) {
    double* real_values = new double[num_vars];
    int start = 0;

    for (int i = 0; i < num_vars; ++i) {
        int end = start + bits_per_var[i];
        int int_val = 0;

        // Если используется Грей-код, преобразуем его в двоичный
        int* var_bits = new int[bits_per_var[i]];
        memcpy(var_bits, binary_str + start, bits_per_var[i] * sizeof(int));
        if (gray_code) {
            int* binary_code = new int[bits_per_var[i]];
            gray_to_binary(var_bits, bits_per_var[i], binary_code);
            for (int j = 0; j < bits_per_var[i]; ++j) {
                int_val = (int_val << 1) | binary_code[j];
            }
            delete[] binary_code;
        } else {
            for (int j = 0; j < bits_per_var[i]; ++j) {
                int_val = (int_val << 1) | var_bits[j];
            }
        }

        // Расчет вещественного значения
        double step = (right[i] - left[i]) / (pow(2, bits_per_var[i]) - 1);
        real_values[i] = left[i] + int_val * step;

        start = end;
        delete[] var_bits;
    }

    return real_values;
}

// Тестовая функция: сумма квадратов
double sphere_function(const double* x, int num_vars) {
    double result = 0.0;
    for (int i = 0; i < num_vars; ++i) {
        result += x[i] * x[i];
    }
    return result;
}

// Главная функция программы
int main() {
    // Установка кодировки для русского языка
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // Ввод параметров от пользователя
    cout << "Введите количество индивидов в популяции (N): ";
    int pop_size;
    cin >> pop_size;

    cout << "Введите количество переменных (n): ";
    int num_vars;
    cin >> num_vars;

    double left[num_vars], right[num_vars];
    int bits_per_var[num_vars];

    for (int i = 0; i < num_vars; ++i) {
        cout << "Введите левую границу для переменной " << i + 1 << ": ";
        cin >> left[i];
        cout << "Введите правую границу для переменной " << i + 1 << ": ";
        cin >> right[i];
        cout << "Введите количество битов для переменной " << i + 1 << ": ";
        cin >> bits_per_var[i];
    }

    bool gray_code;
    cout << "Использовать Грей-код? (1 - да, 0 - нет): ";
    cin >> gray_code;

    // Вычисляем длины бинарных строк для каждого индивида
    int* individual_lengths = new int[pop_size];
    int** population = initialize_population(pop_size, num_vars, bits_per_var, individual_lengths);

    // Открываем файл для записи результатов
    ofstream file("ga_output.txt");
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл для записи!" << endl;
        return 1;
    }

    file << "Индивиды популяции:\n";
    file << "-------------------------------------------------\n";

    // Обработка каждого индивида
    for (int i = 0; i < pop_size; ++i) {
        double* real_vals = binary_to_real(population[i], individual_lengths[i], left, right, bits_per_var, num_vars, gray_code);
        double fitness = sphere_function(real_vals, num_vars);

        // Выводим результаты
        file << "Индивид " << i + 1 << ":\n";
        file << "  Бинарный код: ";
        for (int j = 0; j < individual_lengths[i]; ++j) {
            file << population[i][j];
        }
        file << "\n  Вещественные значения: ";
        for (int j = 0; j < num_vars; ++j) {
            file << fixed << setprecision(4) << real_vals[j] << " ";
        }
        file << "\n  Значение функции: " << fixed << setprecision(4) << fitness << "\n";
        file << "-------------------------------------------------\n";

        // Освобождаем память
        delete[] real_vals;
    }

    // Освобождаем память
    for (int i = 0; i < pop_size; ++i) {
        delete[] population[i];
    }
    delete[] population;
    delete[] individual_lengths;

    file.close();
    cout << "Результаты сохранены в файл ga_output.txt" << endl;
    return 0;
}