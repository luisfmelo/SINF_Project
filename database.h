#include <postgresql/libpq-fe.h>
#include <iostream> // cout
#include <sstream> // cout
#include <stdlib.h>  // exit
#include <string.h> // bzero

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>
#include <unistd.h>

#include <set>
#include <postgresql/libpq-fe.h>

using namespace std;

void initDB();
PGresult* executeSQL(string sql);
void closeDB();
