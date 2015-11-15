/******************************************
 *    SQLite3 minimalistic C++ wrapper    *
 *           written by Kennny            *
 *     https://github.com/MartinUbl       *
 ******************************************/

#ifndef _SQLITE3_WRAPPER_H_
#define _SQLITE3_WRAPPER_H_

#include <vector>

/* class wrapping result row */
class SQLiteResultRow : public std::vector<std::string>
{
    public:
        const char* GetString(int column);
        int GetInt(int column);
        float GetFloat(int column);
};

/* class wrapping query result */
class SQLiteQueryResult
{
    public:
        SQLiteQueryResult();
        ~SQLiteQueryResult();

        /* Stores retrieved row - called from execute callback only, do not use elsewhere if not really necessary */
        void StoreRow(int count, char** values);
        /* Sets column names */
        void SetColumnNames(int count, char** names);

        /* Returns true if column names was stored */
        bool IsColumnNamesSet();

        /* Finalizes working with resultset and clears it from memory */
        SQLiteQueryResult* Finalize();
        /* Destroys this object, this is the final call on this object */
        void Destroy();

        /* Retrieves row count of result set */
        size_t GetRowCount();
        /* Fetches next row available and moves cursor to next row */
        SQLiteResultRow* Fetch();
        /* Resets read cursor to the very beginning */
        void Rewind();

    protected:
        /* Stored column names */
        std::vector<std::string> m_columnNames;
        /* Stored rows of result */
        std::vector<SQLiteResultRow> m_rows;
        /* Result row cursor */
        size_t m_cursor;
};

/* Wrapper class for SQLite database connection */
class SQLiteDB
{
    public:
        ~SQLiteDB();

        /* Opens database file if available, or creates new one */
        static SQLiteDB* OpenDB(const char* filename, bool utf16 = false);

        /* Performs query on opened database */
        SQLiteQueryResult* Query(const char* qr, ...);

    private:
        /* Hidden constructor, use factory method for creating instances */
        SQLiteDB();

        /* Stored connection instance */
        sqlite3* m_DB;
};

#endif
