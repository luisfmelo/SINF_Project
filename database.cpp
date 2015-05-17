#include <postgresql/libpq-fe.h>
#include "database.h"

PGconn* conn = NULL;
 
void initDB()
{
	conn = PQconnectdb("host='vdbm.fe.up.pt' user='sinf15g15' password='david&luis' dbname='sinf15g15'");
	
	if (!conn)
	{
		cout << "Não foi possivel ligar a base de dados Error: " << PQstatus(conn) << endl;
		exit(-1);
	}
	
	if (PQstatus(conn) != CONNECTION_OK)
	{
		cout << "Não foi possivel ligar a base de dados. Error: " << PQstatus(conn) << endl;
		exit(-1);
	}
}
 
PGresult* executeSQL(string sql)
{
  PGresult* res = PQexec(conn, sql.c_str());
 
	if(!(PQresultStatus(res) == PGRES_COMMAND_OK || PQresultStatus(res) == PGRES_TUPLES_OK))
	{
		Scream("ERRO CRITICO: O Servidor vai ser deligado!\nPor mais informaçoes, contacte um dos administradores!\n");
		exit(-1);
	}
 
	return res;
}
 
void closeDB()
{
	PQfinish(conn);
}
