#include <map>

#include "command_lib.h"

#define ADMIN_PERM 0

map<string, int> sockets;
map<int, string> usernames;
map<string, int> jogo_criado;

string intToString(int i) {
        ostringstream oss;
        oss << i;
        return oss.str();
}

/***********	jogo	**********/

void * jogo(void * args)
{
	int game_id = *(int *) args;

	// Ir buscar à BD os dados do jogo
	string query = "SELECT nperguntas, duracao, criador, player1, player2 FROM jogo WHERE id = '" + intToString(game_id) + "';";
	PGresult* res = executeSQL(query);
	
	int nquestoes 	 = atoi(PQgetvalue(res, 0, 0));
	int duracao 	 = atoi(PQgetvalue(res, 0, 1));
	string criador	 = PQgetvalue(res, 0, 2);
	string player1	 = PQgetvalue(res, 0, 3);
	string player2	 = PQgetvalue(res, 0, 4);
	
	int questoes[nquestoes];
	
	// Jogadores estão bloqueados
	cout << "Jogo inciado" << endl;

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
		
		writeline( 6/*socket_player1*/, intToString(i+1) + "." + pergunta + "\nA: " + respostas[ordem[0]] + "\nB: " + respostas[ordem[1]] + "\nC: " + respostas[ordem[2]] + "\nD: " + respostas[ordem[3]] + "\n\n");
		
		
		// FlagWaitingForAwnser = true;
		/*
			Aqui deverá ter uma forma de a função "\awnser" verificar que está a ser aguardada uma resposta do utilizador.
			Esta resposta deverá ser guardada numa qualquer variável que depois será lida após ter terminado o tempo.
		*/
				
		cout << "A entrar em espera" << endl;
		
		// Esperar o tempo da questão
		cout.flush();
		sleep(duracao);
		
		writeline( 6/*socket_player1*/, "Terminou o tempo.\n");
		
		clock_t questionEnd_time = clock();
		double  elapsedTime;
		
		
		do {
			clock_t curTime = clock();
			clock_t clockTicksTaken = curTime - questionEnd_time;
			elapsedTime = clockTicksTaken / (double) CLOCKS_PER_SEC;
		} while(elapsedTime < TIME_BETWEEN_QUESTIONS /* || Todos "ready" */);
		/*
			While(! Todos "ready");
		*/
		
		// Verificar as respostas dos utilizadores
		/*
			...
		*/		
	}
	
	// O jogo terminou
}

/***			end jogo			***/


