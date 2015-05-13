#include <map>

#include "command_lib.h"

#define ADMIN_PERM 0

map<string, int> sockets;
map<int, string> usernames;
map<string, int> jogo_criado;
map<string, bool> waitingForAnswer;
map<string, int> currAnswer;

string intToString(int i) {
        ostringstream oss;
        oss << i;
        return oss.str();
}

/***********	jogo	**********/

void* jogo(void * args)
{
	int game_id = *(int*)args;
	bool player1Presente = false;
	bool player2Presente = false;

	cout << "A ir buscar as perguntas. Id do jogo: " << game_id << endl;

	// Ir buscar à BD os dados do jogo
	string query = "SELECT nperguntas, duracao, criador, player1, player2 FROM jogo WHERE id = '" + intToString(game_id) + "';";
	
	cout << query << endl;
	
	PGresult* res = executeSQL(query);
	
	int nquestoes 	 = atoi(PQgetvalue(res, 0, 0));
	int duracao 	 = atoi(PQgetvalue(res, 0, 1));
	string criador	 = PQgetvalue(res, 0, 2);
	string player1	 = PQgetvalue(res, 0, 3);
	string player2	 = PQgetvalue(res, 0, 4);
	
	int questoes[nquestoes];
	
	// Jogadores estão bloqueados
	cout << "Jogo inciado" << endl;
	
	writeline(sockets[criador], "Jogo Iniciado. Boa sorte!!\n");
	
	if(player1 != "") {
		writeline(sockets[player1], "Jogo Iniciado. Boa sorte!!\n");
		player1Presente = true;
	}
		
	if(player2 != "") {
		writeline(sockets[player2], "Jogo Iniciado. Boa sorte!!\n");
		player2Presente = true;	
	}

	// Seleccionar as perguntas para o jogo
	PGresult* result = executeSQL("SELECT id FROM perguntas ORDER BY random() LIMIT " + intToString(nquestoes));
	
	for(int linha=0; linha < nquestoes; linha++) {
	
        questoes[linha] = atoi(PQgetvalue(result, linha, 0));
	}

	cout << "Questões obtidas" << endl;

	for(int i=0; i < nquestoes; i++) {
		
		string pergunta, respostas[4];
		vector<int> ordem;
		
		for (int n=0; n<4; ++n) ordem.push_back(n);
		
		// Definir a ordem aleatória das respostas
		random_shuffle(ordem.begin(), ordem.end());
		
		cout << "Respostas baralhadas" << endl;
		
		// Imprimir as perguntas nos terminais dos jogadores
		string query = "SELECT questao, respcerta, resperrada1, resperrada2, resperrada3 FROM perguntas WHERE id = '" + intToString(questoes[i]);
		PGresult* res = executeSQL(query + "'; ");
		
		cout << "Dados da pergunta obtidos" << endl;
		
		pergunta = PQgetvalue(res, 0, 0);
		respostas[0] = PQgetvalue(res, 0, 1);
		respostas[1] = PQgetvalue(res, 0, 2);
		respostas[2] = PQgetvalue(res, 0, 3);
		respostas[3] = PQgetvalue(res, 0, 4);
		
		cout << "Parse dos dados feito" << endl;
		
		string questao = intToString(i+1) + "." + pergunta + "\nA: " + respostas[ordem[0]] + "\nB: " + respostas[ordem[1]] + "\nC: " + respostas[ordem[2]] + "\nD: " + respostas[ordem[3]] + "\n\n";
		
		// Imprime a questão no terminal dos jogadores
		writeline( sockets[criador], questao);
		waitingForAnswer[criador] = true;	// Assinala que está à espera de uma resposta
		
		if(player1Presente) {
			writeline( sockets[player1], questao);
			waitingForAnswer[player1] = true;	// Assinala que está à espera de uma resposta
		}
		if(player2Presente) {
			writeline( sockets[player2], questao);
			waitingForAnswer[player2] = true;	// Assinala que está à espera de uma resposta
		}		
		
		// FlagWaitingForAwnser = true;
		/*
			Aqui deverá ter uma forma de a função "\answser" verificar que está a ser aguardada uma resposta do utilizador.
			Esta resposta deverá ser guardada numa qualquer variável que depois será lida após ter terminado o tempo.
		*/
				
		cout << "A entrar em espera" << endl;
		
		// Esperar o tempo da questão
		cout.flush();
		sleep(duracao);
		
		writeline( sockets[criador], "Terminou o tempo!");
		waitingForAnswer[criador] = false;	// Assinala que está à espera de uma resposta
		
		if(player1Presente) {
			writeline( sockets[player1], "Terminou o tempo!");
			waitingForAnswer[player1] = false;	// Assinala que está à espera de uma resposta
		}
		if(player2Presente) {
			writeline( sockets[player2], "Terminou o tempo!");
			waitingForAnswer[player2] = false;	// Assinala que está à espera de uma resposta
		}
		
		// Espera até todos os jogadores estaarem prontos ou ter passado x segundos
		clock_t questionEnd_time = clock();
		double  elapsedTime;
		
		do {
			clock_t curTime = clock();
			clock_t clockTicksTaken = curTime - questionEnd_time;
			elapsedTime = clockTicksTaken / (double) CLOCKS_PER_SEC;
		} while(elapsedTime < TIME_BETWEEN_QUESTIONS /* || Todos "ready" */);
		
		// Verificar as respostas dos utilizadores
		/*
			...
		*/
		
		
		
		
	}
	
	// O jogo terminou
	writeline( sockets[criador], "O jogo terminou!");
	
	if(player1Presente) {
		writeline( sockets[player1], "O jogo terminou!");
	}
	if(player2Presente) {
		writeline( sockets[player2], "O jogo terminou!");
	}
	
	/***
		Retirar do map o id do jogo associado ao seu criador
	***/
}

