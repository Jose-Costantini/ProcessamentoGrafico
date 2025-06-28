#Prova de Grau B.

A prova consiste em criar um jogo 2D com visão isométrica (estratégia diamond view). 

No jogo desenvolvido, controlamos um pequeno marinheiro que coleta conchas na praia. Há 10 conchas no total para serem coletadas.

A maré muda a cada 30 segundos. Na maré alta, o personagem fica impedido de coletar as conchas submersas. Caso o personagem seja pego pela maré, ele  ficará preso até a maré baixar novamente, a menos que o tile imediato ao seu lado seja terra firme. Nesse caso, ele consegue voltar para a praia, mas ainda deve aguardar a maré baixar para poder voltar a coletar as conchas.

As conchas são colocadas em posições aleatórias com a função rand. Quando coletadas, as conchas somem do mapa.

A coleta das conchas é feita com a comparação da posição do personagem no mapa e a posição de cada concha. Caso sejam iguais, a concha é coletada e o jogador recebe um ponto.

Ao coletar todas as 10 conchas, uma mensagem aparece no console parabenizando o jogador.

