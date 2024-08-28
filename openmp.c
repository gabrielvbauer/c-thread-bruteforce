#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define TARGET_PASSWORD "999999999"
#define TARGET_PASSWORD_LENGTH 9

const char dictionary[] = "0123456789";
// const char dictionary[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int dictionarySize = sizeof(dictionary) -1;
long long totalCombinations = 0;

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
  printf("Tempo decorrido: %.10f segundos\n", elapsedTime);
  printf("Possibilidades testadas: %lld / %lld\n\n", testedCombinations, totalCombinations);
  printf("============\n\n");
}

void debug(int iterator) {
  showProgress(testedCombinations, startTime, omp_get_wtime(), iterator);
}

void breakPasswordThread(int iterator) {
  bool found = false;
  char guessedPassword[] = "00000000";
  double lastUpdateTime = startTime;
  startTime = omp_get_wtime();

  #pragma omp parallel private(guessedPassword) shared(found, testedCombinations)
  {
    #pragma omp for schedule(static)
    for(long long i = 0; i < totalCombinations; i++) {
    if (found) continue;

    long long index = i;
    for(int j = 0; j < TARGET_PASSWORD_LENGTH; j++) {
      guessedPassword[j] = dictionary[index % dictionarySize];
      index /= dictionarySize;
    }

    guessedPassword[TARGET_PASSWORD_LENGTH] = '\0';

    /*
      DEBUG MODE
    */
    // double currentTime = omp_get_wtime();
    // if (currentTime - lastUpdateTime >= 1.0 && omp_get_thread_num() == 0) {
    //   #pragma omp critical
    //   {
    //     debug(iterator);
    //   }
    //   lastUpdateTime = currentTime;
    // }

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
        fprintf(threadFile, "Tempo: %.10f segundos\n", endTime - startTime);
        fprintf(threadFile, "Encontrado em: %lld de %lld\n", testedCombinations, totalCombinations);
        fprintf(threadFile, "Porcentagem de tentativas: %.2f%% \n", (double)testedCombinations / totalCombinations * 100.0);
        fprintf(threadFile, "\n");
        testedCombinations = 0;
      }
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

    /*
      DEBUG MODE
    */
    double currentTime = omp_get_wtime();
    if (currentTime - lastUpdateTime >= 1.0 && omp_get_thread_num() == 0) {
      debug(iterator);
      lastUpdateTime = currentTime;
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
      fprintf(sequencialFile, "Tempo: %.10f segundos\n", endTime - startTime);
      fprintf(sequencialFile, "Encontrado em: %lld de %lld\n", testedCombinations, totalCombinations);
      fprintf(sequencialFile, "Porcentagem de tentativas: %.2f%% \n", (double)testedCombinations / totalCombinations * 100.0);
      fprintf(sequencialFile, "\n");
      testedCombinations = 0;
    }
  }
}

int main() {
  threadFile = fopen("threads.txt", "a");
  sequencialFile = fopen("sequencial.txt", "a");
  totalCombinations = pow(dictionarySize, TARGET_PASSWORD_LENGTH);

  printf("Senha alvo: %s\n", TARGET_PASSWORD);
  printf("Possibilidades: %lld\n\n", totalCombinations);

  omp_set_num_threads(12);
  for(int i = 1; i<= 20; i++) {
    breakPasswordThread(i);
  }

  for(int i = 1; i<= 20; i++) {
    breakPasswordSequence(i);
  }
}
