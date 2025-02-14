#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <cstring>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <windows.h>
using namespace std;

// Функция для инициализации популяции
int** initialize_population(int pop_size, int num_vars, const int* bits_per_var, int* individual_lengths) {
    for (int i = 0; i < pop_size; ++i) {
        individual_lengths[i] = 0;
        for (int j = 0; j < num_vars; ++j) {
            individual_lengths[i] += bits_per_var[j];
        }
    }

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

        double step = (right[i] - left[i]) / (pow(2, bits_per_var[i]) - 1);
        real_values[i] = left[i] + int_val * step;

        start = end;
        delete[] var_bits;
    }

    return real_values;
}

// Целевая функция для максимизации
double sphere_function(const double* x, int num_vars) {
    double result = 0.0;
    for (int i = 0; i < num_vars; ++i) {
        result += x[i] * x[i];
    }
    return -result; // Минус для максимизации
}

// Турнирная селекция (максимизация)
int tournament_selection(const double* fitnesses, int pop_size, int tournament_size) {
    vector<int> candidates;
    for (int i = 0; i < tournament_size; ++i) {
        candidates.push_back(rand() % pop_size);
    }
    int best_idx = candidates[0];
    for (int idx : candidates) {
        if (fitnesses[idx] > fitnesses[best_idx]) { // Выбираем максимальный фитнес
            best_idx = idx;
        }
    }
    return best_idx;
}

// Одноточечный кроссинговер
void single_point_crossover(const int* parent1, const int* parent2, int length, int* child1, int* child2) {
    int crossover_point = rand() % length;
    memcpy(child1, parent1, crossover_point * sizeof(int));
    memcpy(child1 + crossover_point, parent2 + crossover_point, (length - crossover_point) * sizeof(int));

    memcpy(child2, parent2, crossover_point * sizeof(int));
    memcpy(child2 + crossover_point, parent1 + crossover_point, (length - crossover_point) * sizeof(int));
}

// Мутация
void mutate(int* individual, int length, double mutation_rate) {
    for (int i = 0; i < length; ++i) {
        if (((double)rand() / RAND_MAX) < mutation_rate) {
            individual[i] = 1 - individual[i]; // Инвертируем бит
        }
    }
}

// Элитизм (максимизация)
void elitism_replacement(int** population, double* fitnesses, int** new_population, double* new_fitnesses, int pop_size, int* individual_lengths) {
    int best_idx = 0;
    for (int i = 1; i < pop_size; ++i) {
        if (fitnesses[i] > fitnesses[best_idx]) { // Выбираем максимальный фитнес
            best_idx = i;
        }
    }

    int* best_individual = new int[individual_lengths[best_idx]];
    memcpy(best_individual, population[best_idx], individual_lengths[best_idx] * sizeof(int));

    for (int i = 0; i < pop_size; ++i) {
        delete[] population[i];
        population[i] = new int[individual_lengths[i]];
        memcpy(population[i], new_population[i], individual_lengths[i] * sizeof(int));
        fitnesses[i] = new_fitnesses[i];
    }

    memcpy(population[0], best_individual, individual_lengths[0] * sizeof(int));
    fitnesses[0] = fitnesses[best_idx];

    delete[] best_individual;
}

// Вывод информации о популяции
void print_population(int generation, int** population, double* fitnesses, int pop_size, int* individual_lengths, const double* left, const double* right, const int* bits_per_var, int num_vars, bool gray_code, ofstream& file) {
    file << "Поколение " << generation << ":\n";
    for (int i = 0; i < pop_size; ++i) {
        double* real_vals = binary_to_real(population[i], individual_lengths[i], left, right, bits_per_var, num_vars, gray_code);

        // Подсчет достоверности (количество единиц в бинарном виде)
        int reliability = 0;
        for (int j = 0; j < individual_lengths[i]; ++j) {
            if (population[i][j] == 1) {
                reliability++;
            }
        }

        file << "Индивид " << i + 1 << ":\n";
        file << "  Бинарный код: ";
        for (int j = 0; j < individual_lengths[i]; ++j) {
            file << population[i][j];
        }
        file << "\n  Вещественные значения: ";
        for (int j = 0; j < num_vars; ++j) {
            file << fixed << setprecision(4) << real_vals[j] << " ";
        }
        file << "\n  Фитнес: " << fixed << setprecision(4) << fitnesses[i];
        file << "\n  Достоверность: " << reliability << "\n";

        delete[] real_vals;
    }
    file << "-------------------------------------------------\n";
}

