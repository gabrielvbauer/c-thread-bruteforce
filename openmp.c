#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define TARGET_PASSWORD "12345678"
#define TARGET_PASSWORD_LENGTH 8

const char dictionary[] = "0123456789";
const int dictionarySize = sizeof(dictionary) -1;
long long totalCombinations = 1LL << (TARGET_PASSWORD_LENGTH * 4);

void showProgress(long long testedCombinations, double startTime, bool found) {
  double elapsedTime = omp_get_wtime() - startTime;
  double progress = (double)testedCombinations / totalCombinations * 100.0;
  long long remaining = totalCombinations - testedCombinations;
  double estimatedTime = (remaining * elapsedTime) / testedCombinations;

  printf("\n");
  printf("Progresso: %.2f%%\n", progress);
  printf("Possibilidades testadas: %lld / %lld\n", testedCombinations, totalCombinations);
  if (!found) {
    printf("Tempo decorrido: %.2f segundos\n", elapsedTime);
    printf("Tempo estimado para verificar todas as possibilidades: %.2f segundos\n", estimatedTime);
  }
  printf("\n");
}

void breakPassword() {
  bool found = false;
  char guessedPassword[] = "00000000";
  long long testedCombinations = 0;
  double startTime = omp_get_wtime();
  double lastUpdateTime = startTime;

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
        found = true;
        printf("Password found: %s\n", guessedPassword);
      }
    }
    
    double currentTime = omp_get_wtime();
    if (currentTime - lastUpdateTime >= 1.0 && omp_get_thread_num() == 0) {
      #pragma omp critical
      {
        showProgress(testedCombinations, startTime, false);
        lastUpdateTime = currentTime;
      }
    }
  }

  showProgress(testedCombinations, startTime, true);
}

int main() {
  omp_set_num_threads(12);
  breakPassword();
}
