#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h> // Thêm thu vi?n errno d? s? d?ng errno

#define MAXSIZE     27
#define MAX_NAME    50
#define MAX_PHONE   12
#define MAX_RESULT  100

// Semaphore operations
struct sembuf p = {0, -1, 0};  // wait operation
struct sembuf v = {0, 1, 0};   // signal operation

int main() {
    int shmid1, shmid2, shmid3, shmid4, shmid5, semid, option;
    key_t key1, key2, key3, key4, key5, semkey;
    char *type, *shared_name, *shared_phone, *shared_response;
    int *buy;
    int price;

    key1 = 5678;
    key2 = 8765;
    key3 = 4321;
    key4 = 1234;
    key5 = 2468; // New key for response (dã s?a l?i trùng l?p)
    semkey = 1111; // Key for semaphore
    printf("Attempting to allocate shared memory with key: %d, size: %zu\n", 
       key5, (size_t)(MAX_RESULT * sizeof(char)));

    // Attach shared memory
    if ((shmid1 = shmget(key1, MAXSIZE, IPC_CREAT | 0666)) == -1) {
        perror("shmget key1");
        exit(1);
    }
    if ((shmid2 = shmget(key2, sizeof(int), IPC_CREAT | 0666)) == -1) {  // Use sizeof(int) for an integer shared memory
        perror("shmget key2");
        exit(1);
    }
    if ((shmid3 = shmget(key3, MAX_NAME, IPC_CREAT | 0666)) == -1) {
        perror("shmget key3");
        exit(1);
    }
    if ((shmid4 = shmget(key4, MAX_PHONE, IPC_CREAT | 0666)) == -1) {
        perror("shmget key4");
        exit(1);
    }

    if ((shmid5 = shmget(key5, MAX_RESULT * sizeof(char), IPC_CREAT | 0666)) == -1) {
        printf("shmget failed. errno: %d\n", errno); // S? d?ng errno
        perror("shmget error");
        
        // Th? l?i v?i quy?n truy c?p khác
        if ((shmid5 = shmget(key5, MAX_RESULT * sizeof(char), IPC_CREAT | 0644)) == -1) {
            printf("shmget failed again with different permissions. errno: %d\n", errno);
            perror("shmget error");
            exit(1);
        }
    }

    semid = semget(semkey, 1, 0666 | IPC_CREAT); // Create or get semaphore

    // Attach to shared memory and check for errors
    if ((type = shmat(shmid1, NULL, 0)) == (char *) -1) {
        perror("shmat key1");
        exit(1);
    }
    if ((buy = shmat(shmid2, NULL, 0)) == (int *) -1) {
        perror("shmat key2");
        exit(1);
    }
    if ((shared_name = shmat(shmid3, NULL, 0)) == (char *) -1) {
        perror("shmat key3");
        exit(1);
    }
    if ((shared_phone = shmat(shmid4, NULL, 0)) == (char *) -1) {
        perror("shmat key4");
        exit(1);
    }
    if ((shared_response = shmat(shmid5, NULL, 0)) == (char *) -1) {
        perror("shmat key5");
        exit(1);
    }

    printf("Welcome to Nguyen Smartphone\n");
    printf("          Server\n");
    printf("==============================\n");

    // Wait for client to enter name
    semop(semid, &p, 1); // Wait (P operation)
    printf("Client Name: %s\n", shared_name);

    // Wait for client to enter phone number
    semop(semid, &p, 1); // Wait (P operation)
    printf("Client Phone Number: %s\n", shared_phone);

    // Wait for client to enter phone type
    semop(semid, &p, 1); // Wait (P operation)
    printf("Phone Type: %s\n", type);

    // Wait for client to enter quantity
    semop(semid, &p, 1); // Wait (P operation)
    printf("Quantity: %d\n", *buy);

    snprintf(shared_response, MAX_RESULT, "Waiting for server calculation...");
    semop(semid, &v, 1); // Notify client that the server is processing

    // Server options and chat loop
    while (1) {
        printf("\nOptions:\n");
        printf("1. Calculate and display the bill\n");
        printf("2. Chat with the client\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        // Clear input buffer to avoid issues with fgets
        getchar();  // This clears the newline character left by scanf

        if (option == 1) {
            srand(time(NULL));
            price = rand() % (7000000 + 1 - 5000000) + 5000000;
            printf("Per unit price: %d\n", price);
            printf("Total price: %d\n", *buy * price);

            snprintf(shared_response, MAX_RESULT, "Your total price is: %d", *buy * price);
        } else if (option == 2) {
            printf("Enter your message to the client: ");
            fgets(shared_response, MAX_RESULT, stdin); // Ensure input does not exceed MAX_RESULT
            shared_response[strcspn(shared_response, "\n")] = '\0'; // Remove newline character if present
        }

        // Signal client that response is ready
        semop(semid, &v, 1); // Signal (V operation)

        // Wait for client message
        semop(semid, &p, 1); // Wait (P operation)
        printf("Client message: %s\n", shared_response);

        if (strcmp(shared_response, "exit") == 0) {
            break; // Exit if client sends exit
        }
    }

    exit(0);
}
