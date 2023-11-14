#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];
pthread_cond_t conditions[NUM_PHILOSOPHERS];
bool chopstick_in_use[NUM_PHILOSOPHERS] = {false};


void* philosopher(void* num);
void pickup_forks(int philosopher_number);
void return_forks(int philosopher_number);
void think_or_eat(int philosopher_number, const char* action);

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int philosopher_numbers[NUM_PHILOSOPHERS];

    // Initialize mutexes and condition variables
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
        pthread_cond_init(&conditions[i], NULL);
        philosopher_numbers[i] = i;
    }

    // Create philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_create(&philosophers[i], NULL, philosopher, &philosopher_numbers[i]);
    }

    // Join philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Clean up
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
        pthread_cond_destroy(&conditions[i]);
    }

    return 0;
}

void* philosopher(void* num) {
    int philosopher_number = *(int*)num;

    while (1) {
        think_or_eat(philosopher_number, "thinks");
        pickup_forks(philosopher_number);
        think_or_eat(philosopher_number, "eats");
        return_forks(philosopher_number);
    }
}
void pickup_forks(int philosopher_number) {
    int left = philosopher_number;
    int right = (philosopher_number + 1) % NUM_PHILOSOPHERS;

    if (philosopher_number == 0) {
        int temp = left;
        left = right;
        right = temp;
    }

    pthread_mutex_lock(&chopsticks[left]);
    printf("Philosopher %d tries to pick left chopstick\n", philosopher_number);
    while (chopstick_in_use[left]) {
        pthread_cond_wait(&conditions[left], &chopsticks[left]);
    }
    chopstick_in_use[left] = true;
    printf("Philosopher %d picks the left chopstick\n", philosopher_number);

    pthread_mutex_lock(&chopsticks[right]);
    printf("Philosopher %d tries to pick right chopstick\n", philosopher_number);
    while (chopstick_in_use[right]) {
        pthread_cond_wait(&conditions[right], &chopsticks[right]);
    }
    chopstick_in_use[right] = true;
    printf("Philosopher %d picks the right chopstick\n", philosopher_number);

    printf("Philosopher %d begins to eat\n", philosopher_number);
}

void return_forks(int philosopher_number) {
    int left = philosopher_number;
    int right = (philosopher_number + 1) % NUM_PHILOSOPHERS;

    if (philosopher_number == 0) {
        int temp = left;
        left = right;
        right = temp;
    }

    chopstick_in_use[right] = false;
    pthread_mutex_unlock(&chopsticks[right]);
    pthread_cond_signal(&conditions[right]);

    chopstick_in_use[left] = false;
    pthread_mutex_unlock(&chopsticks[left]);
    pthread_cond_signal(&conditions[left]);

    printf("Philosopher %d returns chopsticks\n", philosopher_number);
}

void think_or_eat(int philosopher_number, const char* action) {
    printf("Philosopher %d %s\n", philosopher_number, action);
    sleep(rand() % 3 + 1);
}