/* Envia uma string para um socket */
void writeline(int socketfd, string line) {
	string tosend = line + "\n";
	write(socketfd, tosend.c_str(), tosend.length());
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
	
	writeline(socketid, "A conta foi criada com successo!\n");
	
	login_c(socketid, args);
	
	
	
	
	
	/*iss >> user >> pass;

	if(islogged(socketid)) {
		writeline(socketid, "Já se encontra online! Por favor, faça logout antes de criar uma conta.\n");
	}

	/*!!! 
		Possivelmente será melhor dizer que os campos estão mal, em vez de dizer o que está mal. Isto porque se eu só preencher um campo (e.g. password)
		e deixar o username sem nada ele vai achar que o campo é o username e não a password como o utilizador acha que é.
	!!!
	
	if (user.length() > 32) {
		writeline(socketid, "O seu username tem demasiados caracteres. Máximo de caracteres permitidos: 32\n");
		return;
	}
	if (user.length() < 4) {
		writeline(socketid, "O seu username tem poucos caracteres. Minimo de caracteres permitidos: 4\n");
		return;
	}
	else if (pass.length() < 4) {
		writeline(socketid, "A sua password tem poucos caracteres. Minimo de caracteres: 4\n");
		return;
	}		
	else if (pass.length() > 64) {
		writeline(socketid, "A sua password tem demasiados caracteres. Maximo de caracteres: 64\n");
		return;
	}
	else if (!alphanumeric(user)) {
		writeline(socketid, "O username poderá apenas conter caracteres alfanuméricos\n");
		return;	
	}

	if(userexists(user)) {
		writeline(socketid, "O username seleccionado não se encontra disponível!\nPor favor, tente novamente.\n");
		return;
	}
	
	executeSQL("INSERT INTO utilizador VALUES ('" + user + "', '" + pass + "', 1)");
	
	writeline(socketid, "A conta foi criada com successo!\n");*/
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

void say_c(string args)
{
	//if(islogged(socketid))
	//{
			string user, mensagem, palavra;
			istringstream iss(args);
			
			iss>>user;
				
			while (iss >> palavra) 
				mensagem += palavra + ' ';
			
			cout<<endl<<endl<<"Para: "<<user;
			cout<<endl<<endl<<"Mensagem: "<<mensagem<<endl;	
			writeline(sockets[user], mensagem);
	//}
	//else 
	//	writeline(socketid,"Permissão negada! Não se encontra logado");
	
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
	if(islogged(socketid))
	{
				string pergunta, respcerta, resperrada1, resperrada2, resperrada3, detetarfalha, user;
				istringstream iss(args);
				int ok=0;
				
				user=usernames[socketid];
				
				getline(iss, pergunta, '|');
				getline(iss, respcerta, '|');
				getline(iss, resperrada1, '|');
				getline(iss, resperrada2, '|');
				getline(iss, resperrada3, '|');
				getline(iss, detetarfalha, '|');
				
				//cout<<"\n\n"+ pergunta+"\n\n"+ respcerta+"\n\n"+ resperrada1+"\n\n"+ resperrada2+"\n\n"+ resperrada3+"\n\n"+ detetarfalha;
				if (detetarfalha!="\0")
					ok=-1;
				if (pergunta=="\0"||resperrada1=="\0"||resperrada2=="\0"||resperrada3=="\0"||respcerta=="\0" || resperrada1.length() <1 || resperrada2.length() <1 || resperrada3.length() <1 || respcerta.length() <1 || pergunta.length() <1)
					ok=-2;
				
				if(ok==1)
					writeline(socketid, "ERRO: Introduziu argumentos a mais");
				else if(ok==-2)
					writeline(socketid, "ERRO: Introduziu argumentos a menos");
				else if(ok==0)
				{
					executeSQL("INSERT INTO perguntas  VALUES  (DEFAULT, '" + pergunta + "', '" + respcerta + "', '" + resperrada1 + "', '" + resperrada2 + "', '" + resperrada3 + "', '" + user + "')");
					writeline(socketid, "Pergunta inserida com sucesso!");
				}
	}
	else 
		writeline(socketid,"Permissão negada! Não se encontra logado");
		
}

/*
\showallquestions
Permite aos administradores ver uma tabela com todas as perguntas com respetivo id e respostas.

*/
void showallquestions_c(int socketid)
{
	
	if(islogged(socketid))
	{
			string user=usernames[socketid];
			
			
			//podemos usar com a funçao isadmin
			PGresult* res = executeSQL("SELECT (username, permissao) FROM utilizador WHERE username='" + user + "' AND permissao=0;");
			
			if(PQntuples(res)==0)
				writeline(socketid, "ERRO: Não tem permissões para executar esse comando");
			else
			{
				string sql;
						
				PGresult* res2 = executeSQL("SELECT (id, questao) FROM perguntas");
				cout<<"ID\t\tPERGUNTA\n";
				for (int row = 0; row < PQntuples(res2); row++)
				{
					sql=PQgetvalue(res2, row, 0);
					cout<<"\n"<<sql;
				}
				
				cout<<"\n";
			}
	}		
	else 
		writeline(socketid,"Permissão negada! Não se encontra logado");	
	
	
}


/*
(1: questão; 2:resposta certa; 3:2ª resposta; 4:3ª resposta; 5: última resposta). 
JA NAO E PRECISO-----Depois insere-se o texto, os espaços utilizados nos parâmetros têm de ser substituídos por underscores ( _ ).
*/

void editquestion_c(int socketid, string args)
{

	if(islogged(socketid))
	{
				string user=usernames[socketid], id, p, texto;
				istringstream iss(args);
				int parametro;
				
					//podemos usar com a funçao isadmin

				PGresult* res = executeSQL("SELECT (username, permissao) FROM utilizador WHERE username='" + user + "' AND permissao=0;");
				
				if(PQntuples(res)==0)
					writeline(socketid, "ERRO: Não tem permissões para executar esse comando");
				else
				{
					getline(iss, id, ' ');
					getline(iss, p, ' ');
					getline(iss, texto, '|');
					
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
	}

		else 
		writeline(socketid,"Permissão negada! Não se encontra logado");
}


/*
\deletequestion <id>
Permite aos administradores apagar perguntas. Para isso é necessário indicar o “id” da pergunta que pode ser obtido através do comando “showallquestions”.
*/

void deletequestion_c(int socketid, string args)
{

	if(islogged(socketid))
	{
			
			string user=usernames[socketid], id;
			istringstream iss(args);
				
				//podemos usar com a funçao isadmin
			
			PGresult* res = executeSQL("SELECT (username, permissao) FROM utilizador WHERE username='" + user + "' AND permissao=0;");
			
			if(PQntuples(res)==0)
				writeline(socketid, "ERRO: Não tem permissões para executar esse comando");
			else
			{
				//writeline(socketid, "Primeiro é necessário ver todas as questoes!  Sendo assim, este comando vai dar erro!\nTerá de ver as questoes que já se encontram disponiveis no servidor!\n Por favor imprima de acordo com o help desta vez!\n->  \question idpergunta\n\n");
				//showallquestions_c(socketid);
				
				getline(iss, id, ' ');
				
				PGresult* result = executeSQL("SELECT * FROM perguntas WHERE id= " + id +";");
						
				if(PQntuples(result)==0)
					writeline(socketid, "ERRO: ID da pergunta incorreto!");
					
				else
				{
					PGresult* res = executeSQL("DELETE FROM perguntas WHERE id= " + id + ";");	
					writeline(socketid, "Pergunta eliminada com sucesso!");
				}
			}
	}
	else 
		writeline(socketid,"Permissão negada! Não se encontra logado");
	
}

void changepermissions_c(int socketid, string args)
{
	//Verifica se é admin
	if(islogged(socketid))
	{
		if (isadmin(socketid)!=0 )
		{ 
			cout <<"User" + usernames[socketid] +" tentou mudar permissoes"<<endl; 
			writeline(socketid,"Não tem permissões para usar esse comando!");
		}
		else if(isadmin(socketid)==0)
		{
			istringstream iss(args);
			string user, permissao, falha;
			getline(iss, user, ' ');
			getline(iss, permissao, ' ');
			
			if(user==usernames[socketid])
				writeline(socketid, "Não pode mudar as suas permissoes");
			
			else
			{
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
		}
	}
	
	else 
		writeline(socketid,"Permissão negada! Não se encontra logado");	
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
	
	int game_id = jogo_criado[usernames[socketid]];
	
	pthread_t gamethread;
	pthread_create(&gamethread, NULL, jogo, &game_id);	
}

void create_c(int socketid, string args)
{
	istringstream iss(args);
	string tempo, questoes, query;
	
	iss >> tempo >> questoes;

	if(!islogged(socketid)) {
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
		return;
	}
	
	if (jogo_criado.find(usernames[socketid]) != jogo_criado.end()) {
		writeline(socketid, "Já tem um jogo criado!\nPor favor, cancele o jogo anterior antes de criar um novo.");
		return;
	}
	
	if(tempo == "\0" || questoes == "\0")
	{
		writeline(socketid, "Parâmetros incorrectos.\n");
		return;
	}		
	
	query="INSERT INTO jogo VALUES  (DEFAULT, " + questoes + " , " + tempo + " , '" + usernames[socketid] + "'); SELECT currval('id_jogo_seq');"; //ter data:CURRENT_TIMESTAMP(2)
	
	PGresult* result = executeSQL(query);

	int game_id = atoi(PQgetvalue(result, 0, 0));

	jogo_criado[usernames[socketid]] = game_id;

	writeline(socketid, "Jogo criado com sucesso! ID do jogo: " + intToString(game_id));
}

/**
*	INCOMPLETO
*/
void challenge_c(int socketid, string args)
{
	if(!islogged(socketid)) {
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
		return;
	}
		
	istringstream iss(args);
	
	bool criador = false;
	
	string user, tempo, questoes, query, desafiador, id;
	
	iss >> user >> id;
	
	cout<<user<<endl<<id<<endl;
	
	PGresult* res = executeSQL("SELECT (criador) FROM jogo WHERE id=" + id + ";");
	
	//cout<<"SELECT (criador) FROM jogo WHERE id='" + id + ";"<<res;
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
		writeline(sockets[user],"O jogador " + usernames[socketid] + " convidou-o para iniciar um jogo.\nPara aceitar escreva: \\accept, para rejeitar escreva: \\decline\n");

	// FlagWaitingForAccept = true;	
	
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
	string id;
	
	iss >> id;
	
	if(!islogged(socketid)) {
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
		return;
	}
	
	//****************************
	//VER SE O ID DO JOGO E VALIDO
	//****************************
	
	
	/*if(user == "\0") {
		writeline(socketid, "Não indicou o nome do utilizador.\n");
		return;
	}
	
	/*if(!userexists(user)) {
		writeline(socketid, "O nome do utilizador não está correcto.\n");
		return;
	}
		
	if (!(jogo_criado.find(user) != jogo_criado.end())) {
		writeline(socketid, "Esse utilizador não criou nenhum jogo.\n");
		return;
	}*/
	

	PGresult* res1 = executeSQL("SELECT dataehora FROM jogo WHERE id="+id+";");
	string c1 = PQgetvalue(res1, 0, 0);
	if(c1!="")
	{
		writeline(socketid, "O jogo já começou! Não é mais possivel aceitar o convite\n");
		return;
	}
	
	
	res1 = executeSQL("SELECT convidado1 FROM jogo WHERE id="+id+";");
	c1 = PQgetvalue(res1, 0, 0);
	res1 = executeSQL("SELECT convidado2 FROM jogo WHERE id="+id+";");
	string c2 = PQgetvalue(res1, 0, 0);
	res1 = executeSQL("SELECT convidado3 FROM jogo WHERE id="+id+";");
	string c3 = PQgetvalue(res1, 0, 0);
	res1 = executeSQL("SELECT convidado4 FROM jogo WHERE id="+id+";");
	string c4 = PQgetvalue(res1, 0, 0);
	
	cout<<endl<<c1.compare(usernames[socketid])<<endl<<c2.compare(usernames[socketid])<<endl<<c3.compare(usernames[socketid])<<endl<<c4.compare(usernames[socketid])<<endl; 

	
	if(c1.compare(usernames[socketid]) && c2.compare(usernames[socketid]) && c3.compare(usernames[socketid]) && c4.compare(usernames[socketid])) 
	{
		writeline(socketid, "Não foi convidado para este jogo\n");
		return;
	}
	
	res1 = executeSQL("SELECT player1 FROM jogo WHERE id="+id+";");
	c1 = PQgetvalue(res1, 0, 0);
	if(c1=="")
	{
		executeSQL("UPDATE jogo SET player1 = '" + usernames[socketid] + "' WHERE id = " + id + ";");
		writeline(socketid, "Tudo Pronto! Espere pelo inicio do jogo!\n");
		return;
	}
	else
	{
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
		Verificar se o jogador "user" convidou o este jogado para o jogo.
	*/
	
	// Ver se o player1 da tabelo jogo é NULL
	
	// Se for pôr lá o username do utilizador do socketid
	
	// Senão ver se o player2 da tabelo jogo é NULL
	
	// Se for pôr lá o username do utilizador do socketid
	
	// Senão dizer que ocorreu um erro ou que já está cheio e sair
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
	
	cout<<c1.compare(usernames[socketid])<<endl<<isadmin(socketid);
	
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


void deleteaccount_c(int socketid, string args)
{
	if(islogged(socketid))
	{
		
			string user, pass;
			istringstream iss(args);
			iss >> user >> pass;
			
				if (executeSQL("DELETE FROM jogador WHERE (username = '"+user+"' AND password = '"+pass+"')") == NULL) 
					{
					  writeline(socketid, "\nErro a apagar registo. Tente outra vez.");
					  return ;
					}
				
				sockets.erase(usernames[socketid]);
				usernames.erase(socketid);
				writeline(socketid, "A sua conta foi apagada.\n");
				cout << "Utilizador apagado. Username: " << user << endl;
		
					
	}
	
	else
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
}