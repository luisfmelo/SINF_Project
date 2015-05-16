--Criação das tabelas

create table utilizador (
	username varchar(32) PRIMARY KEY,
	password varchar(64),
	permissao integer default 1
);

create table estatisticautilizador (
	id SERIAL PRIMARY KEY,
	respcertas integer default 0,
	resprespondidas integer default 0,
	jogosefectuados integer default 0,
	jogosganhos integer default 0,
	username varchar(32)  REFERENCES utilizador NOT NULL UNIQUE
);
 
create table ajudautilizadores (
	id SERIAL PRIMARY KEY,
	userajuda1 varchar(32) REFERENCES utilizador,
	userajuda2 varchar(32) REFERENCES utilizador,	
	userajuda3 varchar(32) REFERENCES utilizador,	
	userajuda4 varchar(32) REFERENCES utilizador,
	useremjogo varchar(32) REFERENCES utilizador NOT NULL UNIQUE
);

create table perguntas (
	id SERIAL PRIMARY KEY,
	questao text,
	respcerta text,
	resperrada1 text,
	resperrada2 text,
	resperrada3 text,
	criador varchar(32) REFERENCES utilizador
);

create table estatisticapergunta (
	id SERIAL PRIMARY KEY,
	respostascertas integer default 0,
	vezesquesaiu integer default 0,
	id_pergunta integer  REFERENCES perguntas NOT NULL UNIQUE
);

create sequence id_jogo_seq;
create table jogo (
	id integer DEFAULT nextval('id_jogo_seq') PRIMARY KEY,
	nperguntas integer,
	duracao integer,
	criador varchar(32) REFERENCES utilizador NOT NULL,
	dataehora timestamp,
	vencedor varchar(32) REFERENCES utilizador,
	respcertascriador integer default 0,
	player1 varchar(32) REFERENCES utilizador,
	respcertas1 integer default 0,
	player2 varchar(32) REFERENCES utilizador,
	respcertas2 integer default 0,
	fiftyC BOOL NOT NULL DEFAULT '0', -- 1: ja começou
	fifty1 BOOL NOT NULL DEFAULT '0', -- 1: ja começou
	fifty2 BOOL NOT NULL DEFAULT '0', -- 1: ja começou
	askC BOOL NOT NULL DEFAULT '0', -- 1: ja começou
	ask1 BOOL NOT NULL DEFAULT '0', -- 1: ja começou
	ask2 BOOL NOT NULL DEFAULT '0', -- 1: ja começou
	convidado1 varchar(32) REFERENCES utilizador,
	convidado2 varchar(32) REFERENCES utilizador,
	convidado3 varchar(32) REFERENCES utilizador,
	convidado4 varchar(32) REFERENCES utilizador
);

--Criação de um Administrador

INSERT INTO utilizador VALUES  ('admin', 'admin', 0);

--Criação de 25 Perguntas base

INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual o ponto mais alto de Portugal continental?', 'Serra da Estrela', 'Gerês', 'Ilha do Pico', 'Pico Ruivo');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Em ano foi fundada a empresa Ferrari S.p.A.?', '1947', '1954', '1937', '1961');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Como se chamam os habitantes de Chaves?', 'Flavienses', 'Chavenses', 'Chavianos', 'Fechaduras');
INSERT INTO perguntas  VALUES  (DEFAULT, 'De quem é o álbum: “Abbey Road”?', 'The Beatles', 'Justin Bieber', 'Jimi Hendrix', 'Rolling Stones');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual monarca foi Rei de Portugal e imperador do Brasil?', 'D. Pedro IV', 'D. João VI', 'D. Afonso Henriques', 'D. Miguel I');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual o cognome do rei D. Pedro IV?', 'O Libertador', 'O Popular', 'O Esperançoso', 'O Patriota');
INSERT INTO perguntas  VALUES  (DEFAULT, 'O que é o PIB?', 'Produto Interno Bruto', 'Pequeno Índice de Burrice', 'Potencial Individual de Base', 'Preços de Importação Base');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Desde que século Lisboa é capital de Portugal?', 'Século XII', 'Século XIV', 'Século X', 'Século XIX');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual a cidade conhecida como: “O berço de Portugal?’', 'Guimarães', 'Porto', 'Braga', 'Lisboa');
INSERT INTO perguntas  VALUES  (DEFAULT, 'A marca de automoveis Audi é originária de que país?', 'Alemanha', 'França', 'Japão', 'Itália');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual o significado da sigla LG?', 'Life is Good', 'Lots of Guns', 'Large Giants', 'Low Gage');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual destes numeros não é primo?', '1', '2', '13', '31');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual país foi liderado por Josef Stalin?', 'União Soviética', 'Alemanha', 'Itália', 'Geórgia');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual a capital da Bulgária?', 'Sófia', 'Istambul', 'Baku', 'Budapeste');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Como se chama a camada de pele que está à superficie?', 'Epiderme', 'Derme', 'Superficiderme', 'Extederme');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Tony Blair foi líder politico de que país?', 'Reino Unido', 'Estados Unidos da América', 'Portugal', 'África do Sul');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Em que país se encontra a cidade de Valença?', 'Portugal', 'Espanha', 'Cabo Verde', 'Brazil');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual dos rios serve de fronteira entre Portugal e Espanha?', 'Mondego', 'Guadiana', 'Tejo', 'Douro');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Em que país se fala a lingua Bantu?', 'Moçambique', 'Angola', 'Cabo-verde', 'Costa Rica');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Qual o piloto de Formula 1 com mais campeonatos do mundo?', 'Michael Schumacher', 'Ayton Senna', 'Sebastian Vettel', 'Juan Fangio');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Quantos paises haviam na União Europeia em 2013?', '28', '26', '16', '31');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Quem escreveu a série de livros “Game of Thrones”?', 'George R. R. Martin', 'J. K. Rowling', 'Fernando Pessoa', 'E. L. James');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Quantos prémios Nobel foram atribuidos a portugueses?', 'Dois', 'Um', 'Zero', 'Cinco');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Em que ano foi assassinado John Kennedy?', '1963', '1954', '1989', '1975');
INSERT INTO perguntas  VALUES  (DEFAULT, 'Quem foi o Diretor do filme “The Godfather” (1972)?', 'Francis Coppola', 'Brian De Palma', 'Michael Mann', 'Frank Darabont');
