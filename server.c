#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define PORT 12345

pthread_mutex_t lock;

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

void addFile(Details *list, const char *f_name, int f_size, char date[], int uid)
{
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
        printf("file not opened..");
    }
}
int handle(int clientsocket, Details *users)
{
    // fetching user name and pass in user

    // pthread_mutex_lock(&lock);

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

    // chk user exit with same pass otherwise creat user

    userexist(user, nameu, passu, clientsocket);

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
                addUser(users, nameu, 10240);
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

                addFile(users, file_name, t_size, date, uid);

                users->data[uid].r_size -= t_size;

                FILE *file = fopen(full_file_name, "wb");
                if (!file)
                    continue;

                char c[1024];
                int byte;
                bool flag = false;

                if (hasImageExtension(file_name))
                {
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
                            fputs(c, file);
                            flag = true;
                        }
                        else
                        {
                            fputs(c, file);
                            c[0] = '\0';
                        }

                    } while (!flag);
                }

                fclose(file);

                printf("Client %s uploaded file %s\n", nameu, file_name);
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

            char full_file_name[strlen(user) + strlen(file_name) + 2];
            sprintf(full_file_name, "%s_%s", nameu, file_name);

            FILE *file = fopen(full_file_name, "rb");
            char ch[1024];
            if (file)
            {
                if (hasImageExtension(full_file_name))
                {
                    size_t n;
                    while ((n = fread(ch, 1, 1024, file)) > 0)
                    {
                        send(clientsocket, ch, n, 0);
                    }
                }
                else
                {
                    while (fgets(ch, 1024, file) != NULL)
                    {
                        send(clientsocket, ch, strlen(ch), 0);
                    }
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
        // pthread_mutex_unlock(&lock);
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

int main()
{
    initUserList(&users);
    loadFromFile("users.txt", &users);

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
