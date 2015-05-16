#include <map>

#include "command_lib.h"

#define ADMIN_PERM 0

map<string, int> sockets;				// Permite saber o socket associado a um username
map<int, string> usernames;				// Permite saber o username associado a um socket
map<string, int> jogo_criado;			// Permite saber o id do jogo associaddo a um username
map<string, bool> waitingForAnswer;		// Permite saber se um username e suposte responder a uma questao no momento
map<string, int> currAnswer;			// Permite saber a resposta atual de um username (ou ajudas)
map<string, int> emJogo;				// Permite saber o id do jogo associado ao username, =-1 se nao estiver em nennhum
map<int, int> currQuestion;				// Permite saber o id da questao no momento associada ao id do jogo. =-1 se nenhuma
map<string, string> currAskuser;		// Permite saber qual o askuser a que o username soliciou a ajuda

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

	currQuestion[game_id] = -1;

	cout << "Jogo iniciado com id: " << game_id << endl;

    // Ir buscar à BD os dados do jogo
    string query = "SELECT nperguntas, duracao, criador, player1, player2 FROM jogo WHERE id = '" + intToString(game_id) + "';";

    //cout << query << endl;

    PGresult* res = executeSQL(query);

    int nquestoes 	 = atoi(PQgetvalue(res, 0, 0));
    int duracao 	 = atoi(PQgetvalue(res, 0, 1));
    string criador	 = PQgetvalue(res, 0, 2);
    string player1	 = /*"jogador2"*/ PQgetvalue(res, 0, 3);
    string player2	 = /*"jogador3"*/ PQgetvalue(res, 0, 4);

    int questoes[nquestoes];

    // Jogadores estão bloqueados
    cout << "Jogo inciado!" << endl;

    writeline(sockets[criador], "Jogo Iniciado. Boa sorte!!\n");

    query = "UPDATE jogo SET dataehora = CURRENT_TIMESTAMP(2) WHERE id = " + intToString(game_id) + ";";

    PGresult* result = executeSQL(query);

	emJogo[criador] = true;

    if(player1 != "")
    {
        writeline(sockets[player1], "Jogo Iniciado. Boa sorte!!\n");
        emJogo[player1] = game_id;
        player1Presente = true;
    }

    if(player2 != "")
    {
        writeline(sockets[player2], "Jogo Iniciado. Boa sorte!!\n");
        emJogo[player2] = game_id;
        player2Presente = true;
    }

	// Seleccionar as perguntas para o jogo
	result = executeSQL("SELECT id FROM perguntas ORDER BY random() LIMIT " + intToString(nquestoes));
	
	for(int linha=0; linha < nquestoes; linha++) {
	
        questoes[linha] = atoi(PQgetvalue(result, linha, 0));
	}

	/***      Criar linhas na estatisticautilizador     ***/

	query = "SELECT id FROM estatisticautilizador WHERE username='" + criador + "';";
	
	PGresult* resultado = executeSQL(query);
	
	// Criar uma linha
	if(PQntuples(resultado) == 0) {
		query = "INSERT INTO estatisticautilizador VALUES (DEFAULT,DEFAULT,DEFAULT,DEFAULT,DEFAULT,'" + criador + "');";
		executeSQL(query);
	}
	
	if(player1Presente) {
		// Ver se o utilizador já tem um linha na estatisticautilizador		
		query = "SELECT id FROM estatisticautilizador WHERE username='" + player1 + "';";
		PGresult* result = executeSQL(query);
		
		// Criar uma linha
		if(PQntuples(result) == 0) {
			query = "INSERT INTO estatisticautilizador VALUES (DEFAULT,DEFAULT,DEFAULT,DEFAULT,DEFAULT,'" + player1 + "');";
			executeSQL(query);
		}
	}
	
	if(player2Presente) {
		// Ver se a questão já tem um linha na estatisticapergunta		
		query = "SELECT id FROM estatisticautilizador WHERE username='" + player2 + "';";
		PGresult* result = executeSQL(query);
		
		// Criar uma linha
		if(PQntuples(result) == 0) {
			query = "INSERT INTO estatisticautilizador VALUES (DEFAULT,DEFAULT,DEFAULT,DEFAULT,DEFAULT,'" + player2 + "');";
			executeSQL(query);
		}
	}

	/***      End criar linhas na estatisticautilizador     ***/

	for(int i=0; i < nquestoes; i++) {
		
		string pergunta, respostas[4];
		vector<int> ordem;
		
		for (int n=0; n<4; ++n) ordem.push_back(n);
		
		// Definir a ordem aleatória das respostas
		random_shuffle(ordem.begin(), ordem.end());
		
		// Imprimir as perguntas nos terminais dos jogadores
		string query = "SELECT questao, respcerta, resperrada1, resperrada2, resperrada3 FROM perguntas WHERE id = '" + intToString(questoes[i]);
		PGresult* res = executeSQL(query + "'; ");
		
		pergunta = PQgetvalue(res, 0, 0);
		respostas[0] = PQgetvalue(res, 0, 1);
		respostas[1] = PQgetvalue(res, 0, 2);
		respostas[2] = PQgetvalue(res, 0, 3);
		respostas[3] = PQgetvalue(res, 0, 4);
		
		string questao = intToString(i+1) + "." + pergunta + "\nA: " + respostas[ordem[0]] + "\nB: " + respostas[ordem[1]] + "\nC: " + respostas[ordem[2]] + "\nD: " + respostas[ordem[3]] + "\n\n";
		
		// Inicializar as respostas: -1
		currAnswer[criador] = -1;
		currAnswer[player1] = -1;
		currAnswer[player2] = -1;		
		
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
		
		currQuestion[game_id] = questoes[i];
			
		// Esperar o tempo da questão
		clock_t questionStart_time = clock();
		double  elapsedTime;
		int tempo_extra = 0;
		bool inAskCriador = false;
		bool inAskPlayer1 = false;
		bool inAskPlayer2 = false;
		
		do {
			clock_t curTime = clock();
			clock_t clockTicksTaken = curTime - questionStart_time;
			elapsedTime = clockTicksTaken / (double) CLOCKS_PER_SEC;
			
			/*****
				Fazer aqui as coisas necessárias a meio do jogo. (Ajudas, ...)
			*****/
						
			/***     ask criador     ***/
			if(currAnswer[criador] == 5) {
			
				// Verificar se o utilizador ainda tem esta ajuda disponível
				string query = "SELECT askc FROM jogo WHERE id = '" + intToString(game_id) + "'; ";
				PGresult* res = executeSQL(query);
				
				string askc = PQgetvalue(res, 0, 0);
				
				if(askc == "f" || inAskCriador) {
		
					// Informar todos os jogadores que esta questao vai ter +10 segundos
					if(player1Presente) writeline( sockets[player1], "Um utilizador solicitou ajuda.\nEsta questão terá uma duração extra de 10 segundos.");
					if(player2Presente) writeline( sockets[player2], "Um utilizador solicitou ajuda.\nEsta questão terá uma duração extra de 10 segundos.");
					
					if(!inAskCriador) {
						// Imprimir a questao no utilizador ajuda
						writeline( sockets[currAskuser[criador]], "\n\n" + criador + " pediu a sua ajuda para a questão:");
						writeline( sockets[currAskuser[criador]], questao);
						waitingForAnswer[currAskuser[criador]] = true;

						// Update do ajuda na BD
						string query = "UPDATE jogo SET askc = true WHERE id = '" + intToString(game_id)  + "'; ";
						executeSQL(query);
						
						// Aguardar que ele responda (+10 segundos)
						tempo_extra = 10;
						
						inAskCriador = true;
					}
					// Imprimir a resposta do jogador ajuda no que a solicitou
					if(currAnswer[currAskuser[criador]] != -1) {
						writeline( sockets[criador], "\nO utilizador " + currAskuser[criador] + " acha que é a resposta " + numToResp(currAnswer[currAskuser[criador]]));
						
						currAnswer[currAskuser[criador]] = -1;
						currAskuser[criador] = "";
						currAnswer[criador] = -1;
						inAskCriador = false;
					}
				} else {
					writeline( sockets[criador], "Esta ajuda já não se encontra disponível!");
					currAnswer[criador] = -1;
				}
			}
			
			/***     ask player1     ***/
			if(currAnswer[player1] == 5) {
			
				// Verificar se o utilizador ainda tem esta ajuda disponível
				string query = "SELECT ask1 FROM jogo WHERE id = '" + intToString(game_id) + "'; ";
				PGresult* res = executeSQL(query);
				
				string ask1 = PQgetvalue(res, 0, 0);
				
				if(ask1 == "f" || inAskPlayer1) {
		
					// Informar todos os jogadores que esta questao vai ter +10 segundos
					writeline( sockets[criador], "Um utilizador solicitou ajuda.\nEsta questão terá uma duração extra de 10 segundos.");
					if(player2Presente) writeline( sockets[player2], "Um utilizador solicitou ajuda.\nEsta questão terá uma duração extra de 10 segundos.");
					
					if(!inAskPlayer1) {
						// Imprimir a questao no utilizador ajuda
						writeline( sockets[currAskuser[player1]], "\n\n" + player1 + " pediu a sua ajuda para a questão:");
						writeline( sockets[currAskuser[player1]], questao);
						waitingForAnswer[currAskuser[player1]] = true;
						
						// Update do ajuda na BD
						string query = "UPDATE jogo SET ask1 = true WHERE id = '" + intToString(game_id)  + "'; ";
						executeSQL(query);
						
						// Aguardar que ele responda (+10 segundos)
						tempo_extra = 10;
						
						inAskPlayer1= true;
					}
					// Imprimir a resposta do jogador ajuda no que a solicitou
					if(currAnswer[currAskuser[player1]] != -1) {
						writeline( sockets[player1], "\nO utilizador " + currAskuser[player1] + " acha que é a resposta " + numToResp(currAnswer[currAskuser[player1]]));
						
						currAnswer[currAskuser[player1]] = -1;
						currAskuser[player1] = "";
						currAnswer[player1] = -1;
						
						inAskPlayer1= false;
					}
				} else {
					writeline( sockets[player1], "Esta ajuda já não se encontra disponível!");
					currAnswer[player1] = -1;
				}
			}
			
			/***     ask player2     ***/
			if(currAnswer[player2] == 5) {
			
				// Verificar se o utilizador ainda tem esta ajuda disponível
				string query = "SELECT ask2 FROM jogo WHERE id = '" + intToString(game_id) + "'; ";
				PGresult* res = executeSQL(query);
				
				string ask2 = PQgetvalue(res, 0, 0);
				
				if(ask2 == "f" || inAskPlayer2) {
		
					// Informar todos os jogadores que esta questao vai ter +10 segundos
					writeline( sockets[criador], "Um utilizador solicitou ajuda.\nEsta questão terá uma duração extra de 10 segundos.");
					if(player1Presente) writeline( sockets[player1], "Um utilizador solicitou ajuda.\nEsta questão terá uma duração extra de 10 segundos.");
					
					if(!inAskPlayer2) {
						// Imprimir a questao no utilizador ajuda
						writeline( sockets[currAskuser[player2]], "\n\n" + player2 + " pediu a sua ajuda para a questão:");
						writeline( sockets[currAskuser[player2]], questao);
						waitingForAnswer[currAskuser[player2]] = true;
						
						// Update do ajuda na BD
						string query = "UPDATE jogo SET ask2 = true WHERE id = '" + intToString(game_id)  + "'; ";
						executeSQL(query);
						
						// Aguardar que ele responda (+10 segundos)
						tempo_extra = 10;
						
						inAskPlayer2= true;
					}
					// Imprimir a resposta do jogador ajuda no que a solicitou
					if(currAnswer[currAskuser[player2]] != -1) {
						writeline( sockets[player2], "\nO utilizador " + currAskuser[player2] + " acha que é a resposta " + numToResp(currAnswer[currAskuser[player2]]));
						
						currAnswer[currAskuser[player2]] = -1;
						currAskuser[player2] = "";
						currAnswer[player2] = -1;
						inAskPlayer2 = false;
					}
				} else {
					writeline( sockets[player2], "Esta ajuda já não se encontra disponível!");
					currAnswer[player2] = -1;
				}
			}
			
			/***     end ask     ***/
			
			/***     fiftyfifty para o criador    ***/
			
			string usedfifty;
			
			// Verificar se o cliente está num jogo e está a ser aguardada uma resposta
			if(currAnswer[criador] == 4) {
			
				// Verificar se o utilizador ainda tem esta ajuda disponível
				string query = "SELECT fiftyc FROM jogo WHERE id = '" + intToString(game_id) + "'; ";
				PGresult* res = executeSQL(query);
				
				usedfifty = PQgetvalue(res, 0, 0);
				
				if(usedfifty == "f") {
					// Encontrar a respostas certa;
					int certa;
					
					for(certa=0; certa<4; certa++) {
						if(ordem[certa] == 0)
							break;	
					}
					
					// Escolher uma resposta errada
					int errada;
					
					do {
						 errada = rand() % 4;
					} while(errada == certa);
										
					// Imprimir as duas respostas
					writeline( sockets[criador], intToString(i) + "." + pergunta);
					
					if(certa == 0 || errada == 0) {
						writeline( sockets[criador], "A: " + respostas[ordem[0]]);
					} else {
						writeline( sockets[criador], "A:");
					}
					
					if(certa == 1 || errada == 1) {
						writeline( sockets[criador], "B: " + respostas[ordem[1]]);
					} else {
						writeline( sockets[criador], "B:");
					}
					
					if(certa == 2 || errada == 2) {
						writeline( sockets[criador], "C: " + respostas[ordem[2]]);
					} else {
						writeline( sockets[criador], "C:");
					}
					
					if(certa == 3 || errada == 3) {
						writeline( sockets[criador], "D: " + respostas[ordem[3]]);
					} else {
						writeline( sockets[criador], "D:");
					}
					
					/***
						Ignorar estatisticas
					***/
					
					// Update do ajuda na BD
					string query = "UPDATE jogo SET fiftyc = true WHERE id = '" + intToString(game_id)  + "'; ";
					PGresult* res = executeSQL(query);
										
					// Update da variável
					currAnswer[criador] = -1;
					
				} else {
					writeline( sockets[criador], "Esta ajuda já não se encontra disponível!");
					currAnswer[criador] = -1;
				}
			}
			
			/***     fiftyfifty para o player1    ***/
			
			// Verificar se o cliente está num jogo e está a ser aguardada uma resposta
			if(currAnswer[player1] == 4) {
			
				// Verificar se o utilizador ainda tem esta ajuda disponível
				string query = "SELECT fifty1 FROM jogo WHERE id = '" + intToString(game_id) + "'; ";
				PGresult* res = executeSQL(query);
				
				usedfifty = PQgetvalue(res, 0, 0);
				
				if(usedfifty == "f") {
					// Encontrar a respostas certa;
					int certa;
					
					for(certa=0; certa<4; certa++) {
						if(ordem[certa] == 0)
							break;	
					}
					
					// Escolher uma resposta errada
					int errada;
					
					do {
						 errada = rand() % 4;
					} while(errada == certa);
										
					// Imprimir as duas respostas
					writeline( sockets[player1], intToString(i) + "." + pergunta);
					
					if(certa == 0 || errada == 0) {
						writeline( sockets[player1], "A: " + respostas[ordem[0]]);
					} else {
						writeline( sockets[player1], "A:");
					}
					
					if(certa == 1 || errada == 1) {
						writeline( sockets[player1], "B: " + respostas[ordem[1]]);
					} else {
						writeline( sockets[player1], "B:");
					}
					
					if(certa == 2 || errada == 2) {
						writeline( sockets[player1], "C: " + respostas[ordem[2]]);
					} else {
						writeline( sockets[player1], "C:");
					}
					
					if(certa == 3 || errada == 3) {
						writeline( sockets[player1], "D: " + respostas[ordem[3]]);
					} else {
						writeline( sockets[player1], "D:");
					}
					
					/***
						Ignorar estatisticas
					***/
					
					// Update do ajuda na BD
					string query = "UPDATE jogo SET fifty1 = true WHERE id = '" + intToString(game_id)  + "'; ";
					PGresult* res = executeSQL(query);
					
					
					// Update da variável
					currAnswer[player1] = -1;
					
				} else {
					writeline( sockets[player1], "Esta ajuda já não se encontra disponível!");
					currAnswer[player1] = -1;
				}
			}
			
			/***     fiftyfifty para o player2    ***/
			
			// Verificar se o cliente está num jogo e está a ser aguardada uma resposta
			if(currAnswer[player2] == 4) {
			
				// Verificar se o utilizador ainda tem esta ajuda disponível
				string query = "SELECT fifty2 FROM jogo WHERE id = '" + intToString(game_id) + "'; ";
				PGresult* res = executeSQL(query);
				
				usedfifty = PQgetvalue(res, 0, 0);
				
				if(usedfifty == "f") {
					// Encontrar a respostas certa;
					int certa;
					
					for(certa=0; certa<4; certa++) {
						if(ordem[certa] == 0)
							break;	
					}
					
					// Escolher uma resposta errada
					int errada;
					
					do {
						 errada = rand() % 4;
					} while(errada == certa);
										
					// Imprimir as duas respostas
					writeline( sockets[player2], intToString(i) + "." + pergunta);
					
					if(certa == 0 || errada == 0) {
						writeline( sockets[player2], "A: " + respostas[ordem[0]]);
					} else {
						writeline( sockets[player2], "A:");
					}
					
					if(certa == 1 || errada == 1) {
						writeline( sockets[player2], "B: " + respostas[ordem[1]]);
					} else {
						writeline( sockets[player2], "B:");
					}
					
					if(certa == 2 || errada == 2) {
						writeline( sockets[player2], "C: " + respostas[ordem[2]]);
					} else {
						writeline( sockets[player2], "C:");
					}
					
					if(certa == 3 || errada == 3) {
						writeline( sockets[player2], "D: " + respostas[ordem[3]]);
					} else {
						writeline( sockets[player2], "D:");
					}
					
					/***
						Ignorar estatisticas
					***/
					
					// Update do ajuda na BD
					string query = "UPDATE jogo SET fifty2 = true WHERE id = '" + intToString(game_id)  + "'; ";
					PGresult* res = executeSQL(query);					
					
					// Update da variável
					currAnswer[player2] = -1;
					
				} else {
					writeline( sockets[player2], "Esta ajuda já não se encontra disponível!");
					currAnswer[player2] = -1;
				}
			}
			
			/***     end fiftyfifty    ***/
			
		} while((elapsedTime < (duracao + tempo_extra)) &&
				(!(currAnswer[criador] == 0 || currAnswer[criador] == 1 || currAnswer[criador] == 2 || currAnswer[criador] == 3) ||
				(!(currAnswer[player1] == 0 || currAnswer[player1] == 1 || currAnswer[player1] == 2 || currAnswer[player1] == 3) && player1Presente) ||
				(!(currAnswer[player2] == 0 || currAnswer[player2] == 1 || currAnswer[player2] == 2 || currAnswer[player2] == 3) && player2Presente) 	)
				);
		
		if(inAskCriador) waitingForAnswer[currAskuser[criador]] = false;
		if(inAskPlayer1) waitingForAnswer[currAskuser[player1]] = false;
		if(inAskPlayer2) waitingForAnswer[currAskuser[player2]] = false;
		
		if(!(currAnswer[criador] == 0 || currAnswer[criador] == 1 || currAnswer[criador] == 2 || currAnswer[criador] == 3)) {
			writeline( sockets[criador], "Terminou o tempo!");
			waitingForAnswer[criador] = false;	// Assinala que está à espera de uma resposta
		} else {
			writeline( sockets[criador], "Pergunta terminada!");
		}
		
		if(!(currAnswer[player1] == 0 || currAnswer[player1] == 1 || currAnswer[player1] == 2 || currAnswer[player1] == 3) && player1Presente) {
			writeline( sockets[player1], "Terminou o tempo!");
			waitingForAnswer[player1] = false;	// Assinala que está à espera de uma resposta
		} else if(player1Presente) {
			writeline( sockets[player1], "Pergunta terminada!");
		}
		
		if(!(currAnswer[player2] == 0 || currAnswer[player2] == 1 || currAnswer[player2] == 2 || currAnswer[player2] == 3) && player2Presente) {
			writeline( sockets[player2], "Terminou o tempo!");
			waitingForAnswer[player2] = false;	// Assinala que está à espera de uma resposta
		} else if(player2Presente) {
			writeline( sockets[player1], "Pergunta terminada!");
		}
		
		int i_correct;
		
		// Descobrir a resposta certa
		for(i_correct=0; i_correct<4; i_correct++) {
			if(ordem[i_correct] == 0)
				break;	
		}
		
		// Ver se a questão já tem um linha na estatisticapergunta		
		query = "SELECT id FROM estatisticapergunta WHERE id_pergunta='" + intToString(questoes[i]) + "';";
		PGresult* result = executeSQL(query);
		
		// Criar uma linha
		if(PQntuples(result) == 0) {
			query = "INSERT INTO estatisticapergunta VALUES (DEFAULT,DEFAULT,DEFAULT," + intToString(questoes[i]) + ");";
			executeSQL(query);
		}

		/// Verificar as respostas do utilizadores
		if(currAnswer[criador] == i_correct) {
			writeline( sockets[criador], "Resposta certa!");
			
			// Actualizar a estatisticapergunta
			query = "UPDATE estatisticapergunta SET vezesquesaiu=vezesquesaiu+1, respostascertas=respostascertas+1  WHERE id_pergunta='" + intToString(questoes[i]) + "';";
			executeSQL(query);
			
			// Actualizar a estatisticautilizador
			query = "UPDATE estatisticautilizador SET resprespondidas=resprespondidas+1, respcertas=respcertas+1  WHERE username='" + criador + "';";
			executeSQL(query);
			
			// Actualizar respostas certas na tabela jogo
			query = "UPDATE jogo SET respcertascriador=respcertascriador+1  WHERE id='" + intToString(game_id) + "';";
			executeSQL(query);
			
		} else if(currAnswer[criador] == -1) {
			writeline( sockets[criador], "Não respondeu!\nA resposta certa era a: " + numToResp(i_correct));
		} else {
			writeline( sockets[criador], "Resposta errada!\nA resposta certa era a: " + numToResp(i_correct));
			
			// Actualizar a estatisticapergunta
			query = "UPDATE estatisticapergunta SET vezesquesaiu=vezesquesaiu+1 WHERE id_pergunta='" + intToString(questoes[i]) + "';";
			executeSQL(query);
			
			// Actualizar a estatisticautilizador
			query = "UPDATE estatisticautilizador SET resprespondidas=resprespondidas+1  WHERE username='" + criador + "';";
			executeSQL(query);
		}
		
		if(player1Presente) {
			if(currAnswer[player1] == i_correct) {
				writeline( sockets[player1], "Resposta certa!");	
				
				// Actualizar a estatisticapergunta
				query = "UPDATE estatisticapergunta SET vezesquesaiu=vezesquesaiu+1, respostascertas=respostascertas+1  WHERE id_pergunta='" + intToString(questoes[i]) + "';";
				executeSQL(query);
				
				// Actualizar a estatisticautilizador
				query = "UPDATE estatisticautilizador SET resprespondidas=resprespondidas+1, respcertas=respcertas+1  WHERE username='" + player1 + "';";
				executeSQL(query);
				
				// Actualizar respostas certas na tabela jogo
				query = "UPDATE jogo SET respcertas1=respcertas1+1  WHERE id='" + intToString(game_id) + "';";
				executeSQL(query);
			} else  if(currAnswer[player1] == -1) {
				writeline( sockets[player1], "Não respondeu!\nA resposta certa era a: " + numToResp(i_correct));
			} else {
				writeline( sockets[player1], "Resposta errada!\nA resposta certa era a: " + numToResp(i_correct));	
				
				// Actualizar a estatisticapergunta
				query = "UPDATE estatisticapergunta SET vezesquesaiu=vezesquesaiu+1 WHERE id_pergunta='" + intToString(questoes[i]) + "';";
				executeSQL(query);
				
				// Actualizar a estatisticautilizador
				query = "UPDATE estatisticautilizador SET resprespondidas=resprespondidas+1  WHERE username='" + player1 + "';";
				executeSQL(query);
			}
		}
		
		if(player2Presente) {
			if(currAnswer[player2] == i_correct) {
				writeline( sockets[player2], "Resposta certa!");
				
				// Actualizar a estatisticapergunta
				query = "UPDATE estatisticapergunta SET vezesquesaiu=vezesquesaiu+1, respostascertas=respostascertas+1  WHERE id_pergunta='" + intToString(questoes[i]) + "';";
				executeSQL(query);
				
				// Actualizar a estatisticautilizador
				query = "UPDATE estatisticautilizador SET resprespondidas=resprespondidas+1, respcertas=respcertas+1  WHERE username='" + player2 + "';";
				executeSQL(query);
				
				// Actualizar respostas certas na tabela jogo
				query = "UPDATE jogo SET respcertas2=respcertas2+1  WHERE id='" + intToString(game_id) + "';";
				executeSQL(query);
			} else  if(currAnswer[player2] == -1) {
				writeline( sockets[player2], "Não respondeu!\nA resposta certa era a: " + numToResp(i_correct));
			} else {
				writeline( sockets[player2], "Resposta errada!\nA resposta certa era a: " + numToResp(i_correct));
				
				// Actualizar a estatisticapergunta
				query = "UPDATE estatisticapergunta SET vezesquesaiu=vezesquesaiu+1 WHERE id_pergunta='" + intToString(questoes[i]) + "';";
				executeSQL(query);
				
				// Actualizar a estatisticautilizador
				query = "UPDATE estatisticautilizador SET resprespondidas=resprespondidas+1 WHERE username='" + player2 + "';";
				executeSQL(query);
			}
		}
				
		// Espera até todos os jogadores estarem prontos ou ter passado x segundos
		
		/***
			Falta uma função \ready que quando todos os jogadores a executarem avança para a próxima pergunta.
		***/
		if(i != nquestoes-1) {
			clock_t questionEnd_time = clock();
			elapsedTime = 0;
			
			do {
				clock_t curTime = clock();
				clock_t clockTicksTaken = curTime - questionEnd_time;
				elapsedTime = clockTicksTaken / (double) CLOCKS_PER_SEC;
			} while(elapsedTime < TIME_BETWEEN_QUESTIONS /* || Todos "ready" */);
		}
	}
	
	// Fim do jogo //
	writeline( sockets[criador], "O jogo terminou!");
	if(player1Presente)	writeline( sockets[player1], "O jogo terminou!");
	if(player2Presente)	writeline( sockets[player2], "O jogo terminou!");	
	
	currQuestion[game_id] = -1;
	
	/***    Estatistica utilizador criador     ***/

	// Actualizar a estatisticautilizador
	query = "UPDATE estatisticautilizador SET jogosefectuados=jogosefectuados+1 WHERE username='" + criador + "';";
	executeSQL(query);
	
	/***    Estatistica utilizador player1     ***/
	if(player1Presente) {
		// Actualizar a estatisticautilizador
		query = "UPDATE estatisticautilizador SET jogosefectuados=jogosefectuados+1 WHERE username='" + player1 + "';";
		executeSQL(query);
	}
	
	/***    Estatistica utilizador player2     ***/
	if(player2Presente) {
		// Actualizar a estatisticautilizador
		query = "UPDATE estatisticautilizador SET jogosefectuados=jogosefectuados+1 WHERE username='" + player2 + "';";
		executeSQL(query);
	}
	/***    End estatistica utilizador     ***/
	
	/*****
	
	
		Update da estatistica do utilizador do vencedor!
	
	
	*****/
	
	//*******************************
	// vamos ordenar os jogadores deste jogo, e imprimir as posicoes.
	//*********************************
		
	// Ver as respostas certas de cada jogador
	query = "SELECT respcertascriador, respcertas1, respcertas2 FROM jogo WHERE id = '" + intToString(game_id) + "';";
	res = executeSQL(query);
	
	string respostascertascriador = PQgetvalue(res, 0, 0);
	string respostascertasplayer1 = PQgetvalue(res, 0, 1);
	string respostascertasplayer2 = PQgetvalue(res, 0, 2);	
	
	// Ver qual é que tem mais respostas certas
	typedef vector< pair<int,string> > respjogadores;
	
	respjogadores respostasjogadores;
	
	respostasjogadores.push_back(make_pair(atoi(respostascertascriador.c_str()), criador));
  	respostasjogadores.push_back(make_pair(atoi(respostascertasplayer1.c_str()), player1));
  	respostasjogadores.push_back(make_pair(atoi(respostascertasplayer2.c_str()), player2));

  	sort(respostasjogadores.begin(), respostasjogadores.end());
  	
	int a = 1;
	
	respjogadores::reverse_iterator it = respostasjogadores.rbegin();

    string imprimir, imprimir1, pos;

    it = respostasjogadores.rbegin();

    // Empate entre os dois primeiros
    if((((it)->first == (it+1)->first) && ((it)->first != (it+2)->first)) && (player1Presente || player2Presente))
    {
        string jogador1 = respostasjogadores[2].second;
        string jogador2 = respostasjogadores[1].second;
        int respostaswinner = respostasjogadores[2].first;
        string jogador3 = respostasjogadores[0].second;
        int respostasloser = respostasjogadores[0].first;

        imprimir = "\nHouve um empate em 1º entre " + jogador1 + " e " + jogador2 + " com " + intToString(respostaswinner) + " respostas certas\n";

        // Vai haver agora um jogeo de desempate entre os dois primeiros jogadores! Ao jogador que se encontra na 3º posiçao pede-se que aguarde!\n Obrigado pela compreensão!\n\n";
       // imprimir1="O utilizador " + jogador3 + " ficou em 3º com " + intToString(respostasloser) + " respostas certas.\n";
		  if(jogador3.compare("")!=0 && jogador2.compare("")==0)
            imprimir1="O utilizador " + jogador3 + " ficou em 3º com " + intToString(respostasloser) + " respostas certas\n";
        imprimir+=imprimir1;
        writeline( sockets[criador], imprimir);
        if(player1Presente)
        		writeline(sockets[player1], imprimir);
        if(player2Presente)
        		writeline(sockets[player2], imprimir);
        
        string pos="\nPosicoes finais:\n";
        pos+="1º "+jogador1;
        if(jogador2.compare("")!=0)
           pos+= "\n1º "+jogador2;
        if(jogador3.compare("")!=0)
           pos+= "\n2º "+jogador3;
        cout<<pos<<endl;
			
        writeline( sockets[criador], pos);
        if(player1Presente) 
        		writeline( sockets[player1], pos);
        if(player2Presente)
        		writeline( sockets[player2], pos);
    }
    
    // Empate entre os dois últimos
    else if((((it+1)->first == (it+2)->first) && ((it)->first != (it+2)->first)) && (player1Presente || player2Presente))
    {
        cout<<endl<<"------------------"<<endl<<"2 ultimos"<<endl<<"------------------"<<endl;
        string jogador1 = respostasjogadores[2].second;
        int respostaswinner = respostasjogadores[2].first;
        string jogador2 = respostasjogadores[1].second;
        string jogador3 = respostasjogadores[0].second;
        int respostasloser = respostasjogadores[0].first;
        imprimir = "\nO utilizador " + jogador1 + " ficou em 1º com " + intToString(respostaswinner) + "respostas certas.\n";
        if(jogador3.compare("")!=0 && jogador2.compare("")!=0)//player1Presente && player2Presente)
            imprimir1="Houve um empate em 2º entre " + jogador2 + " e " + jogador3 + " com " + intToString(respostasloser) + " respostas certas.!\n";
            //\n Vai haver agora um jogeo de desempate entre os dois ultimos jogadores! Ao jogador vitorioso pede-se que aguarde!\n Obrigado pela compreensão!\n\n";
        if(jogador3.compare("")!=0 && jogador2.compare("")==0)
            imprimir1="O utilizador " + jogador3 + " ficou em 2º com " + intToString(respostasloser) + " respostas certas\n";
        if(jogador2.compare("")!=0 && jogador3.compare("")==0)
            imprimir1="O utilizador " + jogador2 + " ficou em 2º com " + intToString(respostasloser) + " respostas certas\n";
        imprimir+=imprimir1;
        writeline( sockets[criador], imprimir);
        if(player1Presente)
        		writeline(sockets[player1], imprimir);
        if(player2Presente)
        		writeline(sockets[player2], imprimir);
        
        pos="\nPosicoes finais:\n";
        pos+="1º "+jogador1;
        if(jogador2.compare("")!=0)
           pos+= "\n2º "+jogador2;
        if(jogador3.compare("")!=0)
           pos+= "\n2º "+jogador3;
        cout<<pos<<endl;
			
        writeline( sockets[criador], pos);
        if(player1Presente) 
        		writeline( sockets[player1], pos);
        if(player2Presente)
        		writeline( sockets[player2], pos);
    
    	  PGresult * venc = executeSQL("UPDATE jogo SET vencedor = '" + jogador1 + "' WHERE id = " + intToString(game_id) + ";");
    }

    // Empate entre todos
    else if((((it)->first == (it+1)->first) && ((it)->first == (it+2)->first)) && (player1Presente || player2Presente))
    {
        //string jogador1 = respostasjogadores[2].second;
        int respostaswinner = respostasjogadores[0].first;
        //string jogador2 = respostasjogadores[1].second;
        //string jogador3 = respostasjogadores[0].second;
        //int respostasloser = respostasjogadores[0].first;
        //string imprimir = "\nHouve um empate entre todos os jogadores, de onde obtiveram " + intToString(respostaswinner) + "respostas certas.\n\n Vai haver aogra um jogo de desempate!\n\n";
		writeline( sockets[criador], imprimir);
		if(player1Presente)
        writeline( sockets[player1], imprimir);
		if(player2Presente)
        writeline( sockets[player2], imprimir);
        imprimir="\nPosicoes finais:\n";
        imprimir+="1º"+criador;
        if(player1Presente)
            imprimir+="\n1º"+player1;
        if(player2Presente)
            imprimir+="\n1º"+player2+"\n";
        cout<<imprimir;
        writeline( sockets[criador], imprimir);
        if(player1Presente)
           writeline( sockets[player1], imprimir);
        if(player2Presente)
         	writeline( sockets[player2], imprimir);
    }
    
    // O criador esta a jogar sozinho
    else if(!player1Presente && !player2Presente) {
    	writeline( sockets[criador], "Não tem classificaçao por jogar sozinho, mas acabou com " + intToString(it->first) + " respostas certas.");
    	
    }
    // Lugares atribuidos a jogadores com numero de respostas diferentes
    else {
    	it = respostasjogadores.rbegin();
  		  		
  		if(((player1Presente && !player2Presente) || (!player1Presente && player2Presente)))
  		{
  			imprimir="O utilizador " + it->second + " ficou em 1º com " + intToString(it->first) + " respostas certas";
 			imprimir+="O utilizador " + (it+1)->second + " ficou em 2º com " + intToString((it+1)->first) + " respostas certas";
 			
  			imprimir1="\nPosicoes finais:\n";
  			imprimir1+="1º"+it->second;
  			imprimir1+="2º"+(it+1)->second;
  			
  			cout<<imprimir+"\n"+imprimir1;
  			writeline( sockets[criador], imprimir+"\n"+imprimir1);
  			if(player2Presente)
  				writeline( sockets[player2], imprimir+"\n"+imprimir1);
  			if(player1Presente)
  				writeline( sockets[player1], imprimir+"\n"+imprimir1);
  		
  			executeSQL("UPDATE jogo SET vencedor = '" + it->second + "' WHERE id = " + intToString(game_id) + ";");
  			executeSQL("UPDATE estatisticautilizador SET jogosganhos=jogosganhos+1 WHERE username='" + it->second + "';");
  		}
  		
 		if(player1Presente && player2Presente)
  		{
  			imprimir="O utilizador " + it->second + " ficou em 1º com " + intToString(it->first) + " respostas certas";
 			imprimir+="O utilizador " + (it+1)->second + " ficou em 2º com " + intToString((it+1)->first) + " respostas certas";
 			imprimir+="O utilizador " + (it+2)->second + " ficou em 3º com " + intToString((it+2)->first) + " respostas certas";
 			
  			imprimir1="\nPosicoes finais:";
  			imprimir1+="\n1º"+it->second;
  			imprimir1+="\n2º"+(it+1)->second;
  			imprimir1+="\n3º"+(it+2)->second;
  			
  			cout<<imprimir+"\n"+imprimir1<<endl;
  			writeline( sockets[criador], imprimir+"\n"+imprimir1);
			writeline( sockets[player2], imprimir+"\n"+imprimir1);
			writeline( sockets[player1], imprimir+"\n"+imprimir1);
  		
  			executeSQL("UPDATE jogo SET vencedor = '" + it->second + "' WHERE id = " + intToString(game_id) + ";");
  			executeSQL("UPDATE estatisticautilizador SET jogosganhos=jogosganhos+1 WHERE username='" + it->second + "';");
  		}
	}
	
	emJogo[criador] = -1;
	emJogo[player1] = -1;
	emJogo[player2] = -1;
    
    jogo_criado.erase(criador);
}
/***			end jogo			***/


