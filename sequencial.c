#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define TARGET_PASSWORD "123456789"
#define TARGET_PASSWORD_LENGTH 9

const char dictionary[] = "0123456789";
const int dictionarySize = sizeof(dictionary) -1;
long long totalCombinations = 1LL << (TARGET_PASSWORD_LENGTH * 4);

void showProgress(long long testedCombinations, double startTime, bool found) {
  double elapsedTime = time(NULL) - startTime;
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
  double startTime = time(NULL);
  double lastUpdateTime = startTime;

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
      found = true;
      printf("Password found: %s\n", guessedPassword);
    }
    
    double currentTime = time(NULL);
    if (currentTime - lastUpdateTime >= 1.0) {
      showProgress(testedCombinations, startTime, false);
      lastUpdateTime = currentTime;
    }
  }

  showProgress(testedCombinations, startTime, true);
}

int main() {
  breakPassword();
}
