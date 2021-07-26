#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <stdint.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void do_exit(PGconn *conn){
  PQfinish(conn);
  exit(1);
}

int main(){
  PGconn *conn = PQconnectdb("user=postgres dbname = tree_articles");
  if(PQstatus(conn) == CONNECTION_BAD){
      fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
        do_exit(conn);
  }

  printf("Here are all existing articles in Cvetarka Shop.\n\n");
  
   PGresult *res = PQexec(conn, "Select \"ID\", \"Name\" from public.\"Articles\" order by \"ID\";");

   if(PQresultStatus(res)!=PGRES_TUPLES_OK){
     printf("No data retrieved\n");
     PQclear(res);
     do_exit(conn);
   }

   int rows = PQntuples(res);

   // printf("%d", atoi(PQgetvalue(res, 0, 0)));
   
   printf("Article ID      Article Name\n");
   for(int i=0; i<rows; i++){
     printf("%d", atoi(PQgetvalue(res, i, 0)));
     printf("%13s %s\n","", PQgetvalue(res, i, 1));
   }

   do_exit(conn);
   return 0;
}
