#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>

#define PORT 12345

#define MAX_SIZE 100

#define MAX_HEAP_SIZE 100
#define HEAP_SIZE 30720

typedef struct
{
    char *memory;
    int size;
} Node;

typedef struct
{
    Node heap[MAX_HEAP_SIZE];
    int count;
} MinHeap;

void initializeHeap(MinHeap *h)
{
    h->count = 0;
}
void heapifyUp(MinHeap *h, int index)
{
    int parent = (index - 1) / 2;
    if (parent >= 0 && h->heap[index].size < h->heap[parent].size)
    {
        Node temp = h->heap[index];
        h->heap[index] = h->heap[parent];
        h->heap[parent] = temp;

        heapifyUp(h, parent);
    }
}
void heapifyDown(MinHeap *h, int index)
{
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < h->count && h->heap[left].size < h->heap[smallest].size)
    {
        smallest = left;
    }
    if (right < h->count && h->heap[right].size < h->heap[smallest].size)
    {
        smallest = right;
    }

    if (smallest != index)
    {
        Node temp = h->heap[index];
        h->heap[index] = h->heap[smallest];
        h->heap[smallest] = temp;

        heapifyDown(h, smallest);
    }
}
void insertHeap(MinHeap *h, Node value)
{
    if (h->count >= MAX_HEAP_SIZE)
    {
        printf("Heap overflow\n");
        return;
    }

    h->heap[h->count] = value;
    h->count++;
    heapifyUp(h, h->count - 1);
}

Node extractMin(MinHeap *h)
{
    if (h->count <= 0)
    {
        printf("Heap underflow\n");
        Node empty = {NULL, -1};
        return empty;
    }

    Node minValue = h->heap[0];
    h->heap[0] = h->heap[h->count - 1];
    h->count--;

    heapifyDown(h, 0);
    return minValue;
}
Node getMin(MinHeap *h)
{
    if (h->count <= 0)
    {
        printf("Heap underflow\n");
        Node empty = {NULL, -1};
        return empty;
    }

    Node minValue = h->heap[0];
    return minValue;
}

typedef struct Heap
{
    char arr[HEAP_SIZE];
    int bytes;
    int id;
} Heap;

void Heap_init(Heap *heap, MinHeap *h)
{
    memset(heap->arr, -1, HEAP_SIZE);
    heap->bytes = 0;
    heap->id = 0;
    Node value = {heap->arr, HEAP_SIZE};
    insertHeap(h, value);
}

int Heap_get_free_mem(Heap *heap)
{
    return HEAP_SIZE - heap->bytes;
}

void *Heap_ITUN(Heap *heap, int size, MinHeap *h)
{
    if (heap->bytes + size + 1 > HEAP_SIZE)
    {
        printf("Heap overflow\n");
        return NULL;
    }

    int free_mem = Heap_get_free_mem(heap);
    if (free_mem < size)
    {
        printf("Not enough free memory\n");
        return NULL;
    }

    char *memory = NULL;
    int min_size = 0;
    while (true)
    {

        min_size = getMin(h).size;

        if (min_size > size)
        {
            Node node = extractMin(h);
            memory = node.memory;
            int new_size = node.size - size;
            Node n1 = {memory, size};
            char *new_memory = memory + size;
            Node n2 = {new_memory, new_size};
            insertHeap(h, n1);
            insertHeap(h, n2);
        }
        else if (min_size == size)
        {
            Node node = extractMin(h);
            void *ptr = node.memory + 1;
            memory = (char *)ptr;
            heap->arr[heap->bytes] = (char)size;
            for (int j = 1; j <= size; j++)
            {
                heap->arr[heap->id + j] = 0;
            }
            heap->id += size + 1;
            heap->bytes = heap->id;
            break;
        }
        else
        {
            Node combined = {NULL, 0};
            int combined_size = 0;
            char *allocation_start = NULL;

            while (min_size < size && h->count > 0)
            {
                Node node = extractMin(h);

                if (combined.memory == NULL)
                {
                    combined.memory = node.memory;
                    allocation_start = combined.memory;
                }
                combined_size += node.size;
                min_size = combined_size;

                if (min_size >= size)
                {
                    break;
                }
            }

            if (combined_size >= size)
            {
                memory = allocation_start;
                int offset = heap->id;
                heap->arr[offset] = (char)size;

                int j = 1;
                int cpy_size = size;
                while (cpy_size > 0)
                {

                    heap->arr[offset + j] = 0;
                    j++;
                    cpy_size--;
                    if (cpy_size <= 0)
                        break;
                    while (heap->arr[offset + j] != -1)
                    {
                        int s = heap->arr[offset + j];
                        j += s;
                    }
                }

                int remaining_size = combined_size - size;

                if (remaining_size > 0)
                {
                    char *remaining_memory = allocation_start + size;
                    Node remaining_block = {remaining_memory, remaining_size};
                    insertHeap(h, remaining_block);
                }

                heap->bytes += size + 1;
                heap->id = offset + size + 1;
                break;
            }
            else
            {
                printf("Unable to allocate memory: insufficient non-contiguous space\n");
                return NULL;
            }
        }
    }
    return (void *)memory;
}

