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
#include <map>
#include <postgresql/libpq-fe.h>

#include "command_lib.h"
#include "database.h"

using namespace std;

set<int> clients;

extern map<string, int> sockets;
extern map<int, string> usernames;

int mainsocket;

bool iscommand( string line) {
	string::iterator it = line.begin();
	
	if(*it == '\\')
		return true;
	
	return false;
}

/* Lê uma linha de um socket
   retorna false se o socket se tiver fechado */
bool readline(int socketfd, string &line) {
  int n; 
  /* buffer de tamanho 1025 para ter espaço 
     para o \0 que indica o fim de string*/
  char buffer[1025]; 

  /* inicializar a string */
  line = "";

  /* Enquanto não encontrarmos o fim de linha
     vamos lendo mais dados da stream */
  while (line.find('\n') == string::npos) {
    // leu n carateres. se for zero chegamos ao fim
    int n = read(socketfd, buffer, 1024); // ler do socket
    if (n == 0) return false; // nada para ser lido -> socket fechado
    buffer[n] = 0; // colocar o \0 no fim do buffer
    line += buffer; // acrescentar os dados lidos à string
  }

  // Retirar o \r\n (lemos uma linha mas não precisamos do \r\n)
  line.erase(line.end() - 1);
  line.erase(line.end() - 1);
  return true;  
}

/* Envia uma mensagem para todos os clientes ligados exceto 1 */
void broadcast (int origin, string text) {
   /* Usamos um ostringstream para construir uma string
      Funciona como um cout mas em vez de imprimir no ecrã
      imprime numa string */
   ostringstream message;
   message << origin << " said: " << text;

   // Iterador para sets de inteiros 
   set<int>::iterator it;
   for (it = clients.begin(); it != clients.end(); it++)
     if (*it != origin) writeline(*it, message.str());
}

/* Trata de receber dados de um cliente cujo socketid foi
   passado como parâmetro */
void* cliente(void* args) {
  int sockfd = *(int*)args;
  string line;

  clients.insert(sockfd);
  
	writeline(sockfd, "\033[2J");
	writeline(sockfd, "\033[f");
	writeline(sockfd, "Quem não quer ser miseravelmente pobre?");

  cout << "Client connected: " << sockfd << "\n";
  while (readline(sockfd, line)) 
  {
	if(iscommand(line)) {
		string comando, argumentos, palavra;
		istringstream iss(line);
	
		iss >> comando;
	
		while (iss >> palavra) 
			argumentos += palavra + ' ';	 
		if(!comando.compare("\\help"))
			help_c(sockfd);
		else if(!comando.compare("\\register"))
			register_c(sockfd, argumentos);				
		else if(!comando.compare("\\say"))
			say_c(sockfd, argumentos);
		else if(!comando.compare("\\identify"))
			identify_c(sockfd, argumentos);
		else if(!comando.compare("\\login"))
			login_c(sockfd, argumentos);
		else if(!comando.compare("\\logout"))
			logout_c(sockfd);
		else if(!comando.compare("\\resetpassword"))
			resetpassword_c(sockfd, argumentos);	
		else if(!comando.compare("\\changepassword"))
			changepassword_c(sockfd, argumentos);		
		else if(!comando.compare("\\changeusername"))
			changeusername_c(sockfd, argumentos);	
		else if(!comando.compare("\\question"))
			question_c(sockfd, argumentos);		
		else if(!comando.compare("\\showallquestions"))
			showallquestions_c(sockfd);	
		else if(!comando.compare("\\listusers"))
			listusers(sockfd);
		else if(!comando.compare("\\editquestion"))
			editquestion_c(sockfd, argumentos);		
		else if(!comando.compare("\\deletequestion"))
			deletequestion_c(sockfd, argumentos);
		else if(!comando.compare("\\changepermissions"))
			changepermissions_c(sockfd, argumentos);		
		else if(!comando.compare("\\create"))
			create_c(sockfd, argumentos);
		else if(!comando.compare("\\start"))
			start_c(sockfd, argumentos);
		else if(!comando.compare("\\challenge"))
			challenge_c(sockfd, argumentos);	
		else if(!comando.compare("\\accept"))
			accept_c(sockfd, argumentos);	
		else if(!comando.compare("\\usersready"))
			usersready_c(sockfd, argumentos);
		else if(!comando.compare("\\decline"))
			decline_c(sockfd, argumentos);
		else if(!comando.compare("\\listusers_admin"))
			listusers_admin(sockfd);
		else if(!comando.compare("\\deleteaccount"))
			deleteaccount_c(sockfd,argumentos);
		else if(!comando.compare("\\setaskusers"))
			setaskusers_c(sockfd,argumentos);
		else if(!comando.compare("\\answer"))
			answer_c(sockfd, argumentos);
		else if(!comando.compare("\\exit")) {
			cout << "Client disconnected: " << sockfd << endl;
			clients.erase(sockfd);
	
			/* Fechar o socket */
			close(sockfd);
		}
		else if(!comando.compare("\\shutdown"))
			shutdown_c(sockfd);
		else
			writeline(sockfd, "Comando inválido! Ver Manual!\n");

	}
	
	cout << "Socket " << sockfd << " said: " << line << endl;
    //broadcast(sockfd, line);
  }

  cout << "Client disconnected: " << sockfd << endl;
  clients.erase(sockfd);
  
  /* Fechar o socket */
  close(sockfd);
}

/*****				MAIN				*****/
int main(int argc, char *argv[])
{
  /* Estruturas de dados */
  int sockfd, newsockfd, port = 6969;
  socklen_t client_addr_length;
  struct sockaddr_in serv_addr, cli_addr;
  system("clear");
  cout << "Servidor de \"Quem não quer ser miseravelmente pobre?\"" << endl << endl;
  
  initDB();
  
  /*************************************************************************/
  /* Inicializar o socket
     AF_INET - para indicar que queremos usar IP
     SOCK_STREAM - para indicar que queremos usar TCP
     sockfd - id do socket principal do servidor
     Se retornar < 0 ocorreu um erro */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    cout << "Error creating socket" << endl;
     exit(-1);
  }

  /* Criar a estrutura que guarda o endereço do servidor
     bzero - apaga todos os dados da estrutura (coloca a 0's)
     AF_INET - endereço IP
     INADDR_ANY - aceitar pedidos para qualquer IP */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  /* Fazer bind do socket. Apenas nesta altura é que o socket fica ativo
     mas ainda não estamos a tentar receber ligações.
     Se retornar < 0 ocorreu um erro */
  int res = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
  if (res < 0) {
    cout << "Error binding to socket" << endl;
    exit(-1);
  }

  /* Indicar que queremos escutar no socket com um backlog de 5 (podem
     ficar até 10 ligações pendentes antes de fazermos accept */
     
  /*!!!O nº do socket está a começar no 5 sem razão aparente. Analisar situação!!!*/
  listen(sockfd, 10);

  while(true) {
    /* Aceitar uma nova ligação. O endereço do cliente fica guardado em 
       cli_addr - endereço do cliente
       newsockfd - id do socket que comunica com este cliente */
    client_addr_length = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client_addr_length);

    /* Criar uma thread para tratar dos pedidos do novo cliente */
    pthread_t thread;
    pthread_create(&thread, NULL, cliente, &newsockfd);
    
    // Desliga o servidor através do próprio
	string in;
	
	//cin >> in;
	//if(!in.compare("shutdown"))
	//	break;
  }

  closeDB();
  close(sockfd);
  return 0; 
  
}
