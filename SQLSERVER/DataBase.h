#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;


class DataBase {
    sqlite3* db;
    string path = "C:/Users/mikha/Desktop/";
    string db_name = "test.db";
    char* err_msg = 0;
public:
    DataBase(string path, string  name) :path(path), db_name(name) {
        int rc = sqlite3_open((path + db_name).c_str(), &db);
        // если подключение прошло неудачно
        if (rc != SQLITE_OK)
        {
            sqlite3_close(db);
            cerr << "Init DataBase Error\n";
        }
    }

    int sql_create_table(string sql) {
        int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg);
        if (rc != SQLITE_OK)
        {
            printf("SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);      // очищаем ресурсы
            sqlite3_close(db);
            return 1;
        }
        return 0;
    }

    vector<string> find(string sql) {
        vector<string> results;
        char* errMsg = nullptr;

        auto callback = [](void* data, int argc, char** argv, char** colNames) {
            auto* results = static_cast<vector<string>*>(data);

            string row;
            for (int i = 0; i < argc; i++) {
                if (i > 0) row += "|";  // разделитель полей
                row += (argv[i] ? argv[i] : "NULL");
            }
            results->push_back(row);

            return 0;
            };

        if (sqlite3_exec(db, sql.c_str(), callback, &results, &errMsg) != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
            return {};
        }

        return results;
    }

    bool update(string sql)
    {
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Update failed: " << errMsg << endl;
            sqlite3_free(errMsg);
            return false;
        }

        // ѕровер€ем количество измененных строк
        int changes = sqlite3_changes(db);
        if (changes == 0) {
            cerr << "No records found" << endl;
            return false;
        }

        return true;
    }

    ~DataBase() {
        sqlite3_close(db);
    }
};

#endif