void Heap_ITUD(Heap *heap, void *ptr, MinHeap *h)
{
    char *memory = (char *)ptr;
    int i;
    for (i = 0; i < HEAP_SIZE; i++)
    {
        if (&heap->arr[i] == memory)
        {
            break;
        }
    }

    if (i == HEAP_SIZE)
    {
        printf("Illegal memory address.\n");
        return;
    }

    int size = heap->arr[i - 1];
    heap->bytes -= size + 1;

    if (size <= 0)
    {
        printf("Illegal memory size.\n");
        return;
    }
    else
    {
        i -= 1;
        while (i <= size)
        {
            heap->arr[i] = -1;
            i++;
        }
    }
    Node n = {memory, size};
    insertHeap(h, n);
}

void Heap_print(Heap *heap)
{
    for (int i = 1; i < HEAP_SIZE; i += 1)
    {
        if (heap->arr[i] == -1)
        {
            printf("\033[1;32m%c", -37); // Green block
        }
        else
        {
            printf("\033[1;31m%c", -37); // Red block
        }
    }
    printf("\033[0m\n"); // Reset color
}

int Heap_allocated_memory(Heap *heap)
{
    return heap->bytes - 1;
}

unsigned int myhash(const char *str)
{
    unsigned int hasha = 0;
    int c;
    while ((c = *str++))
    {
        hasha = c + (hasha << 6) + (hasha << 16) - hasha;
    }
    return hasha;
}

sem_t wait, W, R, RC, mutex, fcfs;
pthread_mutex_t queue;
int rc = 0;

static int user_flag[100];

typedef struct
{
    int *data;
    size_t size;
    size_t capacity;
} Vector;
void vector_init(Vector *vector)
{
    vector->size = 0;
    vector->capacity = 2;
    vector->data = (int *)malloc(vector->capacity * sizeof(int));
}
void vector_add(Vector *vector, int value)
{
    if (vector->size == vector->capacity)
    {
        vector->capacity *= 2;
        vector->data = realloc(vector->data, sizeof(int) * vector->capacity);
    }
    vector->data[vector->size++] = value;
}
void vector_free(Vector *vector)
{
    free(vector->data);
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}

Vector v;
Vector F;

typedef struct
{
    char f_name[50];
    int f_size;
    char date[50];
} User;

typedef struct
{
    User *data;
    size_t size;
    size_t capacity;

    char name[50];
    int r_size;

} UserList;

typedef struct
{
    UserList *data;
    size_t size;
    size_t capacity;

} Details;

typedef struct
{
    int clientsocket;
    Details *users;
} PTHREAD;

Details users;

