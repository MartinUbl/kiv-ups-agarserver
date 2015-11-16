#include "General.h"
#include "Storage.h"
#include "Log.h"

#include <map>

Storage::Storage()
{
    m_mainDB = nullptr;
}

Storage::~Storage()
{
    //
}

bool Storage::Init()
{
    sLog->Info("Loading storage...");
    m_mainDB = SQLiteDB::OpenDB("main.db");

    // check main database for structure changes
    CheckDBStructure();

    sLog->Info("Storage loaded successfully!\n");

    return true;
}

void Storage::CreateTable(int index)
{
    int i;
    DatabaseStructure::DBKnownTable &tblrec = DatabaseStructure::KnownTables[index];

    // prepare query base
    std::string query = "CREATE TABLE " + std::string(tblrec.name) + " (";

    // serialize all columns with their types to this query
    for (i = 0; tblrec.structure[i].name != nullptr; i++)
    {
        if (i != 0)
            query += ", ";

        query += std::string(tblrec.structure[i].name) + " " + std::string(tblrec.structure[i].type);
    }

    query += ")";

    // execute!
    m_mainDB->Query(query.c_str());
}

void Storage::CheckTableColumns(int index, SQLiteQueryResult* structure)
{
    int i;
    std::string tmp, cols;
    std::map<std::string, std::string> colSetToDel, colSetToAdd;
    DatabaseStructure::DBKnownTable &tblrec = DatabaseStructure::KnownTables[index];
    SQLiteResultRow* rrow;

    // add all expected columns with types to colSetToAdd map
    for (i = 0; tblrec.structure[i].name != nullptr; i++)
        colSetToAdd[tblrec.structure[i].name] = tblrec.structure[i].type;

    // go through all actual columns
    while (rrow = structure->Fetch())
    {
        tmp = rrow->GetString(1);
        // if it does not exist in expected columns set, delete it;
        // otherwise erase it from "columns to be added" set, since it's already present
        if (colSetToAdd.find(tmp) == colSetToAdd.end())
            colSetToDel[tmp] = rrow->GetString(2);
        else
            colSetToAdd.erase(tmp);
    }

    // at first, check for columns to be added and add them
    if (!colSetToAdd.empty())
    {
        // we need to go through all columns to be added and add them one-by-one
        for (std::map<std::string, std::string>::iterator itr = colSetToAdd.begin(); itr != colSetToAdd.end(); itr++)
        {
            sLog->Info("Adding column %s.%s", tblrec.name, (*itr).first.c_str());

            tmp = "ALTER TABLE " + std::string(tblrec.name) + " ADD COLUMN " + (*itr).first + " " + (*itr).second;

            m_mainDB->Query(tmp.c_str());
        }
    }

    // column deletion is pretty tricky, since SQLite does not support "drop column" command
    // we need to rename table, create whole new again based on valid structure, and then copy all records trimmed
    // by columns, we no longer need, to newly created table
    if (!colSetToDel.empty())
    {
        for (std::map<std::string, std::string>::iterator itr = colSetToDel.begin(); itr != colSetToDel.end(); itr++)
            sLog->Info("Removing column %s.%s", tblrec.name, (*itr).first.c_str());

        // rename table with old structure (just add __old suffix)
        tmp = "ALTER TABLE " + std::string(tblrec.name) + " RENAME TO " + std::string(tblrec.name) + "__old";
        m_mainDB->Query(tmp.c_str());

        // recreate original table
        CreateTable(index);

        // serialize valid, expected column names
        cols = "";
        for (i = 0; tblrec.structure[i].name != nullptr; i++)
        {
            if (i != 0)
                cols += ", ";
            cols += tblrec.structure[i].name;
        }

        // copy trimmed records from old table to new
        tmp = "INSERT INTO " + std::string(tblrec.name) + " (" + cols + ") SELECT " + cols + " FROM " + std::string(tblrec.name) + "__old";
        m_mainDB->Query(tmp.c_str());

        // and finally drop the old table
        tmp = "DROP TABLE " + std::string(tblrec.name) + "__old";
        m_mainDB->Query(tmp.c_str());
    }
}

void Storage::CheckDBStructure()
{
    SQLiteQueryResult* res;
    int i;

    // go through all known tables in DB
    for (i = 0; i < sizeof(DatabaseStructure::KnownTables) / sizeof(DatabaseStructure::DBKnownTable); i++)
    {
        // retrieve structure
        res = GetTableStructure(DatabaseStructure::KnownTables[i].name);
        // if it does not exist, create it; otherwise check for structure consistency
        if (!res || res->GetRowCount() == 0)
        {
            sLog->Info("Creating table %s", DatabaseStructure::KnownTables[i].name);
            CreateTable(i);
        }
        else
            CheckTableColumns(i, res);

        res->Finalize()->Destroy();
    }
}

SQLiteQueryResult* Storage::GetTableStructure(const char* table)
{
    return m_mainDB->Query("PRAGMA table_info(%s)", table);
}

StorageResult::UserRecord* StorageResult::UserRecord::Build(SQLiteResultRow* rrow)
{
    StorageResult::UserRecord* nr = new StorageResult::UserRecord;

    nr->id = rrow->GetInt(0);
    nr->username = rrow->GetString(1);
    nr->passwordHash = rrow->GetString(2);

    return nr;
}

StorageResult::UserRecord* Storage::GetUserById(int32_t id)
{
    SQLiteQueryResult* res = m_mainDB->Query("SELECT id, username, password FROM users WHERE id = %i", id);
    if (!res)
        return nullptr;

    if (res->GetRowCount() == 0)
    {
        res->Finalize()->Destroy();
        return nullptr;
    }

    SQLiteResultRow* rrow = res->Fetch();

    // we assume there is only one user with this ID (guaranteed, because id is primary key)
    if (rrow)
        return StorageResult::UserRecord::Build(rrow);

    return nullptr;
}

StorageResult::UserRecord* Storage::GetUserByUsername(const char* username)
{
    SQLiteQueryResult* res = m_mainDB->Query("SELECT id, username, password FROM users WHERE username = '%s'", username);
    if (!res)
        return nullptr;

    if (res->GetRowCount() == 0)
    {
        res->Finalize()->Destroy();
        return nullptr;
    }

    SQLiteResultRow* rrow = res->Fetch();

    // we assume there is only one user with this ID (guaranteed, because id is primary key)
    if (rrow)
        return StorageResult::UserRecord::Build(rrow);

    return nullptr;
}

void Storage::StoreUser(const char* username, const char* passhash)
{
    if (!m_mainDB->Execute("INSERT INTO users (username, password) VALUES ('%s', '%s')", username, passhash))
        sLog->Error("STORAGE: Could not insert user with name '%s' to database", username);
}
