#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

// Параметры алгоритма
const int POPULATION_SIZE = 20;
const int GENERATIONS = 100;
const int INDIVIDUAL_SIZE = 20;

// Типы селекции
enum SelectionType { PROPORTIONAL, RANK, TOURNAMENT };

// Типы скрещивания
enum CrossoverType { ONE_POINT, TWO_POINT, UNIFORM };

// Типы мутации
enum MutationType { WEAK, AVERAGE, STRONG };

// Структура индивида
struct Individual {
    int* genes;
    double fitness;

    Individual() {
        genes = new int[INDIVIDUAL_SIZE];
        fitness = 0;
    }

    // Конструктор копирования (глубокое копирование)
    Individual(const Individual& other) {
        genes = new int[INDIVIDUAL_SIZE];
        for (int i = 0; i < INDIVIDUAL_SIZE; ++i) {
            genes[i] = other.genes[i];
        }
        fitness = other.fitness;
    }

    // Оператор присваивания (глубокое копирование)
    Individual& operator=(const Individual& other) {
        if (this != &other) {
            delete[] genes;
            genes = new int[INDIVIDUAL_SIZE];
            for (int i = 0; i < INDIVIDUAL_SIZE; ++i) {
                genes[i] = other.genes[i];
            }
            fitness = other.fitness;
        }
        return *this;
    }

    ~Individual() {
        delete[] genes;
    }
};

// Функция для вычисления пригодности (количество единиц)
double calculateFitness(const Individual& ind) {
    double fitness = 0;
    for (int i = 0; i < INDIVIDUAL_SIZE; ++i) {
        fitness += ind.genes[i];
    }
    return fitness;
}

// Инициализация популяции
Individual* initializePopulation() {
    Individual* population = new Individual[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        for (int j = 0; j < INDIVIDUAL_SIZE; ++j) {
            population[i].genes[j] = rand() % 2;
        }
        population[i].fitness = calculateFitness(population[i]);
    }
    return population;
}

// Пропорциональная селекция
Individual proportionalSelection(const Individual* population) {
    double totalFitness = 0;
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        totalFitness += population[i].fitness;
    }

    double* probabilities = new double[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        probabilities[i] = population[i].fitness / totalFitness;
    }

    double randomValue = (double)rand() / RAND_MAX;
    double cumulativeProbability = 0;
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        cumulativeProbability += probabilities[i];
        if (randomValue <= cumulativeProbability) {
            delete[] probabilities;
            return population[i];
        }
    }

    delete[] probabilities;
    return population[POPULATION_SIZE - 1];
}

// Ранговая селекция
Individual rankSelection(const Individual* population) {
    Individual* sortedPopulation = new Individual[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        sortedPopulation[i] = population[i];
    }

    // Сортировка по пригодности
    for (int i = 0; i < POPULATION_SIZE - 1; ++i) {
        for (int j = i + 1; j < POPULATION_SIZE; ++j) {
            if (sortedPopulation[i].fitness > sortedPopulation[j].fitness) {
                // Ручной обмен значениями
                Individual temp = sortedPopulation[i];
                sortedPopulation[i] = sortedPopulation[j];
                sortedPopulation[j] = temp;
            }
        }
    }

    double* ranks = new double[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        ranks[i] = i + 1;
    }

    double totalRank = POPULATION_SIZE * (POPULATION_SIZE + 1) / 2;
    double randomValue = (double)rand() / RAND_MAX * totalRank;
    double cumulativeRank = 0;
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        cumulativeRank += ranks[i];
        if (randomValue <= cumulativeRank) {
            delete[] sortedPopulation;
            delete[] ranks;
            return sortedPopulation[i];
        }
    }

    delete[] sortedPopulation;
    delete[] ranks;
    return sortedPopulation[POPULATION_SIZE - 1];
}

// Турнирная селекция
Individual tournamentSelection(const Individual* population, int tournamentSize = 2) {
    Individual* tournament = new Individual[tournamentSize];
    for (int i = 0; i < tournamentSize; ++i) {
        int randomIndex = rand() % POPULATION_SIZE;
        tournament[i] = population[randomIndex];
    }

    Individual best = tournament[0];
    for (int i = 1; i < tournamentSize; ++i) {
        if (tournament[i].fitness > best.fitness) {
            best = tournament[i];
        }
    }

    delete[] tournament;
    return best;
}

// Одноточечное скрещивание
Individual onePointCrossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    int crossoverPoint = 1 + rand() % (INDIVIDUAL_SIZE - 2);

    for (int i = 0; i < crossoverPoint; ++i) {
        child.genes[i] = parent1.genes[i];
    }
    for (int i = crossoverPoint; i < INDIVIDUAL_SIZE; ++i) {
        child.genes[i] = parent2.genes[i];
    }

    child.fitness = calculateFitness(child);
    return child;
}

// Двухточечное скрещивание
Individual twoPointCrossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    int crossoverPoint1 = 1 + rand() % (INDIVIDUAL_SIZE - 2);
    int crossoverPoint2 = 1 + rand() % (INDIVIDUAL_SIZE - 2);

    if (crossoverPoint1 > crossoverPoint2) {
        // Ручной обмен значениями
        int temp = crossoverPoint1;
        crossoverPoint1 = crossoverPoint2;
        crossoverPoint2 = temp;
    }

    for (int i = 0; i < crossoverPoint1; ++i) {
        child.genes[i] = parent1.genes[i];
    }
    for (int i = crossoverPoint1; i < crossoverPoint2; ++i) {
        child.genes[i] = parent2.genes[i];
    }
    for (int i = crossoverPoint2; i < INDIVIDUAL_SIZE; ++i) {
        child.genes[i] = parent1.genes[i];
    }

    child.fitness = calculateFitness(child);
    return child;
}

