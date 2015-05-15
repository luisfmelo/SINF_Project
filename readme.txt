g++ trabalho.cpp -o trabalho command_lib.cpp database.cpp -pthread -lpq
./trabalho

Tarefas:
 - verificar se todos argumentos para todas a funções são verificados;
 - corrigir o comando \exit;
 - corrigir o comando \shutdown;
 - implementar comando "\ready" para avançar para a próxima pergunta quando todos estivessem prontos;
 - utilizar o bool "comecado" na tabela jogo para diferenciar um jogo não existente de um já começado quando alguem faz \accept. (Para isso tem de se voltar a pôr "jogo_criado.erase(criador);" no fim do jogo e não no inicio.)
 - retirar dataehora da tabela jogo (ou implementar esta funcionalidade [menos aconcelhável]);

Bugs conhecidos:
 - ...

Potênciais bugs:
 - ...