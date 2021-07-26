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

vector<string> get_path(int category_id, PGconn *conn){
  vector<string> categories;
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

void print_all_articles_from_category(PGconn *conn, int category_ID){
  category_ID = htonl((int)category_ID);
  const char* paramValues[1];
  int paramLengths[1];
  int paramFormats[1];
  
  paramValues[0] = (char *) &category_ID;
  paramLengths[0] = sizeof(category_ID);
  paramFormats[0] = 1;

  PGresult *res = PQexecParams(conn, "select \"ID\", \"Name\" from public.\"Articles\" as t1, public.\"Dependencies\" as t2 where t1.\"ID\" = t2.\"Article_ID\" and t2.\"Category_ID\" = $1::int4;", 1, NULL, paramValues, paramLengths, paramFormats, 0);
  if(PQresultStatus(res)!=PGRES_TUPLES_OK){
    printf("No data retrieved\n");
    PQclear(res);
    do_exit(conn);
  }

  int rows = PQntuples(res);

  cout<< "\n\nThis category has the following articles: "<<endl;
  printf("Article ID      Article Name\n");
  for(int i=0; i<rows; i++){
    printf("%d", atoi(PQgetvalue(res, i, 0)));
    printf("%13s %s\n","", PQgetvalue(res, i, 1));
  }

}

int main(){
  PGconn *conn = PQconnectdb("user=postgres dbname = tree_articles");
  if(PQstatus(conn) == CONNECTION_BAD){
    fprintf(stderr, "Connection to database failed: %s\n",
	    PQerrorMessage(conn));
  }
  //  cout<<"test2"
  print_categories(conn, 1, 0, 0);

  cout<<"Please enter the ID of a category to see all articles in it."<<endl;
  int ID;
  cin>>ID;

  vector<string> category_path = get_path(ID, conn);
  cout<< "Path of the category:"<<endl;
  print_path(category_path);
  print_all_articles_from_category(conn, ID);
  

  do_exit(conn);
  
  return 0;
}
