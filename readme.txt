g++ -pthread -lpq trabalho.cpp -o trabalho command_lib.cpp database.cpp
./trabalho

ATENÇAO:
-rever todas as funçoes para ser coerente
-fazer do estilo change permissions... avisar servidor se alguem tenta usar comando ilegal,...
-e se num comando der mais argumentos do que os devidos?
-funçao islogged... se fizermos telnet ele assume que estamos logados...
PROBLEMAS
 - por cada jogo criamos uma tabela? com todas as infos?
 - nao estou a conseguir logar em 2 contas ao mesmo tempo
 - funçao exit ta com erro
 - a certo ponto um user começa a receber os comandos que outro user mandou
 - mudança bem sucedida... coisas assim, v er isto, apenas pode aparecer isto quando nao houve erros nas funçoes do libpq





v9.0(Luís) 10-05-2015  
Funções (com bastantes condiçoes):
-help:				OK
-register:			OK
-identify:    			OK	esquecer user guest
-login:    			--	E SE O USER JA ESTIVER LOGADO?
-logout:			OK 	acrescentar ao pdf
-resetpassword:			OK
-changepassword:		OK
-changeusername:		--	ERRO: tenho direito mas e preciso mudar qualquer coisa porque e foreign key... acho que é aquelas tretas da CASCADE



v8.0 (Luís) 03-05-2015
Updates:
 - funções: \deletequestions, \changepermission,  já funcionais.
 - preciso ver de prioridades 
 - acrescentar função \decline (ja nao e preciso)


v6.0 (Luís) 01-05-2015
Updates:
 - funções: \changepassword, \changeutilizador, \question, \showallquestions,  já funcionais.
 - quando inicia limpa terminal ^^

v5.0 (David) 30-04-2015

Updates:
 - funções: \register, \login, \logout e \resetpassword já funcionais;
 - funções listusers, islogged, isadmin e userexists implementadas;
 - nº de sockets em espera aumentado para 10;
 - a cada função foi adicionada um parâmtro do 'id' do socket do utilizador;

Problemas conhecidos:
 - alguns dos bugs estão assinalados no local do código ende poderão ocorrer;

v2.0 (David)

Updates:
 - função \help já imprime no cliente e não no servidor;
 - a cada função comando tem de ser passado por parâmetro um ostringstream para depois ser enviada ao cliente no final da função "readcomand()";
 - função shutdown implementada do lado do servidor;
 - função exit implementada do lado do cliente (requer revisão);
 - função shutdown implementada do lado do cliente (requer revisão);

Problemas conhecidos:
 - função shutdown não verifica permissões do cliente;
