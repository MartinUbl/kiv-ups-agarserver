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

/* Storage class used for accessing data in SQLite and runtime storage */
class Storage
{
    friend class Singleton<Storage>;
    public:
        ~Storage();

        /* Initializes database, check for valid structure and makes sure storage layer is ready */
        bool Init();

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