void fiftyfifty_c(int socketid, string args)
{	
	if(!waitingForAnswer[usernames[socketid]]) {
		writeline( socketid, "Comando não disponível!");
		return;
	}
	
	// Indicar que o jogador quer a ajuda. Para isso pomos a resposta a 4=E.
	currAnswer[usernames[socketid]] = 4;
}

void ranking_c(int socketid, string args)
{
	
	char tempStr[100];
  		
	// Ir buscar cenas?
	string query = "SELECT username, (respcertas*10000/resprespondidas), resprespondidas FROM estatisticautilizador ORDER BY (respcertas*10000/resprespondidas) DESC, resprespondidas DESC;";
	PGresult * res = executeSQL(query);
	
	writeline(socketid, "\nRanking dos jogadores\n\n");
	writeline(socketid, "\tUsername\t\t\t\tRatio\tRespostas");
	
	for (int row = 0; row < PQntuples(res); row++) {
		
		sprintf(tempStr, "\t%i.%-32s\t%02.02f%%\t%d", row+1, PQgetvalue(res, row, 0), (float) stringToInt(PQgetvalue(res, row, 1))/100, stringToInt( PQgetvalue(res, row, 2)));
		//string str = "\t" + intToString(row+1) + '.' + PQgetvalue(res, row, 0) + "\t" + PQgetvalue(res, row, 1) + "\t" + PQgetvalue(res, row, 2);
    	writeline( socketid, tempStr);
    }
	
	// Imprimir cenas?
	
	// Mais cenas...
}

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
	if (!(waitingForAnswer.find(usernames[socketid]) != waitingForAnswer.end())) {
		writeline(socketid, "Não é suposto responder a nada agora.\n");
		return;
	}
	
	// Verificar se o cliente está num jogo e está a ser aguardada uma resposta
	if (waitingForAnswer.at(usernames[socketid]) == false) {
		writeline(socketid, "Já respondeu, não pode voltar a responder.\n");
		return;
	}
	
	// Verificar se a resposta é válida
	if(resposta != "A" && resposta != "B" && resposta != "C" && resposta != "D") {
		writeline(socketid, "Resposta inválida.\n Por favor seleccione a letra correspondente à opção.\n");
		return;
	}	
	
	// Guardar a opção seleccionada pelo jogado na variável destinada a este efeito
    if( resposta == "A" || resposta == "a")	currAnswer[usernames[socketid]] = 0;
    else if( resposta == "B" || resposta == "b")	currAnswer[usernames[socketid]] = 1;
    else if( resposta == "C" || resposta == "c")	currAnswer[usernames[socketid]] = 2;
    else if( resposta == "D" || resposta == "d")	currAnswer[usernames[socketid]] = 3;

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
	} else if(user =="\0" || pass=="\0") {
		writeline(socketid, "Introduziu elementos a menos.\n");
		return;
	} else if (sockets.find(user) != sockets.end()) {
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
		
		currAnswer[user] = -1;
		emJogo[user] = -1;
		currAskuser[user] = "";
	} else {
		writeline(socketid, "Os dados de login não estão correctos!\n Tente novamente.");
		return;
	}
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

