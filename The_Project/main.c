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

int main(int argc, char *argv[])
{
    char *expected_program_name = "chart";
    char *translated_name = argv[0];
    translated_name += 2;

    if (strcmp(translated_name, expected_program_name) != 0)
    {
        fprintf(stderr, "Hiba, a program neve nem \"chart\".\n");
        exit(3);
    }

    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);
    signal(SIGALRM, SignalHandler);

    srand(time(NULL));
    int *Values;

    if (argc == 1)
    {
        int NumValues = Measurement(&Values);
        SendViaFile(Values, NumValues);
    }
    else
    {
        if (argc > 3)
        {
            help();

            return EXIT_SUCCESS;
        }
        if (argc != 3)
        {
            argv[2] = "";
        }

        if (((strcmp(argv[1], "--version") != 0) || (strcmp(argv[1], "--help") != 0)) &&
            (((strcmp(argv[1], "-receive") != 0) || (strcmp(argv[2], "-receive") != 0)) &&
             ((strcmp(argv[1], "-send") != 0) || (strcmp(argv[2], "-send") != 0))) &&
            (((strcmp(argv[1], "-socket") != 0) || (strcmp(argv[2], "-socket") != 0)) &&
             ((strcmp(argv[1], "-file") != 0) || (strcmp(argv[2], "-file") != 0))) &&
            ((strcmp(argv[1], "") != 0) || (strcmp(argv[2], "") != 0)))
        {
            if (strcmp(argv[1], "--version") == 0)
            {
                version();

                return EXIT_SUCCESS;
            }
            else if (strcmp(argv[1], "--help") == 0)
            {
                help();

                return EXIT_SUCCESS;
            }
            else if ((((strcmp(argv[1], "-receive") == 0) || (strcmp(argv[2], "-receive") == 0)) &&
                      ((strcmp(argv[1], "-send") == 0) || (strcmp(argv[2], "-send") == 0))) ||
                     (((strcmp(argv[1], "-socket") == 0) || (strcmp(argv[2], "-socket") == 0)) &&
                      ((strcmp(argv[1], "-file") == 0) || (strcmp(argv[2], "-file") == 0))))
            {
                help();

                return EXIT_SUCCESS;
            }
            else if ((strcmp(argv[1], "-receive") == 0) || (strcmp(argv[2], "-receive") == 0))
            {
                if ((strcmp(argv[1], "-socket") == 0) || (strcmp(argv[2], "-socket") == 0))
                {
                    ReceiveViaSocket();
                }
                else if ((strcmp(argv[1], "-file") == 0) || (strcmp(argv[2], "-file") == 0) || (strcmp(argv[2], "") == 0))
                {
                    while (1)
                    {
                        printf("Várakozás a szignálra...\n");
                        signal(SIGUSR1, ReceiveViaFile);
                        pause();
                    }
                }
                else
                {
                    help();

                    return EXIT_SUCCESS;
                }
            }
            else if ((strcmp(argv[1], "-send") == 0) || (strcmp(argv[2], "-send") == 0))
            {
                if ((strcmp(argv[1], "-socket") == 0) || (strcmp(argv[2], "-socket") == 0))
                {
                    int NumValues = Measurement(&Values);
                    SendViaSocket(Values, NumValues);
                }
                else if ((strcmp(argv[1], "-file") == 0) || (strcmp(argv[2], "-file") == 0) || (strcmp(argv[2], "") == 0))
                {
                    int NumValues = Measurement(&Values);
                    SendViaFile(Values, NumValues);
                }
                else
                {
                    help();

                    return EXIT_SUCCESS;
                }
            }
            else if (((strcmp(argv[1], "-socket") == 0) && (strcmp(argv[2], "") == 0)) || (strcmp(argv[2], "-socket") == 0))
            {
                int NumValues = Measurement(&Values);
                SendViaSocket(Values, NumValues);
            }
            else if (((strcmp(argv[1], "-file") == 0) && (strcmp(argv[2], "") == 0)) || (strcmp(argv[2], "-file") == 0))
            {
                int NumValues = Measurement(&Values);
                SendViaFile(Values, NumValues);
            }
            else
            {
                help();

                return EXIT_SUCCESS;
            }
        }
        else
        {
            help();

            return EXIT_SUCCESS;
        }
    }

    free(Values);

    return EXIT_SUCCESS;
}
