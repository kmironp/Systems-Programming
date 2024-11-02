#include "Functions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <omp.h>

#define BUFFER_SIZE 1024
#define MAX_PATH_LENGTH 4096

char SYMBOL_HELP = '"';

void version()
{
    char *info[8] = {
        "---",
        "Version: 1.8.6",
        "License: AX6Z80",
        "Time Zone: (UTC+01:00) Budapest, Belgrad, Ljubljana, Pozsony, Praga",
        "The date of completion: 2023.Apr.30. - 11:19:12",
        "The name of the developer: Katona Miron Peter",
        "Copyright (c) 2023 Katona Miron Peter",
        "---"};

#pragma omp parallel num_threads(8)
    {
        int numThreads = omp_get_thread_num();
        printf("%s\n", info[numThreads]);
    }
}

void help()
{
    printf("\x1B[1mAvailable run options and their meanings:\x1B[0m\n"); // Lehetseges futtatasi opciok ésjelentesuk
    printf("\x1B[1mUsage: --version\x1B[0m\n"
           "If we start the program with this, it will display the program version number, license, "
           "time zone, completion date, developer's name and copyright. \n");
    /*
        Ha ezzel indítjuk el a programot, akkor megjelenik a program verziószáma, a licenc,
        az időzóna, a befejezés dátuma, a fejlesztő neve és a szerzői jog.
    */
    printf("---\n");
    printf("\x1B[1mUsage: -send\x1B[0m\nThis is the default mode, i.e. if it is not specified explicitly, the system behaves as a sender!\n");
    // Ez az alapértelmezett üzemmód, azaz ha nincs explicit módon megadva, akkor a rendszer feladóként viselkedik!
    printf("---\n");
    printf("\x1B[1mUsage: -receive\x1B[0m\nIn this mode, the system acts as a receiver!\n");
    // Ennél az üzemmódnál fogadóként viselkedik a rendszer!
    printf("---\n");
    printf("\x1B[1mUsage: -file\x1B[0m\nThis is the default communication mode, i.e. if it is not specified explicitly, "
           "the system uses a file during communication!\n(file name should be %cchart%c)\n",
           SYMBOL_HELP, SYMBOL_HELP);
    /*
        Ez az alapértelmezett kommunikációs mód, azaz ha nincs explicit módon megadva,
        akkor a rendszer egy fájlt használ a kommunikáció során!
        (a fájl neve legyen "chart")
    */
    printf("---\n");
    printf("\x1B[1mUsage: -socket\x1B[0m\nIn this communication mode, the system communicates via a network!\n");
    // Ezzel a kommunikációs móddal a rendszer hálózaton keresztül kommunikál!
    printf("---\n");
    printf("\x1B[1mNote:\x1B[0m If the mode and communication mode switches are used together, their order is arbitrary!\n");
    // Megjegyzés: Az üzemmód és a kommunikációs mód kapcsolók együttes használata esetén a sorrendjük tetszőleges.
    printf("---\n");

    exit(EXIT_SUCCESS);
}

void SignalHandler(int sig)
{
    if (sig == SIGINT)
    {
        printf("\nSIGINT szingált kaptam.\n");
        printf("A program leállítása...\n");
        printf("Legyen szép a napja, viszlát!\n");
        exit(EXIT_SUCCESS);
    }
    if (sig == SIGUSR1)
    {
        printf("SIGUSR1 szignált kaptam.\n");
        printf("A fájlon keresztüli küldés szolgáltatás jelenleg nem elérhető!\n");
        exit(1);
    }
    if (sig == SIGALRM)
    {
        printf("SIGALRM szignált kaptam.\n");
        fprintf(stderr, "A szerver (a megadott időkereten belül) nem válaszolt...\n");
        exit(2);
    }
}

int Measurement(int **Values)
{
    time_t now;
    time(&now);
    struct tm *currentTime = localtime(&now);
    int NumValues = (currentTime->tm_min * 60 + currentTime->tm_sec);

    if ((NumValues > 100) && (NumValues < 901))
    {
        NumValues = NumValues;
    }
    else if ((NumValues > 900) && (NumValues < 1801))
    {
        NumValues = NumValues - 900;
    }
    else if ((NumValues > 1800) && (NumValues < 2701))
    {
        NumValues = NumValues - 1800;
    }
    else
    {
        NumValues = NumValues - 2700;
    }

    if (NumValues < 100)
    {
        NumValues = 100;
    }

    *Values = (int *)malloc(NumValues * sizeof(int));
    if (*Values == NULL)
    {
        fprintf(stderr, "Hiba, ha nem sikerül a memóriafoglalás.\n");
        exit(5);
    }

    int x = 0;
    for (int i = 0; i < NumValues; i++)
    {
        if (i == 0)
        {
            (*Values)[i] = x;
            continue;
        }
        double r = rand() % 100 + 1;
        if (r < (100.00 / 31.00 * 11))
        {
            x--;
        }
        else if (r <= ((100.00 / 31.00 * 11) + 42.8571))
        {
            x++;
        }
        else
        {
            x = x;
        }
        (*Values)[i] = x;
    }

    return NumValues;
}