void say_c(int socketid, string args)
{

    istringstream iss(args);
    string user, mensagem, palavra, falha, remetente;

    getline(iss, user, ' ');

	if(!userexists(user)) {
		writeline(socketid, "Utilizador não encontrado!\n");
        return;
    } else if(!islogged(sockets[user])) {
        writeline(socketid, "Erro, o utilizador "+user+"não está online neste momento!\n");
        return;
    } else if(!islogged(socketid)) {
		writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
		return;
	} else if(user =="\0") {
        writeline(socketid, "Introduziu elementos a menos.\n");
        return;
    }

    while (iss >> palavra)
        mensagem += palavra + ' ';

    writeline(socketid, "enviado");
    writeline(sockets[user], "\n" + usernames[socketid] + " disse: " + mensagem);
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

	if(!userexists(user)) {
		writeline(socketid, "O username selecionado não existe!\n");
		return;
	} else if(!user.compare(desafiador)) {
		writeline(socketid, "Não se pode desafiar a si mesmo!\n");
		return;
	} else if(!islogged(sockets[user])) {
		writeline(socketid, "O user especificado não se encontra online. Tente mais tarde!\n");
		return;
	}
	//else
		//writeline(sockets[user],"O jogador " + usernames[socketid] + " convidou-o para iniciar um jogo.\nPara aceitar escreva: \\accept, para rejeitar escreva: \\decline\n");

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
             else{
                 writeline(socketid, "Já convidou 4 jogadores!\n");
				 return;
				 }
				
         }
		}
	}
			writeline(sockets[user],"O jogador " + usernames[socketid] + " convidou-o para iniciar um jogo.\n Para aceitar escreva: \\accept, para rejeitar escreva: \\decline\n");
}

