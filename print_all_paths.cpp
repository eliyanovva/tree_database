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

vector<string> get_path(int article_id, string article_name, int category_id,  PGconn *conn){
  vector<string> categories;
  categories.push_back(article_name);
  int id_seed = category_id;

  while(id_seed>0){
    // query to access row and get parent_ID
    
    id_seed = htonl((int)id_seed);
    const char* paramValues[1];
    int paramLengths[1];
    int paramFormats[1];

    paramValues[0] = (char *) &id_seed;
    paramLengths[0] = sizeof(id_seed);
    paramFormats[0] = 1;

    PGresult *res = PQexecParams(conn, "SELECT * FROM public.\"categories\" WHERE \"ID\" = $1::int4", 1, NULL, paramValues, paramLengths, paramFormats, 0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){
      printf("No data retrieved\n");
      PQclear(res);
      do_exit(conn);
    }

    categories.push_back(PQgetvalue(res, 0, 2));
    id_seed = atoi(PQgetvalue(res, 0, 1));
    
  }
  return categories;
}

void print_path(vector<string> path){
  while(path.size()>1){
    cout<<path[path.size()-1]<<" -----> ";
    path.pop_back();
  }

  cout<<path[0];
  path.pop_back();
}
int main(){
  PGconn *conn = PQconnectdb("user=postgres dbname = tree_articles");
  if(PQstatus(conn) == CONNECTION_BAD){
      fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
        do_exit(conn);
  }

  // input article ID
  printf("Please input the ID of the Article whose path you want to see:\n");
  int input_ID;
  cin>>input_ID;

  input_ID = htonl((int)input_ID);

  const char* paramValues[1];
  int paramLengths[1];
  int paramFormats[1];

  paramValues[0] = (char *) &input_ID;
  paramLengths[0] = sizeof(input_ID);
  paramFormats[0] = 1;

  PGresult *res = PQexecParams(conn, "SELECT * FROM public.\"Dependencies\" WHERE \"Article_ID\" = $1::int4", 1, NULL, paramValues, paramLengths, paramFormats, 0);
  
  if(PQresultStatus(res)!=PGRES_TUPLES_OK){
    printf("No data retrieved from the Dependencies table\n");
    PQclear(res);
    do_exit(conn);
  }

  PGresult *get_article_name = PQexecParams(conn, "SELECT \"Name\" from public.\"Articles\" WHERE \"ID\" = $1::int4", 1, NULL, paramValues, paramLengths, paramFormats, 0);
  
  if(PQresultStatus(res)!=PGRES_TUPLES_OK){
    printf("No article name was retrieved from the Articles table\n");
    PQclear(res);
    do_exit(conn);
  }

  string article_name = PQgetvalue(get_article_name, 0, 0);
  int rows = PQntuples(res);
  cout<<"The article that you are looking for is:  "<<article_name<<endl<<endl;

  
  for(int i=0; i<rows; i++){
    cout<< "\nPath "<< i+1<<":  ";
    vector<string> path = get_path(atoi(PQgetvalue(res, i, 0)), article_name, atoi(PQgetvalue(res, i, 1)), conn);
    print_path(path);
  }
  
  do_exit(conn);
  return 0;
}
