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

//File encryption
char *encrypt(const char *a)
{
    static char encrypted[500];

    char ch = ' ';
    char pre = ' ';
    int count = 0;
    int size = strlen(a);
    int index = 0;

    for (int i = 0; i < size; i++)
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

//File decryption
char *decryption(const char *a)
{
    static char decrypted[500];
    int size = strlen(a);
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

//Image encryption
char* Img_encrypt(const char* a)
{
    static char encrypted[500];

    int size = strlen(a);

    for (int i = 0; i < size; i++)
    {
        encrypted[i] = a[i] + 3;
    }
    
    return encrypted;
}

//Image decryption
char* Img_decryption(const char* a)
{
    static char decrypted[500];

    int size = strlen(a);
    for (int i = 0; i < size; i ++)
    {
        decrypted[i] = a[i] - 3;
    }

    return decrypted;
}

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
                char *str = (char *)malloc(1024 * sizeof(char));

                if (hasImageExtension(name))
                {
                    size_t n;
                    while ((n = fread(ch, 1, 1024, file)) > 0)
                    {
                        str = Img_encrypt(ch);
                        send(clientsocket, str, strlen(str), 0);
						memset(ch, '\0', 1024);
                    }
                }
                else
                {
                    while (fgets(ch, 1024, file) != NULL)
                    {
                        str = encrypt(ch);
                        str[strlen(str)] = '\n';
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

        const char *d_path = "/home/skyark/Downloads/";
        char *path = (char *)malloc(strlen(d_path) + strlen(name) + 1);
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
        char *str = (char *)malloc(1024 * sizeof(char));

        if (hasImageExtension(name))
        {
            do
            {
                byte = recv(clientsocket, c, sizeof(c), 0);
                c[byte] = '\0';
                if (c[byte - 1] == '$')
                {
                    c[byte - 1] = '\0';
                    str = Img_decryption(c);
                    fwrite(str, 1,strlen(str), wr);
                    flag = true;
                }
                else
                {
                    str = Img_decryption(c);
                    fwrite(str, 1, strlen(str), wr);
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
                    str = decryption(c);
                    fputs(str, wr);
                    flag = true;
                }
                else
                {
                    str = decryption(c);
                    fputs(str, wr);
                    c[0] = '\0';
                }

            } while (!flag);
        }

        fclose(wr);
        free(path);
    }
    return 0;
}

int main()
{
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
        fgets(S, sizeof(S), stdin);
        strcat(str, S);
        command(str, clientsocket, user);
    }
    close(clientsocket);
    return 0;
}

