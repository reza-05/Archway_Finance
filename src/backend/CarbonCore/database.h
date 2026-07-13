#ifndef DATABASE_H
#define DATABASE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_NAME_LEN 100
#define MAX_TYPE_LEN 50
#define MAX_DATE_LEN 20
#define MAX_DESC_LEN 256

    // account structure
    typedef struct
    {
        int id;
        char name[MAX_NAME_LEN];
        char type[MAX_TYPE_LEN];
        double balance;
    } Account;
    
#ifdef __cplusplus
}
#endif

#endif // DATABASE_H