/***			end jogo			***/



/* Envia uma string para um socket */
void writeline(int socketfd, string line) {
	string tosend = line + "\n";
	write(socketfd, tosend.c_str(), tosend.length());
}

/**
*Responder a uma questão
*	Verifica no map waitingForAnswer se é suposto o jogador que executou o comando responder.
*	Guarda no map currAnswer o inteiro correspondente à resposta do jogador: A=0; B=1; ...
*
*/
void answer_c(int socketid, string args)
{
	istringstream iss(args);
    string resposta;
	
	getline(iss, resposta, ' ');
	
	// Normalizar a opção para letra maiuscúla
	transform(resposta.begin(), resposta.end(), resposta.begin(), ::toupper);
	
	// Verificar se o cliente está num jogo e está a ser aguardada uma resposta
	if (waitingForAnswer.find(usernames[socketid]) != waitingForAnswer.end()) {
		writeline(socketid, "Comando inválido.\n Não é suposto responder a nada agora.\n");
		return;
	}
	
	// Verificar se a resposta é válida
	if(resposta != "A" && resposta != "B" && resposta != "C" && resposta != "D") {
		writeline(socketid, "Resposta inválida.\n Por favor seleccione a letra correspondente à opção.\n");
		return;
	}	
	
	// Guardar a opção seleccionada pelo jogado na variável destinada a este efeito (qual?)
	if( resposta == "A")	currAnswer[usernames[socketid]] = 0;
	else if( resposta == "B")	currAnswer[usernames[socketid]] = 1;
	else if( resposta == "C")	currAnswer[usernames[socketid]] = 2;
	else if( resposta == "D")	currAnswer[usernames[socketid]] = 3;

	// Assinalar que o jogador respondeu à resposta e Sair
	waitingForAnswer[usernames[socketid]] = false;
	return;
}

void help_c(int socketid)
{
	ifstream ifs;
	ostringstream out_string;
	
	ifs.open ("help.txt", ifstream::in);
	
	if((ifs.rdstate() & ifstream::failbit)!=0) {
		cout<<"Ficheiro de ajuda não encontrado";
		writeline(socketid, "Ocorreu um erro. Não foi possível executar o comando.\n");
	}
	
	char c = ifs.get();
	writeline(socketid, "\n");
	while (ifs.good()) {
    	out_string << c;
    	c = ifs.get();
	}

	ifs.close();

	writeline(socketid, out_string.str());
}

void register_c(int socketid, string args)
{
	istringstream iss(args);
	string user, pass, falha;
	
	getline(iss, user, ' ');
	getline(iss, pass, ' ');
	getline(iss, falha, ' ');
	
	if(falha!="\0")
	{
		writeline(socketid, "Introduziu elementos a mais.A utilizacao de espacos nao e possivel.\n");
		return;
	}		
	if(user =="\0" || pass=="\0")
	{
		writeline(socketid, "Introduziu elementos a menos.\n");
		return;
	}		
	
	if(islogged(socketid)) {
		writeline(socketid, "Já se encontra online! Por favor, faça logout antes de criar uma conta.\n");
		return;
	}		
	
	if (user.length() > 32 || user.length() < 4 || !alphanumeric(user)) 
	{
		writeline(socketid, "O seu username está mal forumulado\n");
		return;
	}
	
	if (pass.length() > 64 || pass.length() < 4 || !alphanumeric(pass)) 
	{
		writeline(socketid, "A sua password está mal forumulada\n");
		return;
	}
	
	if(userexists(user)) {
		writeline(socketid, "O username seleccionado não se encontra disponível!\nPor favor, tente novamente.\n");
		return;
	}
	
	executeSQL("INSERT INTO utilizador VALUES ('" + user + "', '" + pass + "', 1)");
	executeSQL("INSERT INTO ajudautilizadores(id, useremjogo) VALUES  (DEFAULT, '"+user+"')");
	writeline(socketid, "A conta foi criada com successo!\n");
	
	login_c(socketid, args);
}

void identify_c(int socketid, string args)
{
	istringstream iss(args);
	string user, falha;
	
	getline(iss, user, ' ');
	getline(iss, falha, ' ');
	
	if(falha!="\0")
	{
		writeline(socketid, "Introduziu elementos a mais.\n");
		return;
	}		
	if(user =="\0")
	{
		writeline(socketid, "Introduziu elementos a menos.\n");
		return;
	}		
	
	string query = "SELECT (username) FROM utilizador WHERE username='" + user + "';";
	
	// Verifica se o user existe na BD
	PGresult* result = executeSQL(query);
	
	if(PQntuples(result) > 0) 
		writeline(socketid, "\nO user " + user + " existe!");		
	else 
		writeline(socketid, "\nO user " + user + " existe!");	
}

void login_c(int socketid, string args)
{
	istringstream iss(args);
	string user, pass, falha;
	

	getline(iss, user, ' ');
	getline(iss, pass, ' ');
	getline(iss, falha, ' ');
	
	if(falha!="\0")
	{
		writeline(socketid, "Introduziu elementos a mais.\n");
		return;
	}		
	if(user =="\0" || pass=="\0")
	{
		writeline(socketid, "Introduziu elementos a menos.\n");
		return;
	}	

	if (sockets.find(user) != sockets.end())//|| islogged(sockets[user])) 
	{
		writeline(socketid, "Já se encontra ligado, por favor faça logout e tente novamente.\n");
		return;
	}
	
	string query = "SELECT (username, password) FROM utilizador WHERE username='" + user + "' AND password='" + pass + "';";
	
	// Verifica se as credenciais estão na base de dados
	PGresult* result = executeSQL(query);
	
	if(PQntuples(result) > 0) {
		sockets[user] = socketid;
		usernames[socketid] = user;
		writeline(socketid, "\nBem-vindo " + user + "!");		
	}
	else 
	{
		writeline(socketid, "Os dados de login não estão correctos!\n Tente novamente.");
		return;
	}

	//listusers(socketid);
}

