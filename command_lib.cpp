#include <map>

#include "command_lib.h"

#define ADMIN_PERM 0

map<string, int> sockets;
map<int, string> usernames;

/* Envia uma string para um socket */
void writeline(int socketfd, string line) {
	string tosend = line + "\n";
	write(socketfd, tosend.c_str(), tosend.length());
}

/**
*
*/
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

	while (ifs.good()) {
    	out_string << c;
    	c = ifs.get();
	}

	ifs.close();

	writeline(socketid, out_string.str());
}

/**
*
*/
void register_c(int socketid, string args)
{
	istringstream iss(args);
	string user, pass;
	
	iss >> user >> pass;

	if(islogged(socketid)) {
		writeline(socketid, "Já se encontra online! Por favor, faça logout antes de criar uma conta.\n");
	}

	/*!!! 
		O username deverá ter também um nº minimo de carateres; 
		Possivelmente será melhor dizer que os campos estão mal, em vez de dizer o que está mal. Isto porque se eu só preencher um campo (e.g. password)
		e deixar o username sem nada ele vai achar que o campo é o username e não a password como o utilizador acha que é.
	!!!*/
	
	if (user.length() > 32) {
		writeline(socketid, "O seu username tem demasiados caracteres. Máximo de caracteres permitidos: 32\n");
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
	
	writeline(socketid, "A conta foi criada com successo!\n");
}

void login_c(int socketid, string args)
{
	istringstream iss(args);
	string user, pass;
	
	iss >> user >> pass;
	
	if (usernames.find(socketid) != usernames.end()) {
		writeline(socketid, "Já se encontra ligado, por favor faça logout e tente novamente.\n");
		return;
	}
	
	if (pass.length() < 4) {
		writeline(socketid, "Password incorrecta!\n");
		return;
	}	
	
	string query = "SELECT (username, password) FROM utilizador WHERE username='" + user + "' AND password='" + pass + "';";
	
	// Verifica se as credenciais estão na base de dados
	PGresult* result = executeSQL(query);
	
	if(PQntuples(result) > 0) {
		sockets[user] 		= socketid;
		usernames[socketid] = user;
		writeline(socketid, "\n\n\n\nBem-vindo " + user + "!");		
	}
	else 
	{
		writeline(socketid, "Os dados de login não estão correctos!\n Tente novamente.");
		return;
	}

	listusers();
}

void logout_c(int socketid)
{
	if (usernames.find(socketid) != usernames.end()) {
		
		sockets.erase(usernames[socketid]);
		usernames.erase(socketid);
	
		writeline(socketid, "Efectuou logout com successo!\n");

		listusers();
					
		return;
	}
	else {
		writeline(socketid, "Não encontra ligado, por favor faça login.\n");	
	}
}

void resetpassword_c(int socketid, string args)
{
	istringstream iss(args);
	string user, newpass, query;
	
	iss >> user >> newpass;
	
	cout << "valor retornado:" << isadmin(socketid) << endl;
	
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
	
	query = "UPDATE utilizador SET password = '" + newpass + "' WHERE username = '" + user + "';";
	
	cout << query;
	
	PGresult* result = executeSQL(query);	

	writeline(socketid, "A password do utilizador foi alterada com sucesso.\n");	
}


/*
	Muda a password
*/
void changepassword_c(int socketid, string args)
{
	istringstream iss(args);
	string old, newp, confirm, user;
		
	iss >> old >> newp >>confirm;

	user=usernames[socketid];
	//cout<<"\n\nUSER: "+user+"\noldpass: "+old+"\nNew Pass: " + newp + "\nConfirma: " + confirm + "\n";
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
		writeline(socketid, "Password antiga está incorreta!");

}

void changeusername_c(int socketid, string args)
{
	istringstream iss(args);
	string newuser, olduser;
		
	iss >> newuser;

	olduser=usernames[socketid];
	
	string query = "SELECT (username) FROM utilizador WHERE username='" + newuser+";";
	
	PGresult* result = executeSQL(query);
	
	if(PQntuples(result) == 0) 
	{
		query = "UPDATE utilizador SET username = '" + newuser + "' WHERE username = '" + olduser + "';";
		result = executeSQL(query);
		writeline(socketid, "Mudança bem sucedida! Saudações, "+ newuser);	
	}
	else 
		writeline(socketid, "Já existe um utilizador com esse username!");

}

void say_c(int socketid, string args)
{
	string user, mensagem, palavra;
	istringstream iss(args);
	
	iss >> user;
		
	while (iss >> palavra) 
		mensagem += palavra + ' ';
	
	cout<<endl<<endl<<"Para: "<<user;
	cout<<endl<<endl<<"Mensagem: "<<mensagem<<endl;
	
	// Verifica se o utilizador que enviou a mensagem está online
	if(!islogged(socketid)) {
		writeline(socketid, "Por favor, efectue login!");
		return;
	}
	
	// Verifica se o recipiente da mensagem se encontra online
	if (!(sockets.find(user) != sockets.end())) {
		writeline(socketid, "O utilizador " + user + " não se encontra online!");
		return;
	}
	
	writeline(sockets[user], usernames[socketid] + " disse: '" + mensagem + "'\n");
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

/*
\showallquestions
Permite aos administradores ver uma tabela com todas as perguntas com respetivo id e respostas.

*/
void showallquestions_c(int socketid)
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


/*
(1: questão; 2:resposta certa; 3:2ª resposta; 4:3ª resposta; 5: última resposta). 
JA NAO E PRECISO-----Depois insere-se o texto, os espaços utilizados nos parâmetros têm de ser substituídos por underscores ( _ ).
*/

void editquestion_c(int socketid, string args)
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


/*
\deletequestion <id>
Permite aos administradores apagar perguntas. Para isso é necessário indicar o “id” da pergunta que pode ser obtido através do comando “showallquestions”.
*/

void deletequestion_c(int socketid, string args)
{
	string user=usernames[socketid], id;
	istringstream iss(args);
		
		//podemos usar com a funçao isadmin
	
	PGresult* res = executeSQL("SELECT (username, permissao) FROM utilizador WHERE username='" + user + "' AND permissao=0;");
	
	if(PQntuples(res)==0)
		writeline(socketid, "ERRO: Não tem permissões para executar esse comando");
	else
	{
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

void create_c(int socketid, string args)
{
	if(islogged(socketid))
	{
		istringstream iss(args);
		string tempo, questoes, query;
		
		iss >> tempo >> questoes;

		query="INSERT INTO jogo  VALUES  (DEFAULT, " + questoes + " , "+tempo+" , '"+usernames[socketid]+"', DEFAULT , DEFAULT)"; //ter data:CURRENT_TIMESTAMP(2)
		executeSQL(query);

		writeline(socketid, "Jogo criado com sucesso!");
		
		//TALVEZ ESPERAR ELE CONVIDAR? e escolher jogadores de ajuda????????
	}
	else
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
	
}

//INCOMPLETO
void challenge_c(int socketid, string args)
{
	if(islogged(socketid)) // e se já tem jogo criado mas não iniciado ... talvez meter thread para ser obrigado a fazer isto depois de criar?

	{
		istringstream iss(args);
		string user, tempo, questoes, query, desafiador;
		
		iss >> user;
		
		desafiador=usernames[socketid];
	
		if(!userexists(user))
			writeline(socketid, "O username selecionado não existe!\n");
		else if(!user.compare(desafiador))
			writeline(socketid, "Não se pode desafiar a si mesmo!\n");
		else if(!islogged(sockets[user]))
			writeline(socketid, "O user especificado não se encontra online. Tente mais tarde!\n");
		else
			writeline(sockets[user],"O jogador "+ usernames[socketid]+" convidou-o para iniciar um jogo. Para aceitar escreva: \\accept, para rejeitar escreva: \\decline\n");

		//query="INSERT INTO jogo  VALUES  (DEFAULT, " + questoes + " , "+tempo+" , '"+usernames[socketid]+"', DEFAULT , CURRENT_TIMESTAMP(2))";

	}
	else
		writeline(socketid, "Precisa de fazer login para executar o comando!\n");
	
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

void listusers(void)
{
	for (map<string,int>::iterator it=sockets.begin(); it!=sockets.end(); it++)
    	cout << "O utilizador " << it->first << " está logged in no socket " << it->second << endl;
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

/**
*	Verifica se o username existe na base de dados.
*/
bool userexists(string user)
{
	PGresult* result = executeSQL("SELECT username FROM utilizador WHERE username ILIKE '" + user + "'");

	if(PQntuples(result) > 0) {
		return true;
	}
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