void BMPcreator(int *Values, int NumValues)
{
    int width = NumValues;
    int height = NumValues;
    int rowSize = (width + 31) / 32 * 4;
    int pixelArraySize = rowSize * height;
    int fileSize = 54 + 8 + pixelArraySize;

    unsigned char fileHeader[14] = {'B', 'M', fileSize & 0xFF, (fileSize >> 8) & 0xFF, (fileSize >> 16) & 0xFF, (fileSize >> 24) & 0xFF, 0, 0, 0, 0, 54 + 8, 0, 0, 0};
    unsigned char infoHeader[40] = {40, 0, 0, 0, width & 0xFF, (width >> 8) & 0xFF, (width >> 16) & 0xFF, (width >> 24) & 0xFF, height & 0xFF, (height >> 8) & 0xFF, (height >> 16) & 0xFF, (height >> 24) & 0xFF, 1, 0, 1, 0, 0, 0, 0, 0, rowSize & 0xFF, (rowSize >> 8) & 0xFF, (rowSize >> 16) & 0xFF, (rowSize >> 24) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0};

    unsigned char colorTable[8] = {
        0,
        0,
        0,
        0,
        255,
        255,
        255,
        0,
    };

    unsigned char pixelArray[pixelArraySize];
    memset(pixelArray, 0, pixelArraySize);

    int index = (height / 2) * rowSize;
    int bitIndex = 8;
    unsigned char byte = 0;
    int value;
    int prev;
    int tmp;
    int shift = 0;

    for (int i = 0; i < NumValues; i++)
    {
        value = Values[i];
        if (i == 0)
        {
            value = 0;
            byte |= (value << bitIndex);
            bitIndex--;
            pixelArray[index] |= (1 << bitIndex);
            bitIndex++;
        }
        else
        {
            if ((value > height / 2) || (value == height / 2))
            {
                if (NumValues % 2 == 0)
                {
                    tmp = height / 2;
                    index = ((height / 2) + tmp - 1) * rowSize + shift;
                    value = 0;
                    byte |= (value << bitIndex);
                    bitIndex--;
                    pixelArray[index] |= (1 << bitIndex);
                    bitIndex++;
                }
                else
                {
                    tmp = height / 2;
                    index = ((height / 2) + tmp) * rowSize + shift;
                    value = 0;
                    byte |= (value << bitIndex);
                    bitIndex--;
                    pixelArray[index] |= (1 << bitIndex);
                    bitIndex++;
                }
            }
            else if (value < height / 2 * (-1))
            {
                tmp = height / 2;
                index = ((height / 2) - tmp) * rowSize + shift;
                value = 0;
                byte |= (value << bitIndex);
                bitIndex--;
                pixelArray[index] |= (1 << bitIndex);
                bitIndex++;
            }
            else if (value == prev)
            {
                index = (height / 2) * rowSize + shift;
                value = 0;
                byte |= (value << bitIndex);
                bitIndex--;
                pixelArray[index] |= (1 << bitIndex);
                bitIndex++;
            }
            else if (value > prev)
            {
                index = ((height / 2) + value) * rowSize + shift;
                value = 0;
                byte |= (value << bitIndex);
                bitIndex--;
                pixelArray[index] |= (1 << bitIndex);
                bitIndex++;
            }
            else if (value < prev)
            {
                index = ((height / 2) + value) * rowSize + shift;
                value = 0;
                byte |= (value << bitIndex);
                bitIndex--;
                pixelArray[index] |= (1 << bitIndex);
                bitIndex++;
            }
            else
            {
                value = 1;
            }
        }
        prev = value;
        bitIndex--;
        if (bitIndex == 0)
        {
            bitIndex = 8;
            shift++;
        }
    }

    FILE *file = fopen("chart.bmp", "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Hiba, a fájl megnyitása során!\n");
        exit(4);
    }
    fwrite(fileHeader, 1, 14, file);
    fwrite(infoHeader, 1, 40, file);
    fwrite(colorTable, 1, 8, file);
    fwrite(pixelArray, 1, pixelArraySize, file);
    int permission = chmod("chart.bmp", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (permission != 0)
    {
        fprintf(stderr, "Hiba, a jogosultsagok nem allithatok be!\n");
        exit(6);
    }
    fclose(file);
}

