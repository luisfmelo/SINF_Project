g++ trabalho.cpp -o trabalho command_lib.cpp database.cpp -pthread -lpq
./trabalho

ATEN�AO:
 - rever todas as fun�oes para ser coerente
 - fazer do estilo change permissions... avisar servidor se alguem tenta usar comando ilegal,...
 - e se num comando der mais argumentos do que os devidos?
PROBLEMAS
 - fun�ao exit ta com erro

Mais bugs e quest�es exist�nciais:
 - quando algu�m fecha o terminal o seu username/socket n�o � retirado do map;
 - num jogo devia ter um comando "\ready" para avan�ar para a primeira/pr�xima pergunta quando todos estivessem prontos ou tivessem passado 15 segundos;
 - faz sentido um jogador poder criar outro jogo sem ter jogado o anterior?
 - um jogador pode aceitar jogos para os quais n�o foi convidado;
 - o n� de convites deve ser igual ao n� de jogadores

v9.0(Lu�s) 10-05-2015  
Fun��es (com bastantes condi�oes):
-help:				OK
-register:			OK
-identify:    			OK	esquecer user guest
-login:    			OK	
-logout:			OK 	acrescentar ao pdf
-resetpassword:			OK
-changepassword:		OK
-changeusername:		--	ERRO: tenho direito mas e preciso mudar qualquer coisa porque e foreign key... acho que � aquelas tretas da CASCADE




v8.0 (Lu�s) 03-05-2015
Updates:
 - fun��es: \deletequestions, \changepermission,  j� funcionais.
 - preciso ver de prioridades 
 - acrescentar fun��o \decline (ja nao e preciso)


v6.0 (Lu�s) 01-05-2015
Updates:
 - fun��es: \changepassword, \changeutilizador, \question, \showallquestions,  j� funcionais.
 - quando inicia limpa terminal ^^

v5.0 (David) 30-04-2015

Updates:
 - fun��es: \register, \login, \logout e \resetpassword j� funcionais;
 - fun��es listusers, islogged, isadmin e userexists implementadas;
 - n� de sockets em espera aumentado para 10;
 - a cada fun��o foi adicionada um par�mtro do 'id' do socket do utilizador;

Problemas conhecidos:
 - alguns dos bugs est�o assinalados no local do c�digo ende poder�o ocorrer;

v2.0 (David)

Updates:
 - fun��o \help j� imprime no cliente e n�o no servidor;
 - a cada fun��o comando tem de ser passado por par�metro um ostringstream para depois ser enviada ao cliente no final da fun��o "readcomand()";
 - fun��o shutdown implementada do lado do servidor;
 - fun��o exit implementada do lado do cliente (requer revis�o);
 - fun��o shutdown implementada do lado do cliente (requer revis�o);

Problemas conhecidos:
 - fun��o shutdown n�o verifica permiss�es do cliente;
