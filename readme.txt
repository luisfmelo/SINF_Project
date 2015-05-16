g++ trabalho.cpp -o trabalho command_lib.cpp database.cpp -pthread -lpq
./trabalho

Tarefas:
 * avan�ar para a proxima pergunta quando todos tiverem respondido;
 * retirar o utilizador do map quando o seu socket se fecha (e.g. quando o utilizador fecha o terminal sem fazer \exit);
 ~ implementar funcao \ask;
 * corrigir o comando \exit;
 - impedir o jogador de executar certos comandos enquanto est� num jogo;
 - implentar mutexes;
 - corrigir apresentacao dos resultados no fim do jogo;
 - criar um comando para anular um jogo;
 - implementar o comando \info "player";
 - implementar comando "\ready" para avan�ar para a proxima pergunta quando todos estivessem prontos;
 - utilizar o bool "comecado" na tabela jogo para diferenciar um jogo n�o existente de um j� come�ado quando alguem faz \accept. (Para isso tem de se voltar a p�r "jogo_criado.erase(criador);" no fim do jogo e n�o no inicio.)
 - avan�ar automaticamente para o fim do jogo a seguir a ultima questao;
 - verificar se todos argumentos para todas a fun��es s�o verificados;
 - retirar dataehora da tabela jogo (ou implementar esta funcionalidade [menos aconcelh�vel]);
 - optimizar o aspeto de quando o jogo arranca;
 - ignorar as estatisticas se um jogador utilizar uma ajuda;
 - o utilizador que ajuda n�o podera executar nenhum outro comando enquando nao ajudar?;
 - trocar constantes por #defines;

Bugs conhecidos:
 - ...

Potenciais bugs:
 - ...

Notas:
 - Agora ja e possivel usar ^C no servidor sem ter de desligar os utilizadores;