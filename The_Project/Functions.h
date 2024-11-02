#ifndef Functions
#define Functions

void version();
void help();
void SignalHandler(int sig);
int Measurement(int **values);
void BMPcreator(int *Values, int NumValues);
int FindPID();
void SendViaFile(int *Values, int NumValues);
void ReceiveViaFile(int sig);
int SendViaSocket(int *Values, int NumValues);
int ReceiveViaSocket();

#endif
