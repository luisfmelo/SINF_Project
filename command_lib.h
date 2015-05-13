#include <iostream> // cout
#include <sstream> // cout
#include <stdlib.h>  // exit
#include <string.h> // bzero
#include <fstream>  //help_c
#include <vector>       // std::vector
#include <algorithm>    // random_shuffle
#include <postgresql/libpq-fe.h>
#include <time.h>
#include "database.h"
#include <ctype.h>

using namespace std;

#define TIME_BETWEEN_QUESTIONS 5

extern int mainsocket;

void * jogo(void * args);

void writeline(int socketid, string line);
void help_c(int socketid);
void register_c(int socketid, string args);
void identify_c(int socketid, string args);
void login_c(int socketid, string args);
void logout_c(int socketid);
void resetpassword_c(int socketid, string args);
void changepassword_c(int socketid, string args);
void changeusername_c(int socketid, string args);
void question_c(int socketid, string args);
void showallquestions_c(int socketid);
void editquestion_c(int socketid, string args);
void deletequestion_c(int socketid, string args);
void changepermissions_c(int socketid, string args);
void create_c(int socketid, string args);
void challenge_c(int socketid, string args);
void start_c(int socketid, string args);
void accept_c(int socketid, string args);
void usersready_c(int socketid, string args);
void answer_c(int socketid, string args);
void showaskusers_c(int socketid, string args);
void setaskusers_c(int socketid, string args);
void fiftyfifty_c(int socketid, string args);

void banidoporadmin_c(int socketid);
void deleteaccount_c(int socketid, string args);

void say_c(int socketid, string args);
void listusers(int socketid);
bool islogged(int socketid);
int isadmin(int socketid);
bool userexists(string user);
void decline_c(int socketid, string args);
void shutdown_c(int sockfd);
void listusers_admin(int socketid);
int alphanumeric(string str);
string insensitivestring(string original);

string intToString(int i);
string numToResp(int i);