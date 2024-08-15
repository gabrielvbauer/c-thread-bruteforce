#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define TARGET_PASSWORD "88771797"
#define TARGET_PASSWORD_LENGTH 8

const char dictionary[] = "0123456789";
const int dictionarySize = sizeof(dictionary) -1;
long long totalCombinations = 1LL << (TARGET_PASSWORD_LENGTH * 4);

double startTime = 0.0;
long long testedCombinations = 0;

FILE *threadFile;
FILE *sequencialFile;

void showProgress(long long testedCombinations, double startTime, double endTime, int iterator) {
  double elapsedTime = endTime - startTime;
  double progress = (double)testedCombinations / totalCombinations * 100.0;
  long long remaining = totalCombinations - testedCombinations;
  double estimatedTime = (remaining * elapsedTime) / testedCombinations;

  printf("Iteração: %d\n", iterator);
  printf("Progresso: %.2f%%\n", progress);
  printf("Tempo decorrido: %.2f segundos\n", elapsedTime);
  printf("Possibilidades testadas: %lld / %lld\n\n", testedCombinations, totalCombinations);
  printf("============\n\n");
}

void breakPasswordThread(int iterator) {
  bool found = false;
  char guessedPassword[] = "00000000";
  double lastUpdateTime = startTime;
  startTime = omp_get_wtime();

  #pragma omp parallel for private(guessedPassword) shared(found, testedCombinations)
  for(long long i = 0; i < totalCombinations; i++) {
    if (found) continue;

    long long index = i;
    for(int j = 0; j < TARGET_PASSWORD_LENGTH; j++) {
      guessedPassword[j] = dictionary[index % dictionarySize];
      index /= dictionarySize;
    }

    guessedPassword[TARGET_PASSWORD_LENGTH] = '\0';

    #pragma omp atomic
    testedCombinations++;

    int passwordMatch = strcmp(guessedPassword, TARGET_PASSWORD);
    if (passwordMatch == 0) {
      #pragma omp critical
      {
        double endTime = omp_get_wtime();
        found = true;
        printf("Password found using threads: %s\n\n", guessedPassword);
        showProgress(testedCombinations, startTime, endTime, iterator);
        fprintf(threadFile, "\n");
        fprintf(threadFile, "Tempo: %.2f segundos\n", endTime - startTime);
        fprintf(threadFile, "Encontrado em: %lld de %lld\n", testedCombinations, totalCombinations);
        fprintf(threadFile, "Porcentagem de tentativas: %.2f%% \n", (double)testedCombinations / totalCombinations * 100.0);
        fprintf(threadFile, "\n");
        testedCombinations = 0;
      }
    }
  }
}

void breakPasswordSequence(int iterator) {
  bool found = false;
  char guessedPassword[] = "00000000";
  double lastUpdateTime = startTime;
  startTime = omp_get_wtime();

  for(long long i = 0; i < totalCombinations; i++) {
    if (found) continue;

    long long index = i;
    for(int j = 0; j < TARGET_PASSWORD_LENGTH; j++) {
      guessedPassword[j] = dictionary[index % dictionarySize];
      index /= dictionarySize;
    }

    guessedPassword[TARGET_PASSWORD_LENGTH] = '\0';

    testedCombinations++;

    int passwordMatch = strcmp(guessedPassword, TARGET_PASSWORD);
    if (passwordMatch == 0) {
      double endTime = omp_get_wtime();
      found = true;
      printf("Password found by sequence: %s\n\n", guessedPassword);
      showProgress(testedCombinations, startTime, endTime, iterator);
      fprintf(sequencialFile, "\n");
      fprintf(sequencialFile, "Tempo: %.2f segundos\n", endTime - startTime);
      fprintf(threadFile, "Encontrado em: %lld de %lld\n", testedCombinations, totalCombinations);
      fprintf(sequencialFile, "Porcentagem de tentativas: %.2f%% \n", (double)testedCombinations / totalCombinations * 100.0);
      fprintf(sequencialFile, "\n");
      testedCombinations = 0;
    }
  }
}

int main() {
  threadFile = fopen("threads.txt", "a");
  sequencialFile = fopen("sequencial.txt", "a");

  omp_set_num_threads(12);
  for(int i = 1; i<= 20; i++) {
    breakPasswordThread(i);
  }

  for(int i = 1; i<= 20; i++) {
    breakPasswordSequence(i);
  }
}
