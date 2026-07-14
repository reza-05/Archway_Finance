#include "stdio.h"
#include "database.h"
#include "string.h"

void init_database(Database *db) {     // intialized count to 0
    if (db != NULL) {
        db->count = 0;
    }
}


//injection of values into database
int add_transaction(Database *db, double amount, int is_income, const char *category, const char *date, const char *description) {
    Transaction *t = &db->list[db->count];

    t->id = db->count+1;

    t->amount= amount;

    t->is_income = is_income;

    strcpy(t->category, category);
    t->category[49]= '\0';

    strcpy(t->date, date);
    t->date[19]= '\0';

    strcpy(t->description,description);
    t->description[49] = '\0';

    db->count++;

}

int save_database(const Database *db, const char *filename){


    FILE *file = fopen(filename, "wb");

    fwrite(&(db->count), sizeof(int), 1, file);  // transaction count first

    if(db->count > 0)
        fwrite(db->list,sizeof(Transaction),db->count,file);   // then copy all transactions
}