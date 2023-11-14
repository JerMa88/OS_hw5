#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Global variables
sem_t *ta, *student;
pthread_mutex_t mutex;
int waiting = 0;
int chairs = 3;

// Function prototypes
void *ta_function(void *param);
void *student_function(void *param);
void random_sleep(int id);

// Custom semaphore wait operation
void sem_wait_custom(sem_t *sem) {
    sem_wait(sem);
}

// Custom semaphore signal operation
void sem_signal_custom(sem_t *sem) {
    sem_post(sem);
}

// TA thread function
void *ta_function(void *param) {
    while (1) {
        sem_wait_custom(student); // Wait for a student
        pthread_mutex_lock(&mutex);
        if (waiting > 0) {
            waiting--;
        }
        printf("TA is helping a student. Waiting students: %d\n", waiting);
        sem_signal_custom(ta);
        pthread_mutex_unlock(&mutex);
        random_sleep(0); // TA helping a student
    }
}

// Student thread function
void *student_function(void *param) {
    int id = *((int *)param);
    while (1) {
        random_sleep(id); // Student is programming
        pthread_mutex_lock(&mutex);
        if (waiting < chairs) {
            waiting++;
            printf("Student %d is waiting. Remaining chairs: %d\n", id, (3-waiting));
            sem_signal_custom(student);
            pthread_mutex_unlock(&mutex);
            sem_wait_custom(ta); // Wait for TA
            printf("Student %d is getting help.\n", id);
        } else {
            pthread_mutex_unlock(&mutex);
            printf("Student %d will try later.\n", id);
        }
    }
}

// Function to simulate random sleep
void random_sleep(int id) {
    int time = rand() % 5 + 1;
    printf("Student %d : TA sleeping for %d seconds.\n", id, time);
    sleep(time);
}

int main() {
    // Initialize semaphores and mutex
    ta = sem_open("/ta_sem", O_CREAT, 0644, 0);
    student = sem_open("/student_sem", O_CREAT, 0644, 0);
    pthread_mutex_init(&mutex, NULL);

    // Create TA thread
    pthread_t ta_thread;
    pthread_create(&ta_thread, NULL, ta_function, NULL);

    // Create student threads
    int num_students = 5;
    pthread_t students[num_students];
    int student_ids[num_students];
    for (int i = 0; i < num_students; i++) {
        student_ids[i] = i;
        pthread_create(&students[i], NULL, student_function, (void *)&student_ids[i]);
    }

    // Join threads (this program will run indefinitely)
    pthread_join(ta_thread, NULL);
    for (int i = 0; i < num_students; i++) {
        pthread_join(students[i], NULL);
    }

    // Clean up
    sem_close(ta);
    sem_close(student);
    sem_unlink("/ta_sem");
    sem_unlink("/student_sem");
    pthread_mutex_destroy(&mutex);

    return 0;
}