void initUserList(Details *list)
{
    list->size = 0;
    list->capacity = 10;
    list->data = (UserList *)malloc(list->capacity * sizeof(UserList));
    for (int i = 0; i < list->capacity; i++)
    {
        list->data[i].size = 0;
        list->data[i].capacity = 10;
        list->data[i].data = (User *)malloc(list->data->capacity * sizeof(User));
    }
}

void addUser(Details *list, const char *name, int size)
{
    if (list->size == list->capacity)
    {
        list->capacity *= 2;
        list->data = (UserList *)realloc(list->data, list->capacity * sizeof(UserList));
    }

    strcpy(list->data[list->size].name, name);
    list->data[list->size].r_size = size;
    list->size++;
}

void checksamefile(Details *list, const char *f_name, int f_size, char date[], int uid)
{
    for (int i = 0; i < list->data[uid].size; i++)
    {
        if (strcmp(list->data[uid].data[i].f_name, f_name) == 0)
        {
            int old_size = list->data[uid].data[i].f_size;
            old_size -= f_size;
            list->data[uid].data[i].f_size = f_size;
            strcpy(list->data[uid].data[i].date, date);

            // remaining space after uploading file
            list->data[uid].r_size -= old_size;
            return;
        }
    }
}
void addFile(Details *list, const char *f_name, int f_size, char date[], int uid)
{

    checksamefile(list, f_name, f_size, date, uid);
    for (int i = 0; i < list->size; i++)
    {
        if (list->data[i].size == list->data[i].capacity)
        {
            list->data[i].capacity *= 2;
            list->data[i].data = (User *)realloc(list->data[i].data, list->data[i].capacity * sizeof(User));
        }
    }
    list->data[uid].data[list->data[uid].size].f_size = f_size;
    strcpy(list->data[uid].data[list->data[uid].size].date, date);
    strcpy(list->data[uid].data[list->data[uid].size].f_name, f_name);
    list->data[uid].size++;
    // remaining space after uploading file
    list->data[uid].r_size -= f_size;
}
bool user_exist(Details *list, const char *name, int *uid)
{
    for (size_t i = 0; i < list->size; i++)
    {
        if (strcmp(list->data[i].name, name) == 0)
        {

            *uid = i;
            return true;
        }
    }
    return false;
}

// List of valid image file extensions

const char *validExtensions[] = {".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tiff", ".webp", ".image"};
const int numExtensions = sizeof(validExtensions) / sizeof(validExtensions[0]);

// Function to check if a file has a valid image extension
bool hasImageExtension(const char *filename)
{
    size_t filename_len = strlen(filename);

    // Iterate over the list of valid extensions
    for (int i = 0; i < numExtensions; i++)
    {
        const char *extension = validExtensions[i];
        size_t ext_len = strlen(extension);

        // Ensure the filename is long enough to contain the extension
        if (filename_len >= ext_len)
        {
            // Compare the end of the filename with the current extension
            if (strcmp(filename + filename_len - ext_len, extension) == 0)
            {
                return true; // File has a valid image extension
            }
        }
    }
    return false; // File does not have a valid image extension
}

void saveToFile(const char *filename, Details *list)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        return;
    }
    const char *br = "----------";
    for (size_t i = 0; i < list->size; i++)
    {

        fprintf(file, "%s %zu\n", br, list->data[i].size);
        fprintf(file, "%s %i\n", list->data[i].name, list->data[i].r_size);
        for (size_t j = 0; j < list->data[i].size; j++)
        {
            fprintf(file, "%s %d %s\n", list->data[i].data[j].f_name, list->data[i].data[j].f_size, list->data[i].data[j].date);
        }
    }
    fclose(file);
}

void loadFromFile(const char *filename, Details *list)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return;
    }
    char br[11];
    char name[50];
    int r_size, f_size, l_size;
    char f_name[50];
    char date[50];
    while (fscanf(file, "%s %d", br, &l_size) != EOF)
    {
        fscanf(file, "%s %d", name, &r_size);
        addUser(list, name, r_size);
        int uid = list->size - 1;
        for (int j = 0; j < l_size; j++)
        {
            fscanf(file, "%s %d %s", f_name, &f_size, date);
            addFile(list, f_name, f_size, date, uid);
        }
    }

    fclose(file);
}