// Равномерное скрещивание
Individual uniformCrossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    for (int i = 0; i < INDIVIDUAL_SIZE; ++i) {
        child.genes[i] = (rand() % 2) ? parent1.genes[i] : parent2.genes[i];
    }

    child.fitness = calculateFitness(child);
    return child;
}

// Мутация
void mutate(Individual& ind, MutationType mutationType) {
    double mutationProb = 0.0;
    switch (mutationType) {
        case WEAK: mutationProb = 1.0 / (3 * INDIVIDUAL_SIZE); break;
        case AVERAGE: mutationProb = 1.0 / INDIVIDUAL_SIZE; break;
        case STRONG: mutationProb = (3.0 / INDIVIDUAL_SIZE < 1.0) ? 3.0 / INDIVIDUAL_SIZE : 1.0; break;
    }

    for (int i = 0; i < INDIVIDUAL_SIZE; ++i) {
        if ((double)rand() / RAND_MAX < mutationProb) {
            ind.genes[i] = 1 - ind.genes[i]; // Инвертируем ген
        }
    }

    ind.fitness = calculateFitness(ind);
}

// Формирование нового поколения
Individual* createNewGeneration(const Individual* population, SelectionType selectionType, CrossoverType crossoverType, MutationType mutationType) {
    Individual* newPopulation = new Individual[POPULATION_SIZE];

    for (int i = 0; i < POPULATION_SIZE; ++i) {
        Individual parent1, parent2;

        switch (selectionType) {
            case PROPORTIONAL: parent1 = proportionalSelection(population); parent2 = proportionalSelection(population); break;
            case RANK: parent1 = rankSelection(population); parent2 = rankSelection(population); break;
            case TOURNAMENT: parent1 = tournamentSelection(population); parent2 = tournamentSelection(population); break;
        }

        Individual child;
        switch (crossoverType) {
            case ONE_POINT: child = onePointCrossover(parent1, parent2); break;
            case TWO_POINT: child = twoPointCrossover(parent1, parent2); break;
            case UNIFORM: child = uniformCrossover(parent1, parent2); break;
        }

        mutate(child, mutationType);
        newPopulation[i] = child;
    }

    return newPopulation;
}

// Зал славы
void updateHallOfFame(Individual* hallOfFame, const Individual* population, int& hallOfFameSize) {
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (hallOfFameSize < 10 || population[i].fitness > hallOfFame[hallOfFameSize - 1].fitness) {
            if (hallOfFameSize < 10) {
                hallOfFame[hallOfFameSize] = population[i];
                hallOfFameSize++;
            } else {
                hallOfFame[hallOfFameSize - 1] = population[i];
            }

            // Сортировка зала славы
            for (int j = hallOfFameSize - 1; j > 0; --j) {
                if (hallOfFame[j].fitness > hallOfFame[j - 1].fitness) {
                    // Ручной обмен значениями
                    Individual temp = hallOfFame[j];
                    hallOfFame[j] = hallOfFame[j - 1];
                    hallOfFame[j - 1] = temp;
                } else {
                    break;
                }
            }
        }
    }
}

// Вывод зала славы
void printHallOfFame(const Individual* hallOfFame, int hallOfFameSize) {
    cout << "\nHall of Fame:\n";
    for (int i = 0; i < hallOfFameSize; ++i) {
        cout << "Individual " << i + 1 << ": ";
        for (int j = 0; j < INDIVIDUAL_SIZE; ++j) {
            cout << hallOfFame[i].genes[j];
        }
        cout << " Fitness: " << hallOfFame[i].fitness << endl;
    }
}

// Основная функция
int main() {
    srand(time(nullptr));

    Individual* population = initializePopulation();
    Individual* hallOfFame = new Individual[10];
    int hallOfFameSize = 0;

    for (int generation = 0; generation < GENERATIONS; ++generation) {
        //SelectionType { PROPORTIONAL, RANK, TOURNAMENT };
        //CrossoverType { ONE_POINT, TWO_POINT, UNIFORM };
        //enum MutationType { WEAK, AVERAGE, STRONG };
        Individual* newPopulation = createNewGeneration(population, PROPORTIONAL, ONE_POINT, STRONG);
        delete[] population;
        population = newPopulation;

        updateHallOfFame(hallOfFame, population, hallOfFameSize);

        // Находим лучшего индивида в текущем поколении
        Individual bestInGeneration = population[0];
        for (int i = 1; i < POPULATION_SIZE; ++i) {
            if (population[i].fitness > bestInGeneration.fitness) {
                bestInGeneration = population[i];
            }
        }

        // Вывод информации о лучшем индивиде в текущем поколении
        cout << "Generation " << generation + 1 << ": Best Individual Fitness = " << bestInGeneration.fitness << "\n";

        // Проверка на наличие идеального индивида
        if (bestInGeneration.fitness == INDIVIDUAL_SIZE) {
            cout << "Ideal individual found in generation " << generation + 1 << "!\n";
            printHallOfFame(hallOfFame, hallOfFameSize);
            delete[] population;
            delete[] hallOfFame;
            return 0;
        }
    }

    cout << "Ideal individual not found after " << GENERATIONS << " generations.\n";
    printHallOfFame(hallOfFame, hallOfFameSize);

    delete[] population;
    delete[] hallOfFame;

    return 0;
}