// Главная функция программы
int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

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

    int* individual_lengths = new int[pop_size];
    int** population = initialize_population(pop_size, num_vars, bits_per_var, individual_lengths);

    double* fitnesses = new double[pop_size];
    for (int i = 0; i < pop_size; ++i) {
        double* real_vals = binary_to_real(population[i], individual_lengths[i], left, right, bits_per_var, num_vars, gray_code);
        fitnesses[i] = sphere_function(real_vals, num_vars);
        delete[] real_vals;
    }

    int generations = 100;
    int tournament_size = 5;
    double mutation_rate = 0.01;

    ofstream file("ga_output.txt");
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл для записи!" << endl;
        return 1;
    }

    for (int generation = 1; generation <= generations; ++generation) {
        // Вывод текущей популяции
        print_population(generation, population, fitnesses, pop_size, individual_lengths, left, right, bits_per_var, num_vars, gray_code, file);

        int** new_population = new int*[pop_size];
        double* new_fitnesses = new double[pop_size];

        for (int i = 0; i < pop_size; i += 2) {
            // Селекция
            int parent1_idx = tournament_selection(fitnesses, pop_size, tournament_size);
            int parent2_idx = tournament_selection(fitnesses, pop_size, tournament_size);

            // Логирование родителей
            file << "Скрещивание индивидов " << parent1_idx + 1 << " и " << parent2_idx + 1 << ":\n";

            // Кроссинговер
            int* child1 = new int[individual_lengths[parent1_idx]];
            int* child2 = new int[individual_lengths[parent2_idx]];
            single_point_crossover(population[parent1_idx], population[parent2_idx], individual_lengths[parent1_idx], child1, child2);

            // Логирование потомков
            file << "  Результат скрещивания:\n";
            file << "    Потомок 1: ";
            for (int j = 0; j < individual_lengths[parent1_idx]; ++j) {
                file << child1[j];
            }
            file << "\n    Потомок 2: ";
            for (int j = 0; j < individual_lengths[parent2_idx]; ++j) {
                file << child2[j];
            }
            file << "\n";

            // Мутация
            mutate(child1, individual_lengths[parent1_idx], mutation_rate);
            mutate(child2, individual_lengths[parent2_idx], mutation_rate);

            // Вычисление фитнеса для потомков
            double* real_vals1 = binary_to_real(child1, individual_lengths[parent1_idx], left, right, bits_per_var, num_vars, gray_code);
            double* real_vals2 = binary_to_real(child2, individual_lengths[parent2_idx], left, right, bits_per_var, num_vars, gray_code);
            new_fitnesses[i] = sphere_function(real_vals1, num_vars);
            new_fitnesses[i + 1] = sphere_function(real_vals2, num_vars);

            new_population[i] = child1;
            new_population[i + 1] = child2;

            delete[] real_vals1;
            delete[] real_vals2;
        }

        // Замещение с использованием элитизма
        elitism_replacement(population, fitnesses, new_population, new_fitnesses, pop_size, individual_lengths);

        // Освобождаем память
        for (int i = 0; i < pop_size; ++i) {
            delete[] new_population[i];
        }
        delete[] new_population;
        delete[] new_fitnesses;

        cout << "Поколение " << generation << ": Лучший фитнес = " << fitnesses[0] << endl;
    }

    file.close();

    for (int i = 0; i < pop_size; ++i) {
        delete[] population[i];
    }
    delete[] population;
    delete[] individual_lengths;
    delete[] fitnesses;

    cout << "Результаты сохранены в файл ga_output.txt" << endl;
    return 0;
}