bool isFileEmpty(FILE *file)
{
    // Move the file pointer to the end of the file
    fseek(file, 0, SEEK_END);

    // Get the current position of the file pointer
    long fileSize = ftell(file);

    // Return to the start of the file
    fseek(file, 0, SEEK_SET);

    // If fileSize is 0, the file is empty
    if (fileSize == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void creatuser(const char *name, const char *user)
{
    FILE *file = fopen(name, "a");
    char arr[50];
    if (file)
    {
        fprintf(file, "%s\n", user);
    }
    fclose(file);
}

typedef struct
{
    char *command;
    int clientsocket;
    int uid;
    char *nameu;
    char *date;
    char *file_name;
    char *full_file_name;
    int t_size;
} UserCommand;
// Defining the Queue structure
typedef struct
{
    UserCommand items[MAX_SIZE];
    int front;
    int rear;
} Queue;

// Function to initialize the queue
void initializeQueue(Queue *q)
{
    q->front = 0;
    q->rear = 0;
}

// Function to check if the queue is empty
bool isEmpty(Queue *q)
{
    return (q->front == q->rear);
}

// Function to check if the queue is full
bool isFull(Queue *q)
{
    return (q->rear == MAX_SIZE);
}

// Function to add an element to the queue
void enqueue(Queue *q, UserCommand *value)
{
    pthread_mutex_lock(&queue);
    if (isFull(q))
    {
        printf("Queue is full\n");
        pthread_mutex_unlock(&queue);
        return;
    }
    q->items[q->rear] = *value;
    q->rear++;
    pthread_mutex_unlock(&queue);
}

// Function to remove an element from the queue (Dequeue
// operation)
void dequeue(Queue *q)
{
    pthread_mutex_lock(&queue);
    if (isEmpty(q))
    {
        printf("Queue is empty\n");
        pthread_mutex_unlock(&queue);

        return;
    }
    q->front++;
    pthread_mutex_unlock(&queue);
}

// Function to get the element at the front of the queue
UserCommand peek(Queue *q)
{
    // if (isEmpty(q))
    // {
    //     sem_wait(&mutex);
    // }
    return q->items[q->front];
}

// Function to print the current queue
void printQueue(Queue *q)
{
    if (isEmpty(q))
    {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int i = q->front + 1; i < q->rear; i++)
    {
        printf("%d ", q->items[i]);
    }
    printf("\n");
}
void userexist(const char *user, const char *nameu, char *passu, int clientsocket)
{
    char name[50];
    char pass[50];
    char str[100];
    memset(name, '\0', 50);
    memset(pass, '\0', 50);
    memset(str, '\0', 50);

    FILE *file = fopen("history.txt", "r");

    if (file)
    {
        if (!isFileEmpty(file))
        {
            bool indicate = false;
            bool f = false;
            int i = 0;
            while (fgets(str, 100, file) != NULL)
            {
                while (!f)
                {
                    name[i] = str[i];
                    i++;
                    if (str[i] == '~')
                    {
                        f = true;
                        i++;
                    }
                }
                int j = 0;
                while (f)
                {
                    pass[j] = str[i];
                    i++;
                    j++;
                    if (str[i] == '\n')
                    {
                        f = false;
                        i = 0;
                    }
                }

                if (strcmp(nameu, name) == 0)
                {
                    indicate = true;
                    if (strcmp(passu, pass) == 0)
                    {
                        send(clientsocket, "login successfully...", 22, 0);
                    }
                    else
                    {
                        char newpass[50] = "abc";

                        memset(newpass, '\0', 50);

                        while (strcmp(newpass, pass) != 0)
                        {

                            send(clientsocket, "wrong password", 15, 0);

                            recv(clientsocket, newpass, sizeof(newpass), 0);
                        }
                        send(clientsocket, "login successfully...", 22, 0);
                    }
                }
            }
            if (!indicate)
            {
                creatuser("history.txt", user);
                send(clientsocket, "new user created..", 19, 0);
            }
        }
        else
        {
            creatuser("history.txt", user);
            send(clientsocket, "new user created..", 19, 0);
        }

        fclose(file);
    }
    else
    {
        printf("file not found\n");
    }
}

Queue q;
bool isuseralreadyloggedin(Vector *V, char *details)
{
    int n = myhash(details);
    if (V->data[n % V->size] == 1)
        return true;
    V->data[n % V->size] = 1;
    return false;
}

typedef struct
{
    UserCommand C;
    Details *users;
} QueueArgc;

Heap heap;
MinHeap h;
void *reader(void *args)
{
    pthread_detach(pthread_self());

    QueueArgc *QC = (QueueArgc *)args;
    UserCommand C = QC->C;
    Details *users = QC->users;
    int clientsocket = C.clientsocket;
    char *c_buffer = C.command;
    char *nameu = C.nameu;
    char *date = C.date;
    int uid = C.uid;
    char *file_name = C.file_name;
    char *full_file_name;
    full_file_name = malloc(50);

    sprintf(full_file_name, "%s_%s", nameu, file_name);

    sem_post(&R);

    sem_wait(&RC);
    rc++;
    if (rc == 1)
    {
        sem_wait(&W);
    }
    sem_post(&RC);

    FILE *file = fopen(full_file_name, "rb");

    char ch[1024];

    if (file)
    {

        size_t n;
        printf("%s\n", "before sending");
        while ((n = fread(ch, 1, 1024, file)) > 0)
        {
            send(clientsocket, ch, n, 0);
        }

        char c = '$';
        send(clientsocket, &c, 1, 0);

        fclose(file);

        sem_wait(&RC);
        rc--;
        if (rc == 0)
        {
            sem_post(&W);
        }
        sem_post(&RC);

        printf("Client %s downloaded file %s\n", nameu, file_name);
    }
    else
    {
        const char *c = "$FAILURE$NO_CLIENT_DATA!";
        send(clientsocket, c, strlen(c), 0);
    }
}
void *writer(void *args)
{
    pthread_detach(pthread_self());

    QueueArgc *QC = (QueueArgc *)args;
    UserCommand C = QC->C;
    Details *users = QC->users;

    int clientsocket = C.clientsocket;
    char *c_buffer = C.command;
    char *nameu = C.nameu;
    char *date = C.date;
    int uid = C.uid;
    char *file_name = C.file_name;
    int t_size = C.t_size;
    char *full_file_name;
    full_file_name = malloc(50);

    sprintf(full_file_name, "%s_%s", nameu, file_name);

    sem_wait(&W); // wait

    addFile(users, file_name, t_size, date, uid);

    FILE *file = fopen(full_file_name, "wb");
    if (!file)
    {
        printf("Error creating file\n");
        return NULL;
    }

    char c[1024];
    int byte;
    bool flag = false;

    do
    {
        byte = recv(clientsocket, c, sizeof(c), 0);
        c[byte] = '\0';
        if (c[byte - 1] == '$')
        {
            c[byte - 1] = '\0';

            fwrite(c, 1, byte, file);

            flag = true;
        }
        else
        {
            fwrite(c, 1, byte, file);

            c[0] = '\0';
        }

    } while (!flag);

    fclose(file);
    sem_post(&W); // signal
    sem_post(&R);

    printf("Client %s uploaded file %s\n", nameu, file_name);
}
void file_handler(Details *users)
{
    // peek into the queue if empty then spinlock

    while (1)
    {

        if (isEmpty(&q))
        {
            sem_wait(&wait);
        }

        sem_wait(&R);
        UserCommand UC = peek(&q);
        dequeue(&q);

        pthread_t thread;

        if (strcmp(UC.command, "upload") == 0)
        {

            QueueArgc *QC = malloc(sizeof(QueueArgc));
            QC->C = UC;
            QC->users = users;
            pthread_create(&thread, NULL, writer, QC);
        }
        else if (strcmp(UC.command, "download") == 0)
        {
            QueueArgc *QC = malloc(sizeof(QueueArgc));
            QC->C = UC;
            QC->users = users;
            pthread_create(&thread, NULL, reader, QC);
        }
    }
}

int handle(int clientsocket, Details *users)
{
    // fetching user name and pass in user

    char user[100];
    int b = recv(clientsocket, user, sizeof(user), 0);
    user[b] = '\0';
    int uid = 0;

    char nameu[50];
    char passu[50];
    memset(nameu, '\0', 50);
    memset(passu, '\0', 50);

    // alag kr rha ho user name or pass ko

    const char *newline_pos = strchr(user, '~');
    int si = newline_pos - user;

    for (size_t i = 0; i < si; i++)
    {
        nameu[i] = user[i];
    }
    const char *line = strchr(user, '\0');
    int s2 = line - user;

    for (size_t i = si + 1; i < s2; i++)
    {
        passu[i - si - 1] = user[i];
    }

    // chk user exit with same pass otherwise create user

    userexist(user, nameu, passu, clientsocket);

    // check if user is already logged in

    int user_index = myhash(user);

    if (isuseralreadyloggedin(&v, user))
    {
        user_flag[user_index % (sizeof(user_flag) / sizeof(int))] = 1;
    }

    time_t now = time(NULL);

    struct tm *local = localtime(&now);

    int year = local->tm_year + 1900;
    int month = local->tm_mon + 1;
    int day = local->tm_mday;

    char date[50];
    sprintf(date, "%02d-%02d-%04d", day, month, year);
    while (true)
    {
        char c_buffer[50];
        int bytes = recv(clientsocket, c_buffer, sizeof(c_buffer), 0);
        if (bytes <= 0)
            break;

        c_buffer[bytes] = '\0';

        if (strcmp("upload", c_buffer) == 0)
        {

            if (!user_exist(users, nameu, &uid))
            {
                addUser(users, nameu, 50000);
                uid = users->size - 1;
            }

            char sizebuffer[20];
            int filesize = recv(clientsocket, sizebuffer, sizeof(sizebuffer), 0);
            sizebuffer[filesize] = '\0';

            int t_size = atoi(sizebuffer);

            if (users->data[uid].r_size - t_size >= 0)
            {
                const char *success_message = "$SUCCESS$";
                send(clientsocket, success_message, strlen(success_message), 0);

                char file_name[50];
                int name_bytes = recv(clientsocket, file_name, sizeof(file_name), 0);
                file_name[name_bytes] = '\0';
                char full_file_name[100];

                sprintf(full_file_name, "%s_%s", nameu, file_name);

                // if user req same file access
                if (user_flag[user_index % (sizeof(user_flag) / sizeof(int))] == 1)
                {
                    UserCommand U;
                    U.clientsocket = clientsocket;
                    U.command = c_buffer;
                    U.nameu = nameu;
                    U.date = date;
                    U.uid = uid;
                    U.file_name = file_name;
                    U.full_file_name = full_file_name;
                    U.t_size = t_size;
                    enqueue(&q, &U);
                    sem_post(&wait);
                }
                else
                {

                    addFile(users, file_name, t_size, date, uid);

                    users->data[uid].r_size -= t_size;

                    FILE *file = fopen(full_file_name, "wb");
                    if (!file)
                        continue;

                    char c[1024];
                    int byte;
                    bool flag = false;

                    do
                    {
                        byte = recv(clientsocket, c, sizeof(c), 0);
                        c[byte] = '\0';
                        if (c[byte - 1] == '$')
                        {
                            c[byte - 1] = '\0';
                            fwrite(c, 1, byte, file);
                            flag = true;
                        }
                        else
                        {
                            fwrite(c, 1, byte, file);
                            c[0] = '\0';
                        }

                    } while (!flag);

                    fclose(file);

                    printf("Client %s uploaded file %s\n", nameu, file_name);
                }
            }
            else
            {
                const char *fail_message = "$FAILURE$LOW_SPACE$";
                send(clientsocket, fail_message, strlen(fail_message), 0);
            }
        }
        else if (strcmp("download", c_buffer) == 0)
        {
            char file_name[50];
            int file_name_bytes = recv(clientsocket, file_name, sizeof(file_name), 0);
            file_name[file_name_bytes] = '\0';

            char full_file_name[strlen(nameu) + strlen(file_name) + 2];
            sprintf(full_file_name, "%s_%s", nameu, file_name);

            if (user_flag[user_index % (sizeof(user_flag) / sizeof(int))] == 1)
            {
                UserCommand U;
                U.clientsocket = clientsocket;
                U.command = c_buffer;
                U.nameu = nameu;
                U.date = date;
                U.uid = uid;
                U.file_name = file_name;
                enqueue(&q, &U);
                sem_post(&wait);
            }
            else
            {

                FILE *file = fopen(full_file_name, "rb");
                char ch[1024];
                if (file)
                {
                    size_t n;
                    while ((n = fread(ch, 1, 1024, file)) > 0)
                    {
                        send(clientsocket, ch, n, 0);
                    }

                    char c = '$';
                    send(clientsocket, &c, 1, 0);

                    fclose(file);
                    printf("Client %s downloaded file %s\n", nameu, file_name);
                }
                else
                {
                    const char *c = "$FAILURE$NO_CLIENT_DATA!";
                    send(clientsocket, c, sizeof(c), 0);
                }
            }
        }
        else if (strcmp(c_buffer, "view") == 0)
        {
            if (!user_exist(users, nameu, &uid))
            {
                const char *fail_message = "$FAILURE$NO_CLIENT_DATA$";
                send(clientsocket, fail_message, strlen(fail_message), 0);
                continue;
            }
            char detail[200];

            for (int i = 0; i < users->data[uid].size; i++)
            {
                char *info;
                char *date;
                int fsize;
                info = users->data[uid].data[i].f_name;
                date = users->data[uid].data[i].date;
                fsize = users->data[uid].data[i].f_size;
                sprintf(detail, "%s %s %d\n", info, date, fsize);
                send(clientsocket, detail, strlen(detail), 0);
            }
            send(clientsocket, "$", 1, 0);
        }
    }
    saveToFile("users.txt", users);
    close(clientsocket);
    return 0;
}
void *Threadwrapper(void *param)
{
    PTHREAD *params = (PTHREAD *)param;
    int clientsocket = params->clientsocket;
    Details *users = params->users;

    handle(clientsocket, users);
}
void *Threadwrapper_2(void *param)
{
    Details *params = (Details *)param;
    file_handler(params);
}

int main()
{
    initUserList(&users);
    loadFromFile("users.txt", &users);
    vector_init(&v);
    vector_init(&F);
    for (int i = 0; i < 50; i++)
    {
        vector_add(&v, 0);
        vector_add(&F, 0);
    }

    sem_init(&wait, 0, 0);
    sem_init(&W, 0, 1);
    sem_init(&R, 0, 1);
    sem_init(&RC, 0, 1);
    sem_init(&mutex, 0, 1);
    sem_init(&fcfs, 0, 1);
    pthread_mutex_init(&queue, NULL);

    initializeHeap(&h);
    Heap_init(&heap, &h);

    int serversocket, clientsocket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serversocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serversocket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("server is listening...\n");

    // file handler of reader writer problem
    pthread_t t = pthread_create(&t, NULL, Threadwrapper_2, &users);
    pthread_detach(t);

    while (true)
    {
        if ((clientsocket = accept(serversocket, (struct sockaddr *)&address, &addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        PTHREAD threads_arg;
        threads_arg.clientsocket = clientsocket;
        threads_arg.users = &users;

        pthread_t thread;
        thread = pthread_create(&thread, NULL, Threadwrapper, &threads_arg);

        pthread_detach(thread);
    }

    close(serversocket);
    return 0;
}
