#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <stdint.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

void do_exit(PGconn *conn){
  PQfinish(conn);
  exit(1);
}


void print_categories(PGconn *conn, int ID, int parent_ID, int space_mult){
  //cout<<"test 1"<<endl;
  ID = htonl(ID);
  parent_ID = htonl(parent_ID);

  const char* paramValues[1];
  int paramLengths[1];
  int paramFormats[1];

  paramValues[0] = (char *) &ID;
  paramLengths[0] = sizeof(ID);
  paramFormats[0] = 1;
  
  PGresult *category_name = PQexecParams(conn, "SELECT * FROM public.\"categories\" WHERE \"ID\" = $1::int4", 1, NULL, paramValues, paramLengths, paramFormats, 0);
  
  if(PQresultStatus(category_name)!=PGRES_TUPLES_OK){
    printf("No data retrieved from the Dependencies table\n");
    PQclear(category_name);
    do_exit(conn);
  }
  int test_rows = PQntuples(category_name);
  
  cout<<string(5*space_mult, '-')<<"> "<<PQgetvalue(category_name, 0, 0)<<"   "<<PQgetvalue(category_name, 0, 2)<<endl;

  PGresult *res = PQexecParams(conn, "SELECT * FROM public.\"categories\" WHERE \"parent_ID\" = $1::int4", 1, NULL, paramValues, paramLengths, paramFormats, 0);
  
  if(PQresultStatus(res)!=PGRES_TUPLES_OK){
    printf("No data retrieved from the Dependencies table\n");
    PQclear(res);
    do_exit(conn);
  }

  int rows = PQntuples(res);
  
  space_mult++;
  for(int i=0; i<rows; i++){
    print_categories(conn, atoi(PQgetvalue(res, i, 0)), atoi(PQgetvalue(res, i, 1)), space_mult);
  }
}


void print_articles(PGconn *conn){
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
}