void logout_c(int socketid)
{
	if (usernames.find(socketid) != usernames.end()) {
		
		sockets.erase(usernames[socketid]);
		usernames.erase(socketid);
	
		writeline(socketid, "Efectuou logout com successo!\n");

		//listusers(socketid);
					
		return;
	}
	else {
		writeline(socketid, "Não encontra ligado, por favor faça login.\n");	
	}
}

void resetpassword_c(int socketid, string args)
{
	if(islogged(socketid))
	{
				istringstream iss(args);
				string user, newpass, query, falha;
				

				getline(iss, user, ' ');
				getline(iss, newpass, ' ');
				getline(iss, falha, ' ');
				
				if(falha!="\0")
				{
					writeline(socketid, "Introduziu elementos a mais.\n");
					return;
				}		
				if(user =="\0" || newpass=="\0")
				{
					writeline(socketid, "Introduziu elementos a menos.\n");
					return;
				}	
				
				// Verifica se o utlizador que executou o comando é administrador
				if(isadmin(socketid)) {
					writeline(socketid, "Este comando é reservado a administradores.\n");	
                    cout <<"User" + usernames[socketid] +" tentou fazer reset a uma password!\n"<<endl;
					return;
				}
				
				// Verifica se o username existe na base de dados
				if(!userexists(user)) {
					writeline(socketid, "O username fornecido não foi encontrado.\n");	
					return;
				}
				
				else if (newpass.length() < 4) {
					writeline(socketid, "A sua password tem poucos caracteres. Minimo de caracteres: 4\n");
					return;
				}
				else if (newpass.length() > 64) {
					writeline(socketid, "A sua password tem muitos caracteres. Máximo de caracteres: 64\n");
					return;
				}
				
				query = "UPDATE utilizador SET password = '" + newpass + "' WHERE username = '" + user + "';";
					
				PGresult* result = executeSQL(query);	

				writeline(socketid, "A password do utilizador foi alterada com sucesso.\n");	
	}
	else 
		writeline(socketid,"Permissão negada! Não se encontra logado");
	
	}

void changepassword_c(int socketid, string args)
{
	istringstream iss(args);
	string old, newp, confirm, user, falha;
	
	getline(iss, old, ' ');
	getline(iss, newp, ' ');
	getline(iss, confirm, ' ');
	getline(iss, falha, ' ');
	
	if(!islogged(socketid)) {
		writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
		return;
	}		
	
	if(falha!="\0")
	{
		writeline(socketid, "Introduziu elementos a mais.\n");
		return;
	}		
	if(old =="\0" || newp=="\0" || confirm=="\0")
	{
		writeline(socketid, "Introduziu elementos a menos.\n");
		return;
	}
	if (newp.length() > 64 || newp.length() < 4 || !alphanumeric(newp)) 
	{
		writeline(socketid, "A sua password está mal forumulada\n");
		return;
	}
	user=usernames[socketid];
	
	string query = "SELECT (username, password) FROM utilizador WHERE username='" + user + "' AND password='" + old + "';";
	PGresult* result = executeSQL(query);
	
	if(PQntuples(result) != 0) {
		if(newp.compare(confirm))
			writeline(socketid, "A confirmação da password nao coincide");

		else
		{
			query = "UPDATE utilizador SET password = '" + newp + "' WHERE username = '" + user + "';";
			result = executeSQL(query);
			writeline(socketid, "Mudança bem sucedida");	
		}
	}
	else 
		writeline(socketid, "Password antiga nao está incorreta!");

}

void changeusername_c(int socketid, string args)
{
	
				istringstream iss(args);
				string newuser, olduser, falha;
					
				getline(iss, newuser, ' ');
				getline(iss, falha, ' ');
				
				if (newuser.length() > 64 || newuser.length() < 4 || !alphanumeric(newuser)) 
				{
					writeline(socketid, "O novo username está mal forumulado\n");
					return;
				}
				
				if(!islogged(socketid)) {
					writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
					return;
				}		
				
				if(falha!="\0")
				{
					writeline(socketid, "Introduziu elementos a mais.\n");
					return;
				}		
				if(newuser =="\0")
				{
					writeline(socketid, "Introduziu elementos a menos.\n");
					return;
				}	
				
				olduser=usernames[socketid];
				
				string query = "SELECT (username) FROM utilizador WHERE username='" + olduser+"';";
				cout<<query<<endl;
				PGresult* result = executeSQL(query);
				cout<<PQntuples(result);
				if(PQntuples(result) != 0) 
				{
					query = "UPDATE utilizador SET username = '" + newuser + "' WHERE username = '" + olduser + "';";
						cout<<query<<endl;
					result = executeSQL(query);
					writeline(socketid, "Mudança bem sucedida! Saudações, "+ newuser);	
				}
				else 
					writeline(socketid, "Já existe um utilizador com esse username!");
					

}