/**
*	args : Nome do utilizador que fez o convite
*
*/
void accept_c(int socketid, string args)
{
	istringstream iss(args);
    string user, falha;
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

	if(!(jogo_criado.find(user) != jogo_criado.end())) {
		writeline(socketid, "Jogo não disponível.\n");
		return;
	}
	
	if(!userexists(user)) {
		writeline(socketid, "O nome do utilizador não está correcto.\n");
		return;
	}
	
	string id = intToString(jogo_criado[user]);
	
	/*** Funcionalidade não implementada: data e hora de inicio do jo
	
	// Ver se o jogo já começou
	PGresult* res1 = executeSQL("SELECT dataehora FROM jogo WHERE id=" + intToString(jogo_criado[user]) + ";");
	string timestart = PQgetvalue(res1, 0, 0);
	
	if(timestart != "")
	{
		writeline(socketid, "O jogo já começou! Não é mais possivel aceitar o convite\n");
		return;
	}
	
	***/
	
	PGresult* res1 = executeSQL("SELECT convidado1, convidado2, convidado3, convidado4 FROM jogo WHERE id=" + id + ";");
	
	string c1 = PQgetvalue(res1, 0, 0);
	string c2 = PQgetvalue(res1, 0, 1);
	string c3 = PQgetvalue(res1, 0, 2);
	string c4 = PQgetvalue(res1, 0, 3);
	
	if(c1.compare(usernames[socketid]) && c2.compare(usernames[socketid]) && c3.compare(usernames[socketid]) && c4.compare(usernames[socketid])) 
	{
		writeline(socketid, "Não foi convidado para este jogo\n");
		return;
	}
	
	res1 = executeSQL("SELECT player1 FROM jogo WHERE id=" + id + ";");
	//cout<<"query:"<<"SELECT player1 FROM jogo WHERE id=" + id + ";";
	c1 = PQgetvalue(res1, 0, 0);
	
	if(c1=="") {
		executeSQL("UPDATE jogo SET player1 = '" + usernames[socketid] + "' WHERE id = " + id + ";");
		writeline(socketid, "Tudo Pronto! Espere pelo inicio do jogo!\n");
		writeline(sockets[user], "O utilizador '"+ usernames[socketid] +"' aceitou jogar no seu jogo!.\n");
		return;
	}
	else {
		res1 = executeSQL("SELECT player2 FROM jogo WHERE id="+id+";");
		c2= PQgetvalue(res1, 0, 0);
		if(c2=="")
		{
			executeSQL("UPDATE jogo SET player2 = '" + usernames[socketid] + "' WHERE id = " + id + ";");
			writeline(socketid, "Tudo Pronto! Espere pelo inicio do jogo!\n");
			writeline(sockets[user], "O utilizador '"+ usernames[socketid] +"' aceitou jogar no seu jogo!.\n");
			return;
		}
		else
		{
			writeline(socketid, "Vagas para o Jogo preenchidas!\n");
			return;
		}
	}
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

	else {
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
			cout << "O servidor foi fechado pelo administrador: " << usernames[socketid] << endl;
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
							
							writeline(socketid, "Os utilizadores| "+ it->first +" |estão online como Administradores!\n");	
							}
					
					
				//else
				//	writeline(socketid, "Nãp há utilizadores que estão online como Administradores!\n");
						
				}}
	
	else
				writeline(socketid, "Precisa de fazer login para executar o comando!\n");
}

