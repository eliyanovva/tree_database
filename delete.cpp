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
#include "./functions.h"

using namespace std;

void delete_article(PGconn *conn, int article_ID){
  const char* paramValues[1];
  int paramLengths[1];
  int paramFormats[1];

  article_ID = htonl(article_ID);

  paramValues[0] = (char *) &article_ID;
  paramLengths[0] = sizeof(article_ID);
  paramFormats[0] = 1;

  PGresult *res = PQexecParams(conn, "Delete from public.\"Articles\" where \"ID\" = $1::integer;", 1, NULL, paramValues, paramLengths, paramFormats, 0);

  if(PQresultStatus(res)!=PGRES_COMMAND_OK){
    printf("Deleting Items failed. \n");
    PQclear(res);
    do_exit(conn);
    return;
  }

  PQclear(res);
}

void delete_articles_without_category(PGconn *conn){

  PGresult *res = PQexec(conn, "delete from public.\"Articles\" where \"ID\" in (select \"ID\" from public.\"Articles\" where \"ID\" not in (select \"Article_ID\" from public.\"Dependencies\"));");

  if(PQresultStatus(res)!=PGRES_COMMAND_OK){
    printf("Deleting Items failed. \n");
    PQclear(res);
    do_exit(conn);
    return;
  }

  PQclear(res);

}

void delete_category(PGconn *conn, int category_ID){
  const char* paramValues[1];
  int paramLengths[1];
  int paramFormats[1];

  category_ID = htonl(category_ID);

  paramValues[0] = (char *) &category_ID;
  paramLengths[0] = sizeof(category_ID);
  paramFormats[0] = 1;

  PGresult *res = PQexecParams(conn, "Delete from public.\"categories\" where \"ID\" = $1::integer;", 1, NULL, paramValues, paramLengths, paramFormats, 0);

  if(PQresultStatus(res)!=PGRES_COMMAND_OK){
    printf("Deleting Items failed. \n");
    PQclear(res);
    do_exit(conn);
    return;
  }
  PQclear(res);
  delete_articles_without_category(conn);
}



int main(){
  PGconn *conn = PQconnectdb("user=postgres dbname = tree_articles");
  if(PQstatus(conn) == CONNECTION_BAD){
    fprintf(stderr, "Connection to database failed: %s\n",
	    PQerrorMessage(conn));
  }
  char input = 'a';
  while(true){
    cout<<"\n\n Press \"A\" if you want to delete an article, \"C\" if you want to delete a category, and \"Q\" if you want to quit."<<endl<<endl;

    cin>>input;

    if (input == 'Q')
      break;
    else if(input== 'A'){
      print_articles(conn);
      cout<<"Please choose the article you would like to delete."<<endl;
      int ID;
      cin>>ID;
      delete_article(conn, ID);
    }

    else if(input == 'C'){
      print_categories(conn, 1, 0, 0);
      cout<<"Please choose the category you would like to delete"<<endl;
      int ID;
      cin>>ID;
      delete_category(conn, ID);
    }

    else
      continue;
      }
    return 0;
}