void say_c(int socketid, string args)
{

    istringstream iss(args);
    string user, mensagem, palavra, falha, remetente;

    getline(iss, user, ' ');
    getline(iss, falha, ' ');

    if(!islogged(sockets[user]))
    {
        writeline(socketid, "Erro, o utilizador "+user+"não está online neste momento!\n");
        return;
    }

    if(falha!="\0")
    {
        writeline(socketid, "Introduziu elementos a mais.\n");
        return;
    }
    if(user =="\0")
    {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }

    while (iss >> palavra)
        mensagem += palavra + ' ';

   /* if(!islogged(socketid))
    {
        writeline(socketid, "Qual o seu nome?\n");
        readline(socketid, remetente);
        writeline(socketid, "O utilizador não registado: "+remetente+" enviou lhe a seguinte mensagem:\n");
        writeline(sockets[user], mensagem);
        return;
    }*/
    writeline(socketid, "O utilizador registado: "+usernames[socketid]+" enviou lhe a seguinte mensagem:\n");
    writeline(sockets[user], mensagem);
}

/*
Insere uma nova questão na base de dados

NAO E PRECISO USAR UNDERSCORES-----> GENIUS xD <----- 

ok:
	0 : ok
	-1: argumentos a mais
	-2: argumentos a menos
	-3: outro erro
*/

void question_c(int socketid, string args)
{
    if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    string pergunta, respcerta, resperrada1, resperrada2, resperrada3, falha, user;
    istringstream iss(args);
    int ok=0;

    user=usernames[socketid];

    getline(iss, pergunta, '|');
    getline(iss, respcerta, '|');
    getline(iss, resperrada1, '|');
    getline(iss, resperrada2, '|');
    getline(iss, resperrada3, '|');
    getline(iss, falha, '|');

    if(falha!="\0")
        ok=-1;
    if (pergunta=="\0" || resperrada1=="\0"||resperrada2=="\0"||resperrada3=="\0"||respcerta=="\0")
        ok=-2;

    if(ok==-1)
        writeline(socketid, "ERRO: Introduziu argumentos a mais");
    else if(ok==-2)
        writeline(socketid, "ERRO: Introduziu argumentos a menos");
    else if(ok==0)
    {
        executeSQL("INSERT INTO perguntas  VALUES  (DEFAULT, '" + pergunta + "', '" + respcerta + "', '" + resperrada1 + "', '" + resperrada2 + "', '" + resperrada3 + "', '" + user + "')");
        writeline(socketid, "Pergunta inserida com sucesso!");
    }
}

/*
\showallquestions
Permite aos administradores ver uma tabela com todas as perguntas com respetivo id e respostas.

*/
void showallquestions_c(int socketid)
{
	
    if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    string user=usernames[socketid];

    if (isadmin(socketid)!=0 )
    {
        cout <<"User" + usernames[socketid] +" tentou ver todas as questoes!\n"<<endl;
        writeline(socketid,"Não tem permissões para usar esse comando!");
        return;
    }

    string sql;

    PGresult* res2 = executeSQL("SELECT (id, questao) FROM perguntas");
    cout<<"ID\t\tPERGUNTA\n"<<endl;
    for (int row = 0; row < PQntuples(res2); row++)
    {
        sql=PQgetvalue(res2, row, 0);
        cout<<sql<<endl;
    }
}


/*
(1: questão; 2:resposta certa; 3:2ª resposta; 4:3ª resposta; 5: última resposta). 
JA NAO E PRECISO-----Depois insere-se o texto, os espaços utilizados nos parâmetros têm de ser substituídos por underscores ( _ ).
*/

void editquestion_c(int socketid, string args)
{
    string user=usernames[socketid], id, p, texto, falha;
    istringstream iss(args);
    int parametro;

    if (isadmin(socketid)!=0 )
    {
        cout <<"User" + usernames[socketid] +" tentou editar questoes!\n"<<endl;
        writeline(socketid,"Não tem permissões para usar esse comando!");
        return;
    }

    getline(iss, id, ' ');
    getline(iss, p, ' ');
    getline(iss, texto, ' ');
    getline(iss, falha, ' ');

    if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }
    if(falha!="\0")
    {
        writeline(socketid, "Introduziu elementos a mais.\n");
        return;
    }
    if(id =="\0" || p=="\0" || texto=="\0")
    {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }

    parametro=atoi(p.c_str());

    PGresult* result = executeSQL("SELECT * FROM perguntas WHERE id=" + id +";");

    if(PQntuples(result)==0)
        writeline(socketid, "ERRO: ID da pergunta incorreto!");

    else
    {
        if(parametro==1){
            PGresult* res = executeSQL("UPDATE perguntas SET questao = '" + texto + "' WHERE id = " + id + ";");
            writeline(socketid, "Parametro alterado com sucesso!");
        }
        else if(parametro==2){
            PGresult* res = executeSQL("UPDATE perguntas SET respcerta = '" + texto + "' WHERE id = " + id + ";");
            writeline(socketid, "Parametro alterado com sucesso!");
        }
        else if(parametro==3){
            PGresult* res = executeSQL("UPDATE perguntas SET resperrada1 = '" + texto + "' WHERE id = " + id + ";");
            writeline(socketid, "Parametro alterado com sucesso!");
        }
        else if(parametro==4){
            PGresult* res = executeSQL("UPDATE perguntas SET resperrada2 = '" + texto + "' WHERE id = " + id + ";");
            writeline(socketid, "Parametro alterado com sucesso!");
        }
        else if(parametro==5){
            PGresult* res = executeSQL("UPDATE perguntas SET resperrada3 = '" + texto + "' WHERE id = " + id + ";");
            writeline(socketid, "Parametro alterado com sucesso!");
        }
        else
            writeline(socketid, "ERRO: Parametro incorreto!");
    }
}


/*
\deletequestion <id>
Permite aos administradores apagar perguntas. Para isso é necessário indicar o “id” da pergunta que pode ser obtido através do comando “showallquestions”.
*/

