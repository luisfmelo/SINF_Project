g++ -pthread -lpq trabalho.cpp -o trabalho command_lib.cpp database.cpp
./trabalho

ATEN�AO:
-rever todas as fun�oes para ser coerente
-fazer do estilo change permissions... avisar servidor se alguem tenta usar comando ilegal,...
-e se num comando der mais argumentos do que os devidos?
-fun�ao islogged... se fizermos telnet ele assume que estamos logados...
PROBLEMAS
 - por cada jogo criamos uma tabela? com todas as infos?
 - nao estou a conseguir logar em 2 contas ao mesmo tempo
 - fun�ao exit ta com erro
 - a certo ponto um user come�a a receber os comandos que outro user mandou
 - mudan�a bem sucedida... coisas assim, v er isto, apenas pode aparecer isto quando nao houve erros nas fun�oes do libpq





v9.0(Lu�s) 10-05-2015  
Fun��es (com bastantes condi�oes):
-help:				OK
-register:			OK
-identify:    			OK	esquecer user guest
-login:    			--	E SE O USER JA ESTIVER LOGADO?
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
