#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <arpa/inet.h>

#define PORT 12345

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
                    while (heap->arr[offset + j] == 0)
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

typedef struct Stack
{
    char *Vs;
    int size;
    int top;
} Stack;
Stack *createStack(int s)
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->size = s;
    stack->top = 0;
    stack->Vs = (char *)malloc(s * sizeof(char));
    return stack;
}

void push(Stack *stack, char V)
{
    if (stack->top != stack->size)
    {
        stack->Vs[stack->top] = V;
        stack->top++;
    }
    else
    {
        printf("Stack Overflow!!!\n");
    }
}

void pop(Stack *stack)
{
    stack->top--;
}

char top(Stack *stack)
{
    return stack->Vs[stack->top - 1];
}

bool isEmpty(Stack *stack)
{
    return stack->top == 0;
}

bool isFull(Stack *stack)
{
    return stack->top == stack->size;
}

void printStack(Stack *stack)
{
    for (int i = 0; i < stack->top; i++)
    {
        printf("%c", stack->Vs[i]);
    }
    printf("\n");
}

void destroyStack(Stack *stack)
{
    free(stack->Vs);
    free(stack);
}
bool fileExists(const char *filePath)
{
    FILE *file = fopen(filePath, "r");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    fclose(file);
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

// File encryption
char *encrypt(const char *a, int size)
{
    static char encrypted[1024];

    char ch = ' ';
    char pre = ' ';
    int count = 0;
    int index = 0;

    for (int i = 0; i <= size; i++)
    {
        ch = a[i];
        if (ch == pre)
        {
            count++;
        }
        else
        {
            if (count > 0)
            {
                encrypted[index++] = pre;
                encrypted[index++] = '0' + count;
            }
            count = 1;
        }
        pre = ch;
    }
    encrypted[index] = '\0';
    return encrypted;
}

// File decryption
char *decryption(const char *a, int size)
{
    static char decrypted[1024];
    int index = 0;

    char ch = ' ';
    for (int i = 0; i < size; i += 2)
    {
        ch = a[i];
        for (int j = 0; j < a[i + 1] - '0'; j++)
        {
            decrypted[index++] = ch;
        }
    }

    decrypted[index] = '\0';
    return decrypted;
}

// Image encryption
char *Img_encrypt(const char *a, int size)
{
    static char encrypted[1024];

    for (int i = 0; i < size; i++)
    {
        encrypted[i] = a[i] + 3;
    }

    return encrypted;
}

// Image decryption
char *Img_decryption(const char *a, int size)
{
    static char decrypted[1024];

    for (int i = 0; i < size; i++)
    {
        decrypted[i] = a[i] - 3;
    }

    return decrypted;
}

Heap heap;
MinHeap h;
int command(char str[], int clientsocket, char user[])
{
    // parsing the command string
    char name[50], p[100], c[50], b[20];
    int j = 1;
    int len = 0;
    while (j <= strlen(str) && str[j] != '$')
    {
        c[len] = str[j];
        c[len] = tolower(c[len]);
        c[len + 1] = '\0';
        j++;
        len++;
    }
    int l = j + 1;
    len = 0;

    while (l < strlen(str) && str[l] != '/')
    {
        b[len] = str[l];
        b[len + 1] = '\0';
        l++;
        len++;
    }
    if (strcmp(c, "upload") == 0)
    {
        int j = strlen(c) + strlen(b) + 2;
        len = 0;
        while (j < strlen(str) - 2)
        {
            p[len] = str[j];
            p[len + 1] = '\0';
            j++;
            len++;
        }

        int i = strlen(str) - 3;
        Stack *S = createStack(100);
        while (i >= 0 && str[i] != '/')
        {
            push(S, str[i]);
            i--;
        }
        len = 0;
        while (!isEmpty(S))
        {
            char c = top(S);
            name[len] = c;
            name[len + 1] = '\0';
            pop(S);
            len++;
        }
    }
    else if (strcmp(c, "download") == 0)
    {

        int i = strlen(str) - 3;
        Stack *S = createStack(100);
        while (i >= 0 && str[i] != '$')
        {
            push(S, str[i]);
            i--;
        }
        len = 0;
        while (!isEmpty(S))
        {
            char c = top(S);
            name[len] = c;
            name[len + 1] = '\0';
            pop(S);
            len++;
        }
    }

    // sending the command
    if (send(clientsocket, c, strlen(c), 0) == -1)
    {
        printf("Error sending info...\n");
        close(clientsocket);
        return 1;
    }

    if (strcmp(c, "view") == 0)
    {
        char msg[100];

        int bytes;
        bool flag = false;
        do
        {
            bytes = recv(clientsocket, msg, sizeof(msg), 0);
            msg[bytes] = '\0';
            if (strcmp(msg, "$FAILURE$NO_CLIENT_DATA$") == 0)
            {
                printf("%s\n", msg);
                break;
            }
            else if (msg[bytes - 1] == '$')
            {
                msg[bytes - 1] = '\0';
                printf("%s\n", msg);
                flag = true;
            }
            else
                printf("%s\n", msg);

        } while (!flag);
    }
    else if (strcmp(c, "upload") == 0)
    {
        if (fileExists(p))
        {
            sleep(1);

            // upload the file to the server

            FILE *file = fopen(p, "rb");
            if (file == NULL)
            {
                printf("%s\n", "file does not exist");
            }

            // sending file size
            send(clientsocket, b, strlen(b), 0);

            // receiving success message
            char m_buffer[100];

            int bytes = recv(clientsocket, m_buffer, sizeof(m_buffer), 0);
            m_buffer[bytes] = '\0';

            if (strcmp(m_buffer, "$SUCCESS$") == 0)
            {

                // sending the name and contents

                if (send(clientsocket, name, strlen(name), 0) == -1)
                {
                    printf("Error sending info...\n");
                    close(clientsocket);
                    return 1;
                }
                char ch[1024];
                char *str = (char *)Heap_ITUN(&heap,1024 * sizeof(char),&h);

                if (hasImageExtension(name))
                {
                    size_t n;
                    while ((n = fread(ch, 1, 1024, file)) > 0)
                    {
                        str = Img_encrypt(ch, n);
                        send(clientsocket, str, n, 0);
                        memset(ch, '\0', 1024);
                    }
                }
                else
                {
                    size_t n;
                    while ((n = fread(ch, 1, 1024, file)) > 0)
                    {
                        str = encrypt(ch, n);
                        send(clientsocket, str, strlen(str), 0);
                        memset(ch, '\0', 1024);
                    }
                }

                char c = '$';
                send(clientsocket, &c, 1, 0);

                fclose(file);
            }
            else
            {
                printf("%s", m_buffer);
            }
        }
        else
        {
            printf("File does not exist.\n");
        }
    }
    else
    {
        sleep(1);

        // download the file

        if (send(clientsocket, name, strlen(name), 0) == -1)
        {
            printf("Error sending info...\n");
            close(clientsocket);
            return 1;
        }

        // char directory[100];
        // fgets(directory, 100, stdin);

        const char *d_path = "/home/skyark/Downloads/";

        // strcpy(d_path, directory);

        char *path = (char *)Heap_ITUN(&heap,strlen(d_path) + strlen(name) + 1,&h);
        strcpy(path, d_path);
        strcat(path, name);
        FILE *wr = fopen(path, "wb");

        if (wr == NULL)
        {
            printf("Error opening file.\n");
            return 1;
        }

        // if file exists only then download
        char c[1024];
        int byte;
        bool flag = false;
        char *str = (char *)Heap_ITUN(&heap,1024 * sizeof(char),&h);

        if (hasImageExtension(name))
        {
            do
            {
                byte = recv(clientsocket, c, sizeof(c), 0);
                c[byte] = '\0';
                if (c[byte - 1] == '$')
                {
                    c[byte - 1] = '\0';
                    str = Img_decryption(c, byte);

                    fwrite(str, 1, byte, wr);
                    flag = true;
                }
                else
                {
                    str = Img_decryption(c, byte);
                    fwrite(str, 1, byte, wr);
                    c[0] = '\0';
                }

            } while (!flag);
        }
        else
        {

            do
            {
                byte = recv(clientsocket, c, sizeof(c), 0);

                c[byte] = '\0';
                if (c[byte - 1] == '$')
                {
                    c[byte - 1] = '\0';
                    str = decryption(c, byte);

                    fwrite(str, 1, strlen(str), wr);

                    flag = true;
                }
                else
                {
                    str = decryption(c, byte);
                    fwrite(str, 1, strlen(str), wr);

                    c[0] = '\0';
                }

            } while (!flag);
        }

        fclose(wr);
    }
    return 0;
}

int main()
{
    initializeHeap(&h);
    Heap_init(&heap, &h);

    int status, valread, clientsocket;
    struct sockaddr_in serv_addr;

    if ((clientsocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(clientsocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    char msg[50] = "wrong password";
    char user[50];
    memset(user, '\0', 50);
    printf("login-enter username: ");
    scanf("%s", user);

    int s = strlen(user);
    user[s] = '~';
    printf("login-enter password: ");
    scanf("%s", user + s + 1);
    int s2 = strlen(user);
    user[s2] = '\0';
    send(clientsocket, user, sizeof(user), 0);

    recv(clientsocket, msg, sizeof(msg), 0);

    while (strcmp(msg, "wrong password") == 0)
    {
        char pass[50];
        memset(pass, '\0', 50);
        printf("wrong password enter again: ");
        scanf("%s", pass);
        int s2 = strlen(pass);
        pass[s2] = '\0';
        send(clientsocket, pass, sizeof(pass), 0);

        recv(clientsocket, msg, sizeof(msg), 0);
    }

    printf("%s", msg);
    printf("\n");

    printf("client connected to the server....\n");

    while (1)
    {
        char str[500];
        char S[500];
        printf("enter command>\n");
        scanf("%s", str);
        if (strcmp(str, "clear") == 0)
        {
            system("clear");
            continue;
        }
        if (strcmp(str, "exit") == 0)
        {
            system("exit");
            continue;
        }
        fgets(S, sizeof(S), stdin);
        strcat(str, S);
        command(str, clientsocket, user);
    }
    close(clientsocket);
    return 0;
}