int FindPID()
{
    int pid = -1;
    char buffer[BUFFER_SIZE];
    char name[BUFFER_SIZE];
    DIR *dir = opendir("/proc");
    struct dirent *entry;

    if (dir == NULL)
    {
        fprintf(stderr, "Hiba, nem sikerült megnyitni a /proc directory-t.\n");
        return pid;
    }

    pid_t myPid = getpid();

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && entry->d_name[0] >= '0' && entry->d_name[0] <= '9')
        {
            sprintf(buffer, "/proc/%s/status", entry->d_name);
            FILE *fp = fopen(buffer, "r");

            if (fp == NULL)
            {
                fprintf(stderr, "Nem sikerült megnyitni %s.\n", buffer);
                continue;
            }

            while (fgets(buffer, BUFFER_SIZE, fp))
            {
                if (sscanf(buffer, "Name:\t%s\n", name) == 1 && strcmp(name, "chart") == 0)
                {
                    int currentPid = atoi(entry->d_name);

                    if (currentPid != myPid)
                    {
                        while (fgets(buffer, BUFFER_SIZE, fp))
                        {
                            if (sscanf(buffer, "Pid:\t%d", &pid) == 1)
                            {
                                fclose(fp);
                                closedir(dir);
                                return pid;
                            }
                        }
                    }
                }
            }

            fclose(fp);
        }
    }
    closedir(dir);
    return pid;
}

void SendViaFile(int *Values, int NumValues)
{
    char name[BUFFER_SIZE];
    sprintf(name, "%s/Measurement.txt", getenv("HOME"));
    FILE *fp = fopen(name, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Hiba, a Measurement.txt fájl megnyitásánál.\n");
        exit(4);
    }

    for (int i = 0; i < NumValues; i++)
    {
        fprintf(fp, "%d\n", Values[i]);
    }

    fclose(fp);

    free(Values);

    int pid = FindPID();
    if (pid == -1)
    {
        fprintf(stderr, "Nem található üzemmódban működő folyamat (process).\n");
        exit(7);
    }

    kill(pid, SIGUSR1);
}

void ReceiveViaFile(int sig)
{
    int minValues = 100;

    int *Values = NULL;
    int NumValues = 0;

    char name[BUFFER_SIZE];
    sprintf(name, "%s/Measurement.txt", getenv("HOME"));
    FILE *file = fopen(name, "r");

    if (file == NULL)
    {
        printf("Hiba, nem sikerült megnyitni a fájlt.\n");
        exit(4);
    }

    Values = (int *)malloc(minValues * sizeof(int));

    if (Values == NULL)
    {
        fprintf(stderr, "Hiba, nem sikerült allokálni.\n");
        exit(8);
    }

    while (fscanf(file, "%d", &Values[NumValues]) == 1)
    {
        NumValues++;

        if (NumValues == minValues)
        {
            Values = (int *)realloc(Values, minValues * 2 * sizeof(int));

            if (Values == NULL)
            {
                fprintf(stderr, "Hiba, nem sikerült újra allokálni.\n");
                exit(8);
            }

            minValues = minValues * 2;
        }
    }

    fclose(file);

    BMPcreator(Values, NumValues);

    free(Values);
}

