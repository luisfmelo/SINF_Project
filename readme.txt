g++ trabalho.cpp -o trabalho command_lib.cpp database.cpp -pthread -lpq
./trabalho

Tarefas:
 - verificar se todos argumentos para todas a fun��es s�o verificados;
 - corrigir o comando \exit;
 - corrigir o comando \shutdown;
 - implementar comando "\ready" para avan�ar para a pr�xima pergunta quando todos estivessem prontos;
 - utilizar o bool "comecado" na tabela jogo para diferenciar um jogo n�o existente de um j� come�ado quando alguem faz \accept. (Para isso tem de se voltar a p�r "jogo_criado.erase(criador);" no fim do jogo e n�o no inicio.)
 - retirar dataehora da tabela jogo (ou implementar esta funcionalidade [menos aconcelh�vel]);

Bugs conhecidos:
 - ...

Pot�nciais bugs:
 - ...