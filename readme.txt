g++ trabalho.cpp -o trabalho command_lib.cpp database.cpp -pthread -lpq
./trabalho

Tarefas:
 * avan�ar para a proxima pergunta quando todos tiverem respondido;
 * retirar o utilizador do map quando o seu socket se fecha (e.g. quando o utilizador fecha o terminal sem fazer \exit);
 ~ implementar funcao \ask;
 * corrigir o comando \exit;
 - impedir o jogador de executar certos comandos enquanto est� num jogo;
 - corrigir apresentacao dos resultados no fim do jogo;
                   *(OK com 1 bug) empate entre os dois ultimos parece tar ok no entanto ele imprimime que o jogo terminou... 2 x 
 - criar um comando para anular um jogo;
 - implementar o comando \info "player";
 - implementar comando "\ready" para avan�ar para a proxima pergunta quando todos estivessem prontos;
 - utilizar o bool "comecado" na tabela jogo para diferenciar um jogo n�o existente de um j� come�ado quando alguem faz \accept. (Para isso tem de se voltar a p�r "jogo_criado.erase(criador);" no fim do jogo e n�o no inicio.)
 - avan�ar automaticamente para o fim do jogo a seguir a ultima questao;
 - verificar se todos argumentos para todas a fun��es s�o verificados;
 - retirar dataehora da tabela jogo (ou implementar esta funcionalidade [menos aconcelh�vel]);
 - optimizar o aspeto de quando o jogo arranca;
 - ignorar as estatisticas se um jogador utilizar uma ajuda;
 - Usersready_c ver esta fun�ao (desatualizada)
 - Fazer Manual completo

Bugs conhecidos:
 - ...

Potenciais bugs:
 - ...

Notas:
 - Agora ja e possivel usar ^C no servidor sem ter de desligar os utilizadores;
 - Linha 14/17: tirei o come�ado do script sql... ja implementei a atualiza�ao da data e hora quando o jogo e come�ado logo, quando um jogo nao foi come�ado, fazer ("SELECT dataehora FROM jogo WHERE id="game_id) ... = "" (string sem nada la dentro, faz se um .compare