void deletequestion_c(int socketid, string args)
{
    string user=usernames[socketid], id, falha;
    istringstream iss(args);

    if (isadmin(socketid)!=0 )
    {
        cout <<"User" + usernames[socketid] +" tentou apagar perguntas"<<endl;
        writeline(socketid,"Não tem permissões para usar esse comando!");
        return;
    }

    getline(iss, id, ' ');
    getline(iss, falha, ' ');

    if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }
    if(falha!="\0")
    {
        writeline(socketid, "Introduziu elementos a mais.\n");
        return;
    }
    if(id =="\0")
    {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }

    PGresult* result = executeSQL("SELECT * FROM perguntas WHERE id= " + id +";");

    if(PQntuples(result)==0)
        writeline(socketid, "ERRO: ID da pergunta incorreto!");

    else
    {
        PGresult* res = executeSQL("DELETE FROM perguntas WHERE id= " + id + ";");
        writeline(socketid, "Pergunta eliminada com sucesso!");
    }
}


void changepermissions_c(int socketid, string args)
{

    if (isadmin(socketid)!=0 )
    {
        cout <<"User" + usernames[socketid] +" tentou mudar permissoes"<<endl;
        writeline(socketid,"Não tem permissões para usar esse comando!");
        return;
    }
    istringstream iss(args);
    string user, permissao, falha;

    getline(iss, user, ' ');
    getline(iss, permissao, ' ');
    getline(iss, falha, ' ');

    if(user==usernames[socketid])
    {
        writeline(socketid, "Não pode mudar as suas permissoes");
        return;
    }
    if(!islogged(socketid))
    {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    if(falha!="\0")
    {
        writeline(socketid, "Introduziu elementos a mais.\n");
        return;
    }
    if(user=="\0" || permissao=="\0")
    {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }

    string query = "SELECT (username) FROM utilizador WHERE username='" + user + "';";
    PGresult* result = executeSQL(query);

    if(atoi(permissao.c_str())!=0 && atoi(permissao.c_str())!=1)
        writeline(socketid, "Inseriu um valor de perissao errado. 0-admin, 1-user normal");
    else if(PQntuples(result) == 0)
        writeline(socketid, "Inseriu um utilizador invalido!");
    else
    {
        query = "UPDATE utilizador SET permissao = " + permissao + " WHERE username = '" + user + "';";
        result = executeSQL(query);
        writeline(socketid, "Mudança bem sucedida");
    }

}

void start_c(int socketid, string args) {
	
	if(!islogged(socketid)) {
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
		return;
	}
	
	if (!(jogo_criado.find(usernames[socketid]) != jogo_criado.end())) {
		writeline(socketid, "Não tem um jogo criado!\nPor favor, crie um jogo.");
		return;
	}
	
	cout << "A entrar na tarefa jogo\n" << endl;
	
	pthread_t gamethread;
	pthread_create(&gamethread, NULL, jogo, &jogo_criado[usernames[socketid]]);	
}

/**

*	Cria um jogo na BD e grava no map jogos_criados o id do jogo associado ao username do criador.

*/
void create_c(int socketid, string args)
{
    istringstream iss(args);
    string tempo, questoes, query, falha;

    getline(iss, tempo, ' ');
    getline(iss, questoes, ' ');
    getline(iss, falha, ' ');

    if(falha!="\0")
    {
        writeline(socketid, "Introduziu elementos a mais.\n");
        return;
    }
    if(questoes=="\0" || tempo=="\0")
    {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }


	if(!islogged(socketid)) {
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
		return;
	}
	
	if (jogo_criado.find(usernames[socketid]) != jogo_criado.end()) {
		writeline(socketid, "Já tem um jogo criado!\nPor favor, cancele o jogo anterior antes de criar um novo.");
		return;
	}
	
    if(tempo == "\0" || questoes == "\0" || atoi(questoes.c_str())<1 || atoi(questoes.c_str())>20 || atoi(tempo.c_str())<10 || atoi(tempo.c_str())>60 )
	{
		writeline(socketid, "Parâmetros incorrectos.\n");
		return;
	}		
	
	query = "INSERT INTO jogo VALUES  (DEFAULT, " + questoes + " , " + tempo + " , '" + usernames[socketid] + "'); SELECT currval('id_jogo_seq');"; //ter data:CURRENT_TIMESTAMP(2)
	
	PGresult* result = executeSQL(query);

	int game_id = atoi(PQgetvalue(result, 0, 0));

	jogo_criado[usernames[socketid]] = game_id;

	writeline(socketid, "Jogo criado com sucesso! ID do jogo: " + intToString(game_id));
}

/**

*	INCOMPLETO

*

*	Param: 	user - user a convidar

*			id - id do jogo

*/

void challenge_c(int socketid, string args)
{
	bool criador = false;
	istringstream iss(args);
	string id, user, tempo, questoes, query, desafiador, falha;
	int i;
	
	getline(iss, user, ' ');
   getline(iss, falha, ' ');
	
	 if(!islogged(socketid))
    {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    if(falha!="\0")
    {
        writeline(socketid, "Introduziu elementos a mais.\n");
        return;
    }
    if(user=="\0")
    {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }
	
	i=jogo_criado[usernames[socketid]];
	id=intToString(i);
	//cout<<endl<<id<<endl<<intToString(i)<<endl<<endl;
	
	query="SELECT (criador) FROM jogo WHERE id=" + id + ";";
	//cout<<query<<endl;
	PGresult* res = executeSQL(query);
	
	if(PQntuples(res)!=0 || isadmin(socketid)==0)
		criador = true;
	if(!criador)
	{
		writeline(socketid, "ERRO: Não é o criador do jogo");
		return;
	}
	desafiador=usernames[socketid];

	if(!userexists(user))
		writeline(socketid, "O username selecionado não existe!\n");
	else if(!user.compare(desafiador))
		writeline(socketid, "Não se pode desafiar a si mesmo!\n");
	else if(!islogged(sockets[user]))
		writeline(socketid, "O user especificado não se encontra online. Tente mais tarde!\n");
	else
		writeline(sockets[user],"O jogador " + usernames[socketid] + " convidou-o para iniciar um jogo.\n Para aceitar escreva: \\accept, para rejeitar escreva: \\decline\n");

	// FlagWaitingForAccept = true;
	
	query="SELECT convidado1 FROM jogo WHERE id="+id+";";
	//cout<<query<<endl;
	
	PGresult* res1 = executeSQL("SELECT convidado1 FROM jogo WHERE id="+id+";");
	string c1 = PQgetvalue(res1, 0, 0);
	if(c1=="")
		executeSQL("UPDATE jogo SET convidado1 = '" + user + "' WHERE id = " + id + ";");	
	else
	{
		PGresult* res2 = executeSQL("SELECT convidado2 FROM jogo WHERE id="+id+";");
		string c2 = PQgetvalue(res2, 0, 0);
		if(c2=="")
			executeSQL("UPDATE jogo SET convidado2 = '" + user + "' WHERE id = " + id + ";");
		else
		{
			PGresult* res3 = executeSQL("SELECT convidado3 FROM jogo WHERE id="+id+";");
			string c3 = PQgetvalue(res3, 0, 0);
			if(c3=="")
				executeSQL("UPDATE jogo SET convidado3 = '" + user + "' WHERE id = " + id + ";");
			else
			{
             PGresult* res4 = executeSQL("SELECT convidado4 FROM jogo WHERE id="+id+";");
             string c4 = PQgetvalue(res4, 0, 0);
             if(c4=="")
                 executeSQL("UPDATE jogo SET convidado4 = '" + user + "' WHERE id = " + id + ";");
             else
                 writeline(socketid, "Já convidou 4 jogadores!\n");
         }
		}
	}
}

/**
*	args : Nome do utilizador que fez o convite
*
*/
void accept_c(int socketid, string args)
{
	istringstream iss(args);
    string user, id, falha;
    int i;

    getline(iss, user, ' ');
    getline(iss, falha, ' ');

    if(!islogged(socketid))
    {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    if(falha!="\0")
    {
        writeline(socketid, "Introduziu elementos a mais.\n");
        return;
    }
    if(user=="\0")
    {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }

	i=jogo_criado[user];
	id=intToString(i);
	//cout<<endl<<id<<endl;
	// Ver se o id do jogo é válido
	if ((jogo_criado.find(id) != jogo_criado.end())) {
		writeline(socketid, "O jogo indicado não se encotra disponível\n");
		return;
	}	
	
	if(!userexists(user)) {
		writeline(socketid, "O nome do utilizador não está correcto.\n");
		return;
	}
		
	if (!(jogo_criado.find(user) != jogo_criado.end())) {
		writeline(socketid, "Esse utilizador não criou nenhum jogo.\n");
		return;
	}
	
	// Ver se o jogo já começou
	PGresult* res1 = executeSQL("SELECT dataehora FROM jogo WHERE id="+id+";");
	string timestart = PQgetvalue(res1, 0, 0);
	
	cout<<endl<<"TEMPO:" + timestart<<endl;
	
	if(timestart != "")
	{
		writeline(socketid, "O jogo já começou! Não é mais possivel aceitar o convite\n");
		return;
	}
	
	res1 = executeSQL("SELECT convidado1, convidado2, convidado3, convidado4 FROM jogo WHERE id=" + id + ";");
	
	string c1 = PQgetvalue(res1, 0, 0);
	string c2 = PQgetvalue(res1, 0, 1);
	string c3 = PQgetvalue(res1, 0, 2);
	string c4 = PQgetvalue(res1, 0, 3);
	
	cout << endl<<c1<<endl<<c2<<endl<<c3<<endl<<c4<<endl<<c1.compare(usernames[socketid]) << endl << c2.compare(usernames[socketid]) << endl << c3.compare(usernames[socketid]) << endl << c4.compare(usernames[socketid]) << endl; 

	/***

		Que se passa aqui?

	***/
	if(c1.compare(usernames[socketid]) && c2.compare(usernames[socketid]) && c3.compare(usernames[socketid]) && c4.compare(usernames[socketid])) 
	{
		writeline(socketid, "Não foi convidado para este jogo\n");
		return;
	}
	
	res1 = executeSQL("SELECT player1 FROM jogo WHERE id=" + id + ";");
	cout<<"query:"<<"SELECT player1 FROM jogo WHERE id=" + id + ";";
	c1 = PQgetvalue(res1, 0, 0);
	
	if(c1=="") {
		executeSQL("UPDATE jogo SET player1 = '" + usernames[socketid] + "' WHERE id = " + id + ";");
		writeline(socketid, "Tudo Pronto! Espere pelo inicio do jogo!\n");
		return;
	}
	else {
		res1 = executeSQL("SELECT player2 FROM jogo WHERE id="+id+";");
		c2= PQgetvalue(res1, 0, 0);
		if(c2=="")
		{
			executeSQL("UPDATE jogo SET player2 = '" + usernames[socketid] + "' WHERE id = " + id + ";");
			writeline(socketid, "Tudo Pronto! Espere pelo inicio do jogo!\n");
			return;
		}
		else
		{
			writeline(socketid, "Vagas para o Jogo preenchidas!\n");
			return;
		}
	}
	
	/*

		Verificar se o jogador "user" convidou o este jogador para o jogo.

	*/
	
	// Ver se o player1 da tabelo jogo é NULL
	
	// Se for pôr lá o username do utilizador do socketid
	
	// Senão ver se o player2 da tabelo jogo é NULL
	
	// Se for pôr lá o username do utilizador do socketid
	
	// Senão dizer que ocorreu um erro ou que já está cheio e sair
}



//*****************************
//VER ESTA FUNCAO..NAO SEI SE DA ASSIM
//*****************************
void decline_c(int socketid, string args)
{
	istringstream iss(args);
    string user, id, falha;
    int i;

    getline(iss, user, ' ');
    getline(iss, falha, ' ');

		if(!islogged(socketid))
		{
			writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
			return;
		}

		if(falha!="\0")
		{
			writeline(socketid, "Introduziu elementos a mais.\n");
			return;
		}
		if(user=="\0")
		{
			writeline(socketid, "Introduziu elementos a menos.\n");
			return;
		}

			i=jogo_criado[user];
			id=intToString(i);
	
			if ((jogo_criado.find(id) != jogo_criado.end())) {
				writeline(socketid, "O jogo indicado não se encotra disponível\n");
				return;
			}	
	
			if(!userexists(user)) {
				writeline(socketid, "O nome do utilizador não está correcto.\n");
				return;
			}
		
			if (!(jogo_criado.find(user) != jogo_criado.end())) {
				writeline(socketid, "Esse utilizador não criou nenhum jogo.\n");
				return;
			}
	
				// Ver se o jogo já começou
				PGresult* res1 = executeSQL("SELECT dataehora FROM jogo WHERE id="+id+";");
				string timestart = PQgetvalue(res1, 0, 0);
	
				//cout<<endl<<"TEMPO:" + timestart<<endl;
	
				if(timestart != "")
				{
					writeline(socketid, "O jogo já começou! Não é mais possivel aceitar o convite\n");
					return;
				}
	
				res1 = executeSQL("SELECT convidado1, convidado2, convidado3, convidado4 FROM jogo WHERE id=" + id + ";");
				
				string c1 = PQgetvalue(res1, 0, 0);
				string c2 = PQgetvalue(res1, 0, 1);
				string c3 = PQgetvalue(res1, 0, 2);
				string c4 = PQgetvalue(res1, 0, 3);
	
				//cout << endl<<c1<<endl<<c2<<endl<<c3<<endl<<c4<<endl<<c1.compare(usernames[socketid]) << endl << c2.compare(usernames[socketid]) << endl << c3.compare(usernames[socketid]) << endl << c4.compare(usernames[socketid]) << endl; 

	/***

		Que se passa aqui?

	***/
				if(c1.compare(usernames[socketid]) && c2.compare(usernames[socketid]) && c3.compare(usernames[socketid]) && c4.compare(usernames[socketid])) 
				{
					writeline(socketid, "Não foi convidado para este jogo\n");
					return;
				}
	
				else{
				writeline(socketid, "Recusou jogar este jogo!\n\n\n");
				writeline(sockets[user], "O utilizador '"+ usernames[socketid] +"' não aceitou jogar no seu jogo!.\n");
				return;
		}
}


void usersready_c(int socketid, string args)
{
	istringstream iss(args);
	string id;
	
	iss >> id;
	
	if(!islogged(socketid)) {
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
		return;
	}
	
	//****************************
	//VER SE O ID DO JOGO E VALIDO
	//****************************

	PGresult* res1 = executeSQL("SELECT dataehora FROM jogo WHERE id="+id+";");
	string c1 = PQgetvalue(res1, 0, 0);
	if(c1!="")
	{
		writeline(socketid, "O jogo já começou!\n");
		return;
	}
	
	res1 = executeSQL("SELECT criador FROM jogo WHERE id="+id+";");
	c1 = PQgetvalue(res1, 0, 0);
	
	if(c1.compare(usernames[socketid]) || isadmin(socketid))
	{
		writeline(socketid, "Impossivel executar o comando, não é o criador do jogo!\n");
		return;
	}
	
	res1 = executeSQL("SELECT player1 FROM jogo WHERE id="+id+";");
	c1 = PQgetvalue(res1, 0, 0);
	res1 = executeSQL("SELECT player2 FROM jogo WHERE id="+id+";");
	string c2= PQgetvalue(res1, 0, 0);
	if(c1=="" && c2=="")
	{
		writeline(socketid, "Nenhum jogador aceitou o convite!\n");
		return;
	}
	
	else if(c1!="" && c2=="")
	{
		writeline(socketid, "1 Jogador pronto: "+c1+"!\n");
		return;
	}
	
	else if(c1=="" && c2!="")
	{
		writeline(socketid, "1 Jogador pronto: "+c2+"!\n");
		return;
	}
	
	else if(c1!="" && c2!="")
	{
		writeline(socketid, "2 Jogadores prontos: "+c1+", "+c2+"!\n");
		return;
	}
}



int alphanumeric(string str)
{
	for(int i=0; i < str.length(); i++)
	{	
		if(!isalnum(str[i]))
		{
			return 0;
		}
	}
	return 1;
}

void listusers(int socketid)
{
	stringstream aux;
	
	if(islogged(socketid)) {

	for (map<string,int>::iterator it=sockets.begin(); it!=sockets.end(); it++)
    	aux << "O utilizador " << it->first << " está logged in no socket " << it->second << endl;
		
	writeline(socketid, aux.str());	
	}
	
	else
				writeline(socketid, "Precisa de fazer login para executar o comando!\n");
}

/**
*	retorna TRUE se o utilizador do socket estiver "logado"
*/
bool islogged(int socketid)
{
	return usernames.find(socketid) != usernames.end();
}

/**
*	returns: 	 0 É Admin
*				 1 NÃO é Admin
*				-3 Erro: foram retornadas mais que uma linha da tabela
*				-2 Utilizanão não está atual online
*				-1 Utilizador não encontrado na base de dados;
*
*/
int isadmin(int socketid)
{
	if(!islogged(socketid))
		return -2;
	
	string user = usernames[socketid];
	
	string query = "SELECT permissao FROM utilizador WHERE username='" + user + "';";
	PGresult* result = executeSQL(query);
	
	//cout << "isadmin linhas:" << PQntuples(result) << " valor:"<< PQgetvalue(result, 0, 0) << endl;
	
	if(PQntuples(result) == 0)
		return -1;
	
	else if(PQntuples(result) > 1)
		return -3;
	
	else if(PQntuples(result) == 1) {
		if(atoi(PQgetvalue(result, 0, 0)) != 0)
			return 1;
		else 
			return 0;
	}		
}

bool userexists(string user)
{
	PGresult* result = executeSQL("SELECT username FROM utilizador WHERE username ILIKE '" + user + "'");

	if(PQntuples(result) > 0) 
		return true;
	else 
		return false;
}
				
		
	
	

void shutdown_c(int socketid)
{
	
	//Verifica se é admin
	if(islogged)
	{
		if (isadmin(socketid)!=0 )
		{ 
			cout <<"User não-Admin tentou usar \\shutdown"<<endl; 
			writeline(socketid,"Não tem permissões para usar esse comando!");
		}
		else if(isadmin(socketid)==0)
		{
			cout << "O servidor foi fechado pelo administrador :"<< usernames[socketid] << endl;
			close(mainsocket);
			exit(0);
		}
	}
	
	else 
		writeline(socketid,"Permissão negada! Não se encontra logado");	
}

/*
*/
void deleteaccount_c(int socketid, string args)
{
	if(islogged(socketid))
	{
		if(isadmin(socketid)==0)
		{
			string user, pass;
			istringstream iss(args);
			iss >> user >> pass;
			
				if (executeSQL("DELETE FROM utilizador WHERE (username = '"+user+"' AND password = '"+pass+"')") == NULL) 
					{
					  writeline(socketid, "\nErro a apagar o utilizador "+user+"'. Tente outra vez.");
					  return ;
					}
				
			
				
				writeline(socketid, "A conta '"+user+"' foi apagada.\n");
				banidoporadmin_c(sockets[user]);
				cout << "Utilizador apagado. Username: " << user << endl;
				executeSQL("DELETE FROM utilizador WHERE (username = '"+user+"')");
		}
		else if (isadmin(socketid)!=0 )
		{ 
			cout <<"User não-Admin tentou usar \\deleteaccount"<<endl; 
			writeline(socketid,"Não tem permissões para usar esse comando!");
		}
					
	}
	
	else
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
}

void banidoporadmin_c(int socketid)
{
	//serve para imprimir na funcao deleteaccount
	//if (usernames.find(socketid) != usernames.end()) {
		
		sockets.erase(usernames[socketid]);
		usernames.erase(socketid);
	
		writeline(socketid, "Foi banido por um dos administradores deste servidor!!!!!\n");

		//listusers(socketid);
					
		return;
	
}


void listusers_admin(int socketid)
{
	if(islogged(socketid)) {
	
					string user;
					int socket;
					for (map<string,int>::iterator it=sockets.begin(); it!=sockets.end(); it++)
					{
						socket = it->second;
					
						if(isadmin(socket)==0){
							
							writeline(socketid, "Os utilizadores "+ it->first +" estão online como Administradores!\n");	
							}
					
					
				//else
				//	writeline(socketid, "Nãp há utilizadores que estão online como Administradores!\n");
						
				}}
	
	else
				writeline(socketid, "Precisa de fazer login para executar o comando!\n");
}




void setaskusers_c(int socketid, string args)
{
    if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    string ask1, ask2, ask3, ask4, falha, user, query;
    istringstream iss(args);
    int ok=0;

    user=usernames[socketid];

    getline(iss, ask1, ' ');
    getline(iss, ask2, ' ');
    getline(iss, ask3, ' ');
    getline(iss, ask4, ' ');
    getline(iss, falha, ' ');

	cout<<endl<<"Ask Users:"<<endl<<ask1<<endl<<ask2<<endl<<ask3<<endl<<ask4<<endl<<"-----------------"<<endl;

    if(falha!="\0")
        ok=-1;
    if (ask1=="\0")
        ok=-2;

    if(ok==-1)
        writeline(socketid, "ERRO: Introduziu argumentos a mais!");
    else if(ok==-2)
        writeline(socketid, "ERRO: Não introduziu nenhum utilizador!");
    else if(ok==0)
    {
			query="INSERT INTO ajudautilizadores  VALUES  (DEFAULT, '" + ask1 + "', '" + ask2 + "', '" + ask3 + "', '" + ask4 + "' WHERE useremjogo = '"+user+"')";
			cout<<query<<endl;
        executeSQL(query);
        writeline(socketid, "Utilizadores inseridos com sucesso!");
    }
}


void showaskusers_c(int socketid, string args)
{
}/*	
    if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    string user=usernames[socketid];

	PGresult* res2 = executeSQL("SELECT () FROM perguntas WHERE ");

    if (isadmin(socketid)!=0 || !user.compare())
    {
        cout <<"User" + usernames[socketid] +" tentou ver todas as questoes!\n"<<endl;
        writeline(socketid,"Não tem permissões para usar esse comando!");
        return;
    }

    string sql;

    PGresult* res2 = executeSQL("SELECT (id, questao) FROM perguntas");
    cout<<"ID\t\tPERGUNTA\n"<<endl;
    for (int row = 0; row < PQntuples(res2); row++)
    {
        sql=PQgetvalue(res2, row, 0);
        cout<<sql<<endl;
    }
}











*/