/**
*	Funcao \ask
*/
void ask_c(int socketid, string args)
{
	 if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
     }
     
     if(!waitingForAnswer[usernames[socketid]]) {
     	  writeline(socketid, "Comando não disponível\n");
     	  return;
     }
     
     string askuser, user, ask1, ask2, ask3, ask4;
     istringstream iss(args);
	getline(iss, askuser, ' ');

	if(askuser == "") {
		writeline(socketid, "Tem de escolher o jogador ao qual pertende perguntar.\n");
		return;	
	} else if (!userexists(askuser)) {
		writeline(socketid, "Utilizador não encontrado.\n");
		return;	
	}
    
    cout << "cenas verificadas" << endl;
    
	user = usernames[socketid];
     
	PGresult* res = executeSQL("SELECT userajuda1, userajuda2, userajuda3, userajuda4  FROM ajudautilizadores WHERE useremjogo = '" + user + "';");
     
     ask1 = PQgetvalue(res, 0, 0);
     ask2 = PQgetvalue(res, 0, 1);
     ask3 = PQgetvalue(res, 0, 2);
     ask4 = PQgetvalue(res, 0, 3);
     
     if((ask1.compare(askuser) && ask2.compare(askuser) && ask3.compare(askuser) && ask4.compare(askuser))) {
     	writeline(socketid, "Este utilizador não pertence à sua lista!\n");
        return;
     }

	cout << "base de dados" << endl;
     
	// Verificar se o askuser não está num Jogo
	if (!islogged(sockets[askuser])) {
		writeline(socketid, "O utilizador " + askuser + " não se encontra online.\n");
		return;	
	} else if(emJogo[askuser] != -1) {
		writeline(socketid, "O jogador " + askuser + " encontra-se num jogo.\nNão poderá ajudar...");
        return;
	} else if (currAskuser.count(askuser) > 1) {
		writeline(socketid, "O utilizador " + askuser + " está a ajudar alguém e não poderá o poderá ajudar.\n");
		return;	
	}
    
    cout << "a definir o 5" << endl;
    
    // Ajuda ask (5)
	currAnswer[usernames[socketid]] = 5;
	currAskuser[usernames[socketid]] = askuser;
	
	cout << "so far, so good" << endl;
}

