#include "sqlite3.h"
#include "sqlite3_wrapper.h"

#include <iostream>
#include <cstdarg>
#include <string>

SQLiteQueryResult::SQLiteQueryResult() : m_cursor(0)
{
    //
}

SQLiteQueryResult::~SQLiteQueryResult()
{
    //
}

const char* SQLiteResultRow::GetString(int column)
{
    return (*this)[column].c_str();
}

int SQLiteResultRow::GetInt(int column)
{
    return std::stoi((*this)[column].c_str());
}

float SQLiteResultRow::GetFloat(int column)
{
    return std::stof((*this)[column].c_str());
}

SQLiteQueryResult* SQLiteQueryResult::Finalize()
{
    m_rows.clear();
    m_columnNames.clear();

    return this;
}

void SQLiteQueryResult::Destroy()
{
    delete this;
}

void SQLiteQueryResult::StoreRow(int count, char** values)
{
    int ind = m_rows.size();
    m_rows.resize(ind + 1);

    for (int i = 0; i < count; i++)
        m_rows[ind].push_back(values[i] ? values[i] : "");
}

void SQLiteQueryResult::SetColumnNames(int count, char** names)
{
    m_columnNames.resize(count);

    for (int i = 0; i < count; i++)
        m_columnNames[i] = names[i];
}

bool SQLiteQueryResult::IsColumnNamesSet()
{
    return !m_columnNames.empty();
}

size_t SQLiteQueryResult::GetRowCount()
{
    return m_rows.size();
}

SQLiteResultRow* SQLiteQueryResult::Fetch()
{
    if (m_cursor >= m_rows.size())
        return nullptr;

    return &(m_rows[m_cursor++]);
}

void SQLiteQueryResult::Rewind()
{
    m_cursor = 0;
}

SQLiteDB::SQLiteDB()
{
    m_DB = nullptr;
}

SQLiteDB::~SQLiteDB()
{
    if (m_DB)
        sqlite3_close(m_DB);
}

SQLiteDB* SQLiteDB::OpenDB(const char* filename, bool utf16)
{
    int result;
    SQLiteDB* sqldb = new SQLiteDB();

    if (utf16)
        result = sqlite3_open16(filename, &sqldb->m_DB);
    else
        result = sqlite3_open(filename, &sqldb->m_DB);

    if (result != SQLITE_OK)
    {
        if (sqldb->m_DB)
            sqlite3_close(sqldb->m_DB);
        delete sqldb;
        return nullptr;
    }

    return sqldb;
}

int SQLiteDBWrapper_QueryCallback(void* resptr, int argc, char** argv, char** col)
{
    SQLiteQueryResult* res = ((SQLiteQueryResult*)resptr);

    if (!res->IsColumnNamesSet())
        res->SetColumnNames(argc, col);

    res->StoreRow(argc, argv);

    return 0;
}

SQLiteQueryResult* SQLiteDB::Query(const char* qr, ...)
{
    SQLiteQueryResult *res = new SQLiteQueryResult();

    va_list argList;
    va_start(argList, qr);
    char buf[2048];
    vsnprintf(buf, 2048, qr, argList);
    va_end(argList);

    int result = sqlite3_exec(m_DB, buf, SQLiteDBWrapper_QueryCallback, res, nullptr);

    if (result != SQLITE_OK)
    {
        res->Finalize();
        delete res;
        return nullptr;
    }

    return res;
}

bool SQLiteDB::Execute(const char* qr, ...)
{
    va_list argList;
    va_start(argList, qr);
    char buf[2048];
    vsnprintf(buf, 2048, qr, argList);
    va_end(argList);

    int result = sqlite3_exec(m_DB, buf, nullptr, nullptr, nullptr);

    return (result == SQLITE_OK);
}
