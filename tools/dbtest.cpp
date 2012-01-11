#include <tcutil.h>
#include <tchdb.h>
/*#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
*/
int main(int argc, char **argv){

TCHDB *hdb;
char *key, *value; 

/* create the object */
hdb = tchdbnew();

/* open the database */
if(!tchdbopen(hdb, "teloon.hdb", HDBOWRITER|HDBOTRUNC)){
	fprintf(stderr, "open error\n");
}

/* store records */
if(!tchdbput2(hdb, "foo", "hop") ||
!tchdbput2(hdb, "barz", "step") ||
!tchdbput2(hdb, "ssss", "jump")){
fprintf(stderr, "put error\n");
}

/* retrieve records */
value = tchdbget2(hdb, "foo");
if(value){
printf("%s\n", value);
free(value);
} else {
fprintf(stderr, "get error\n");
}

/* traverse records */
tchdbiterinit(hdb);
while((key = tchdbiternext2(hdb)) != NULL){
value = tchdbget2(hdb, key);
if(value){
printf("%s:%s\n", key, value);
free(value);
}
free(key);
}
tchdbiterinit(hdb);
while((key = tchdbiternext2(hdb)) != NULL){
value = tchdbget2(hdb, key);
if(value){
printf("%s:%s\n", key, value);
free(value);
}
free(key);
}

/* close the database */
if(!tchdbclose(hdb)){
fprintf(stderr, "close error\n");
}

/* delete the object */
tchdbdel(hdb);

return 0;
} 