void addaskuser_c(int socketid, string args)
{
    if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    } else if(emJogo[usernames[socketid]] == true) {
		writeline(socketid, "Não pode executar este comando a meio de um jogo!\n");
		return;
    }

    string askuser, falha, query;
    istringstream iss(args);

    getline(iss, askuser, ' ');
    getline(iss, falha, ' ');

    if(falha!="\0") {
		writeline(socketid, "ERRO: Introduziu argumentos a mais!");
		return;
	} else if(askuser=="\0") {
		writeline(socketid, "ERRO: Não introduziu nenhum utilizador!");
		return;
	} else if(!userexists(askuser)) {
		writeline(socketid, "Utilizador não existente!");
		return;
	} else if(askuser == usernames[socketid]) {
		writeline(socketid, "Não se pode adicionar a si mesmo!");
		return;
	}
	
	// Ver quais os userajudas livres na base de dados
	string ask1, ask2, ask3, ask4;
	
	PGresult* res = executeSQL("SELECT userajuda1, userajuda2, userajuda3, userajuda4 FROM ajudautilizadores WHERE useremjogo='" + usernames[socketid] + "';");
	//cout<<"query:"<<"SELECT player1 FROM jogo WHERE id=" + id + ";";
	ask1 = PQgetvalue(res, 0, 0);
	ask2 = PQgetvalue(res, 0, 1);
	ask3 = PQgetvalue(res, 0, 2);
	ask4 = PQgetvalue(res, 0, 3);
	
	if(askuser == ask1 || askuser == ask2 || askuser == ask3 || askuser == ask4 ) {
		writeline(socketid, "O utilizador " + askuser + " já faz parte da sua lista!\n");
		return;
	}
	
	if(ask1 == "") {
		executeSQL("UPDATE ajudautilizadores SET userajuda1 = '" + askuser + "' WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "Jogador adicionado à sua lista!\n");
		return;
	} else if(ask2 == "") {
		executeSQL("UPDATE ajudautilizadores SET userajuda2 = '" + askuser + "' WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "Jogador adicionado à sua lista!\n");
		return;
	} else if(ask3 == "") {
		executeSQL("UPDATE ajudautilizadores SET userajuda3 = '" + askuser + "' WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "Jogador adicionado à sua lista!\n");
		return;
	} else if(ask4 == "") {
		executeSQL("UPDATE ajudautilizadores SET userajuda4 = '" + askuser + "' WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "Jogador adicionado à sua lista!\n");
		return;
	}
	
	// Se chegou aqui é porque já não tinha mais askusers disponiveis
	writeline(socketid, "Os seus 4 utilizadores de ajuda já estão definidos!\nPor favor, remova algum para adicionar outro...");
	return;	
}

/**
*
*/
void removeaskuser_c(int socketid, string args)
{
	if(!islogged(socketid)) {
        writeline(socketid, "Precisa de estar logado para executar esse comando!\n");
        return;
    }

    string raskuser, falha, query;
    istringstream iss(args);

    getline(iss, raskuser, ' ');
    getline(iss, falha, ' ');

    if(falha!="\0") {
		writeline(socketid, "ERRO: Introduziu argumentos a mais!");
		return;
	} else if(raskuser=="\0") {
		writeline(socketid, "ERRO: Não introduziu nenhum utilizador!");
		return;
	} else if(!userexists(raskuser)) {
		writeline(socketid, "Utilizador não existente!");
		return;
	}

	// Ver o utilizador faz parte da lista
	string ask1, ask2, ask3, ask4;
	
	PGresult* res = executeSQL("SELECT userajuda1, userajuda2, userajuda3, userajuda4 FROM ajudautilizadores WHERE useremjogo='" + usernames[socketid] + "';");
	//cout<<"query:"<<"SELECT player1 FROM jogo WHERE id=" + id + ";";
	ask1 = PQgetvalue(res, 0, 0);
	ask2 = PQgetvalue(res, 0, 1);
	ask3 = PQgetvalue(res, 0, 2);
	ask4 = PQgetvalue(res, 0, 3);
	
	if(ask1 == raskuser) {
		executeSQL("UPDATE ajudautilizadores SET userajuda1 = NULL WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "O utilizador " + raskuser + " foi removido da sua lista!");
		return;
	} else if(ask2 == raskuser) {
		executeSQL("UPDATE ajudautilizadores SET userajuda2 = NULL WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "O utilizador " + raskuser + " foi removido da sua lista!");
		return;
	} else if(ask3 == raskuser) {
		executeSQL("UPDATE ajudautilizadores SET userajuda3 = NULL WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "O utilizador " + raskuser + " foi removido da sua lista!");
		return;
	} else if(ask4 == raskuser) {
		executeSQL("UPDATE ajudautilizadores SET userajuda4 = NULL WHERE useremjogo = '" + usernames[socketid] + "';");
		writeline(socketid, "O utilizador " + raskuser + " foi removido da sua lista!");
		return;
	}
	
	writeline(socketid, "O utilizador " + raskuser + " não faz parte da sua lista!");
	return;
}

/**
*
*/
void info_c(int socketid, string args)
{
	string user;
	istringstream iss(args);
		
	getline(iss, user, ' ');
	
	
	cout << user << endl;
	
	if(!userexists(user)) {
		writeline(socketid, "O jogador " + user + " não foi encontrado!\n");
		return;
	}
	
	cout << "A obter dados" << endl;
	
	PGresult* res1 = executeSQL("SELECT respcertas, resprespondidas, jogosefectuados, jogosganhos FROM estatisticautilizador WHERE username = '" + user + "';");
		 
	string respostascertas = PQgetvalue(res1, 0, 0);
	string respostasrespondidas = PQgetvalue(res1, 0, 1);
	string jogosefectuados = PQgetvalue(res1, 0, 2);
	string jogosganhos = PQgetvalue(res1, 0, 3);
	
	string str = "O utilizador " + user + " até ao momento possuí " + jogosefectuados + " jogos efectuados no sistema!\nDe onde obteve " + jogosganhos + " jogos ganhos.\nDe um total de " + respostasrespondidas + " respostas respondidas, acertou " + respostascertas + ".\n";
	
	cout << str << endl;
	
	writeline( socketid, str);
}

/**
*	utils: transforma um numero numa resposta (string)
*/
string numToResp(int i)
{
	if(i==0)
		return "A";	
	if(i==1)
		return "B";
	if(i==2)
		return "C";
	if(i==3)
		return "D";
	else 
		return "?";
}

int stringToInt(string str)
{
	istringstream buffer(str);
	int value;
	
	buffer >> value;

	return value;
}

/*
void showaskusers_c(int socketid, string args)
{
}
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
}*/

/*
void showaskusers_c(int socketid, string args)
{
}
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