// Client
int SendViaSocket(int *Values, int NumValues)
{
    // Declarations
    int s;                     // socket ID
    int bytes;                 // received/sent bytes
    int flag;                  // transmission flag
    char on;                   // sockopt option
    unsigned int server_size;  // length of the sockaddr_in server
    struct sockaddr_in server; // address of server
    int PORT_NUMBER = 3333;

    // Initialization
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT_NUMBER);
    server_size = sizeof server;

    // Creating socket
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        fprintf(stderr, "Hiba, a socket létrehozásánál.\n");
        exit(9);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    // Sending data
    printf("Üzenet küldése... ");
    bytes = sendto(s, &NumValues, sizeof(int), flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        fprintf(stderr, "Hiba, a Küldésnél.\n");
        exit(10);
    }
    printf("(%i byte)\n", bytes - 1);

    signal(SIGALRM, SignalHandler);

    alarm(1);

    // Receive data
    int receivedSize = 0;
    bytes = recvfrom(s, &receivedSize, sizeof(int), flag, (struct sockaddr *)&server, &server_size);
    if (NumValues != receivedSize)
    {
        fprintf(stderr, "Hiba, a fogadásnál. (Az küldött és kapott adatok nem egyenlő méretűek)\n");
        exit(11);
    }
    printf("A szerver (%s:%d) nyugtája: %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port), receivedSize);
    printf("---\n");

    // A feladat folytása, a tömb küldése(numvalus, values értékei)...
    printf("Adatok küldése üzenetként... ");
    int currentSize = NumValues * sizeof(int);
    bytes = sendto(s, Values, currentSize, flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        fprintf(stderr, "Hiba, a Küldésnél.\n");
        exit(10);
    }
    printf("(%i byte)\n", bytes - 1);

    receivedSize = 0;
    bytes = recvfrom(s, &receivedSize, sizeof(int), flag, (struct sockaddr *)&server, &server_size);
    if (currentSize != receivedSize)
    {
        fprintf(stderr, "Fogadási hiba! A küldött és kapott méretek nem egyenlőek.\n");
        exit(11);
    }
    printf("A szerver (%s:%d) nyugtája: %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port), receivedSize);

    // Closing
    signal(SIGALRM, SIG_IGN);
    close(s);

    return EXIT_SUCCESS;
}

// Server
int ReceiveViaSocket()
{
    // Declarations
    int s;
    int bytes;                 // received/sent bytes
    int err;                   // error code
    int flag;                  // transmission flag
    char on;                   // sockopt option
    unsigned int server_size;  // length of the sockaddr_in server
    unsigned int client_size;  // length of the sockaddr_in client
    struct sockaddr_in server; // address of server
    struct sockaddr_in client; // address of client
    int PORT_NUMBER = 3333;

    // Initialization
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NUMBER);
    server_size = sizeof server;
    client_size = sizeof client;

    // Creating socket
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        fprintf(stderr, "Hiba, a socket létrehozásánál.\n");
        exit(9);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    // Binding socket
    err = bind(s, (struct sockaddr *)&server, server_size);
    if (err < 0)
    {
        fprintf(stderr, "Hiba, a kötésnél.\n");
        exit(12);
    }
    int count = 0;

    // Continuous server operation
    while (1)
    {
        if (count > 0)
        {
            printf("\n\n");
        }

        // Receive data
        printf("Várakozás az üzenetre...\n");
        int receivedSize = 0;
        bytes = recvfrom(s, &receivedSize, sizeof(int), flag, (struct sockaddr *)&client, &client_size);
        if (bytes < 0)
        {
            fprintf(stderr, "Hiba, a fogadásnál.\n");
            exit(11);
        }
        printf("- %d byte került fogadásra a (%s:%d) klienstől.\nA kliens üzenete: \n- Az %d ", bytes - 1, inet_ntoa(client.sin_addr), ntohs(client.sin_port), receivedSize);

        int currentSize = receivedSize * sizeof(int);
        int *values = calloc(receivedSize, currentSize);

        // Sending data
        bytes = sendto(s, &receivedSize, sizeof(int), flag, (struct sockaddr *)&client, client_size);
        if (bytes <= 0)
        {
            fprintf(stderr, "Hiba, a küldésnél.\n");
            exit(10);
        }
        printf("nyugta elküldve a kliensnek.\n");

        printf("\nVárakozás újabb üzenetre...\n");

        bytes = recvfrom(s, values, currentSize, flag, (struct sockaddr *)&client, &client_size);
        if (bytes < 0)
        {
            fprintf(stderr, "Hiba, a fogadásnál.\n");
            exit(11);
        }
        printf("- %d byte került fogadásra a (%s:%d) klienstől.\nA kliens üzenete: \n- Az %d ", bytes - 1, inet_ntoa(client.sin_addr), ntohs(client.sin_port), currentSize);

        // Sending data
        bytes = sendto(s, &currentSize, sizeof(int), flag, (struct sockaddr *)&client, client_size);
        if (bytes <= 0)
        {
            fprintf(stderr, "Küldési hiba.\n");
            exit(10);
        }
        printf("nyugta elküldve a kliensnek.\n");

        BMPcreator(values, receivedSize);

        free(values);
        count++;
    }

    return EXIT_SUCCESS;
}
