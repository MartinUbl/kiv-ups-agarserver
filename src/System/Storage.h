#ifndef AGAR_STORAGE_H
#define AGAR_STORAGE_H

#include "Singleton.h"
#include "sqlite3.h"
#include "sqlite3_wrapper.h"

/* Namespace for database structures, known tables, and everything needed for database structure consistency */
namespace DatabaseStructure
{
    /* database column definition structure */
    struct DBColumn
    {
        const char* name;
        const char* type;
    };

    /* users table */
    static DBColumn Users[] = {
        { "id", "INTEGER PRIMARY KEY" },
        { "username", "TEXT" },
        { "password", "TEXT" },
        { nullptr, nullptr } // ending record
    };

    /* known table description structure */
    struct DBKnownTable
    {
        const char* name;
        DBColumn* structure;
    };

    /* array of all known tables */
    static DBKnownTable KnownTables[] = {
        { "users", Users }
    };
};

/* namespace for fetched Storage getters result sets */
namespace StorageResult
{
    /* result of Users table select */
    struct UserRecord
    {
        /* assigned id */
        int32_t id;
        /* user name */
        const char* username;
        /* SHA1 password hash stored */
        const char* passwordHash;

        /* Static factory method for building record from fetched row */
        static UserRecord* Build(SQLiteResultRow* rrow);
    };
};

/* Storage class used for accessing data in SQLite and runtime storage */
class Storage
{
    friend class Singleton<Storage>;
    public:
        ~Storage();

        /* Initializes database, check for valid structure and makes sure storage layer is ready */
        bool Init();

        StorageResult::UserRecord* GetUserById(int32_t id);
        StorageResult::UserRecord* GetUserByUsername(const char* username);
        void StoreUser(const char* username, const char* passhash);

    protected:
        /* Hidden constructor (singleton) */
        Storage();

        /* Checks database for structure changes, creates missing tables, modifies existing if necessary */
        void CheckDBStructure();
        /* Checks column presence in existing table */
        void CheckTableColumns(int index, SQLiteQueryResult* structure);
        /* Creates a new table based on stored known structure */
        void CreateTable(int index);
        /* Retrieves SQLite table structure in standard SQLiteQueryResult */
        SQLiteQueryResult* GetTableStructure(const char* table);

    private:
        /* Main database */
        SQLiteDB* m_mainDB;
};

#define sStorage Singleton<Storage>::getInstance()

#endif
