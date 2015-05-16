g++ trabalho.cpp -o trabalho command_lib.cpp database.cpp -pthread -lpq
./trabalho

Tarefas:
 * avançar para a proxima pergunta quando todos tiverem respondido;
 * retirar o utilizador do map quando o seu socket se fecha (e.g. quando o utilizador fecha o terminal sem fazer \exit);
 ~ implementar funcao \ask;
 * corrigir o comando \exit;
 - impedir o jogador de executar certos comandos enquanto está num jogo;
 - corrigir apresentacao dos resultados no fim do jogo;
                   *(OK com 1 bug) empate entre os dois ultimos parece tar ok no entanto ele imprimime que o jogo terminou... 2 x 
 - criar um comando para anular um jogo;
 - implementar o comando \info "player";
 - implementar comando "\ready" para avançar para a proxima pergunta quando todos estivessem prontos;
 - utilizar o bool "comecado" na tabela jogo para diferenciar um jogo não existente de um já começado quando alguem faz \accept. (Para isso tem de se voltar a pôr "jogo_criado.erase(criador);" no fim do jogo e não no inicio.)
 - avançar automaticamente para o fim do jogo a seguir a ultima questao;
 - verificar se todos argumentos para todas a funções são verificados;
 - retirar dataehora da tabela jogo (ou implementar esta funcionalidade [menos aconcelhável]);
 - optimizar o aspeto de quando o jogo arranca;
 - ignorar as estatisticas se um jogador utilizar uma ajuda;
 - Usersready_c ver esta funçao (desatualizada)
 - Fazer Manual completo

Bugs conhecidos:
 - ...

Potenciais bugs:
 - ...

Notas:
 - Agora ja e possivel usar ^C no servidor sem ter de desligar os utilizadores;
 - Linha 14/17: tirei o começado do script sql... ja implementei a atualizaçao da data e hora quando o jogo e começado logo, quando um jogo nao foi começado, fazer ("SELECT dataehora FROM jogo WHERE id="game_id) ... = "" (string sem nada la dentro, faz se um .compare
