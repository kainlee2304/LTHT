#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXSIZE     27
#define MAX_NAME    50
#define MAX_PHONE   12 // 11 digits + null terminator
#define MAX_RESULT  100

// Semaphore operations
struct sembuf p = {0, -1, 0};  // wait operation
struct sembuf v = {0, 1, 0};   // signal operation

int is_valid_phone_number(char* phone) {
    int length = strlen(phone);
    if (length != 10 && length != 11) return 0; // Check for valid length
    for (int i = 0; i < length; i++) {
        if (!isdigit(phone[i])) return 0; // Ensure all characters are digits
    }
    return 1;
}

int main() {
    int shmid1, shmid2, shmid3, shmid4, shmid5, semid, option;
    key_t key1, key2, key3, key4, key5, semkey;
    char *type, *shared_name, *shared_phone, *shared_response;
    int *buy;
    char full_name[MAX_NAME];
    char phone_number[MAX_PHONE];

    key1 = 5678;
    key2 = 8765;
    key3 = 4321;
    key4 = 1234;
    key5 = 2468; // New key for response
    semkey = 1111; // Key for semaphore

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
        perror("shmget key5");
        exit(1);
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

    printf("Welcome to Nguyen Smartphone\n\n");
    printf("          Client\n");
    printf("==============================\n\n");

    // Input Full Name
    printf("Input your Full Name: ");
    fgets(full_name, MAX_NAME, stdin);
    full_name[strcspn(full_name, "\n")] = '\0'; // Remove newline character
    strcpy(shared_name, full_name); // Copy full name to shared memory

    // Signal server that name is ready
    semop(semid, &v, 1); 

    // Input Phone Number with validation
    do {
        printf("Input your Phone Number (10 or 11 digits): ");
        scanf("%s", phone_number);
        if (!is_valid_phone_number(phone_number)) {
            printf("Invalid phone number format. Please enter a valid 10 or 11 digit phone number.\n");
        }
    } while (!is_valid_phone_number(phone_number));
    strcpy(shared_phone, phone_number); // Copy phone number to shared memory

    // Signal server that phone number is ready
    semop(semid, &v, 1); 

    // Input Phone Type
    do {
        printf("Input your phone type [Nokia, Samsung, Apple] [\"exit\" to close this program] : ");
        scanf("%s", type);
        if (strcmp("exit", type) == 0) {
            break;
        }
    } while (strcmp("nokia", type) != 0 && strcmp("samsung", type) != 0 && strcmp("apple", type) != 0);

    // Signal server that phone type is ready
    semop(semid, &v, 1); 

    // Input Quantity of Phones to Buy
    do {
        printf("Input how many phones you want to buy [1..8] : ");
        scanf("%d", buy);
    } while (*buy < 1 || *buy > 8);

    // Signal server that quantity is ready
    semop(semid, &v, 1); 

    // Waiting for server response
    printf("\nWaiting for server response...\n");
    semop(semid, &p, 1); // Wait for server response
    printf("Server response: %s\n", shared_response);

    // Wait for server to complete calculation or chat
    semop(semid, &p, 1); // Wait for final response from server
    printf("Server response: %s\n", shared_response);

    // Chat option and continuous interaction
    while (1) {
        printf("\nOptions:\n");
        printf("1. Chat with server\n");
        printf("2. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &option);

        if (option == 1) {
            // Chat with server
            printf("Enter your message to server: ");
            getchar(); // Clear buffer
            fgets(shared_response, MAX_RESULT, stdin);
            shared_response[strcspn(shared_response, "\n")] = '\0'; // Remove newline character

            // Signal server that message is ready
            semop(semid, &v, 1);

            // Wait for server response
            printf("\nWaiting for server response...\n");
            semop(semid, &p, 1); // Wait for server's response
            printf("Server response: %s\n", shared_response);

            if (strcmp(shared_response, "exit") == 0) {
                break; // Exit if server sends exit
            }
        } else if (option == 2) {
            // Send exit message to server
            strcpy(shared_response, "exit");
            semop(semid, &v, 1);
            break;
        }
    }

    exit(0);
}

