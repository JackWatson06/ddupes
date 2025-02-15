#include <sqlite3.h>

#include <iostream>
#include <string>

#include "../extract_output.h"

constexpr uint MD5_DIGEST_LENGTH = 16;

/* -------------------------------------------------------------------------- */
/*                         Connecting to the Database                         */
/* -------------------------------------------------------------------------- */
/**
 * Useful tutorial here: https://zetcode.com/db/sqlitec/
 */

// int main() {
//   sqlite3 *db;
//   sqlite3_stmt *res;

//   int rc = sqlite3_open(":memory:", &db);

//   if (rc != SQLITE_OK) {
//     std::cerr << "Could not open the database. Error message: "
//               << sqlite3_errmsg(db);
//     sqlite3_close(db);

//     return 1;
//   }

//   rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);

//   if (rc != SQLITE_OK) {
//     std::cerr << "Failed to fetch data. Error message: " <<
//     sqlite3_errmsg(db); return 1;
//   }

//   rc = sqlite3_step(res);

//   if (rc == SQLITE_ROW) {
//     std::cout << sqlite3_column_text(res, 0);
//   }

//   sqlite3_finalize(res);
//   sqlite3_close(db);

//   return 0;
// }

/* -------------------------------------------------------------------------- */
/*                               Inserting Data                               */
/* -------------------------------------------------------------------------- */
// int main() {
//   sqlite3 *db;
//   char *err_msg = 0;

//   int rc = sqlite3_open("test.db", &db);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
//     sqlite3_close(db);
//     return 1;
//   }

//   const char *sql =
//       "DROP TABLE IF EXISTS Cars;"
//       "CREATE TABLE Cars(Id INT, Name TXT, Price INT);"
//       "INSERT INTO Cars VALUES(1, 'Audi', 52642);"
//       "INSERT INTO Cars VALUES(2, 'Mercedes', 52342);"
//       "INSERT INTO Cars VALUES(3, 'Skoda', 52642);"
//       "INSERT INTO Cars VALUES(4, 'Volvo', 52652);"
//       "INSERT INTO Cars VALUES(5, 'Bentley', 56642);"
//       "INSERT INTO Cars VALUES(6, 'Citroen', 52742);"
//       "INSERT INTO Cars VALUES(7, 'Hummer', 52122);"
//       "INSERT INTO Cars VALUES(8, 'Volkswagen', 32642);";

//   // The third parameter here is a callback which loops over each result row.
//   I
//   // could probably use that for querying the tables.
//   rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "SQL error: %s\n", err_msg);

//     sqlite3_free(err_msg);
//     sqlite3_close(db);
//     return 1;
//   }

//   sqlite3_close(db);

//   return 0;
// }

/* -------------------------------------------------------------------------- */
/*                                  Querying                                  */
/* -------------------------------------------------------------------------- */
// int callback(void *, int, char **, char **);

// int main(void) {
//   sqlite3 *db;
//   char *err_msg = 0;

//   int rc = sqlite3_open("test.db", &db);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));

//     sqlite3_close(db);

//     return 1;
//   }

//   const char *sql = "SELECT * FROM Cars;";

//   rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "Failed to selected data\n");
//     fprintf(stderr, "SQL error: %s\n", err_msg);

//     sqlite3_free(err_msg);
//     sqlite3_close(db);

//     return 1;
//   }

//   sqlite3_close(db);
//   return 0;
// }

// int callback(void *NotUsed, int argc, char **argv, char **az_col_name) {
//   NotUsed = 0;
//   for (int i = 0; i < argc; i++) {
//     printf("%s = %s\n", az_col_name[i], argv[i] ? argv[i] : "nullptr");
//   }

//   printf("\n");

//   return 0;
// }

/* -------------------------------------------------------------------------- */
/*                            Parameterized Queries                           */
/* -------------------------------------------------------------------------- */
// int main() {
//   sqlite3 *db;
//   char *err_msg = 0;
//   sqlite3_stmt *res;

//   int rc = sqlite3_open("test.db", &db);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
//     sqlite3_close(db);

//     return 1;
//   }

//   const char *sql = "SELECT Id, Name FROM Cars WHERE Id = ?";
//   rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

//   if (rc == SQLITE_OK) {
//     sqlite3_bind_int(res, 1, 3);
//   } else {
//     fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
//   }

//   int step = sqlite3_step(res);

//   if (step == SQLITE_ROW) {
//     printf("%s: ", sqlite3_column_text(res, 0));
//     printf("%s: ", sqlite3_column_text(res, 1));
//   }

//   sqlite3_finalize(res);
//   sqlite3_close(db);

//   return 0;
// }

/* -------------------------------------------------------------------------- */
/*                                Query hash.db                               */
/* -------------------------------------------------------------------------- */

// const std::string CONNECTION_URI = "/home/jack/.cache/fdupes/hash.db";

// Hash makeHash(const void *blob);

// int main() {
//   DirectoryTableRow::Rows directories{};
//   HashTableRow::Rows hashes{};

//   sqlite3 *db;
//   char *err_msg;
//   sqlite3_stmt *res;

//   int rc = sqlite3_open(CONNECTION_URI.c_str(), &db);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
//     sqlite3_close(db);
//     return 1;
//   }

//   /* ---------------------------- Query Directories ----------------------*/
//   const char *directories_sql = "SELECT * FROM Directories;";
//   rc = sqlite3_prepare_v2(db, directories_sql, -1, &res, 0);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "Could not prepare the SQLite statement: %s\n",
//             sqlite3_errmsg(db));
//     sqlite3_close(db);
//     return 1;
//   }

//   int step = 0;
//   // Move this while loop outside of the class. Mapping should happen inside
//   // though.
//   while ((step = sqlite3_step(res)) == SQLITE_ROW) {
//     directories.push_back(DirectoryTableRow{
//         .id = (unsigned int)sqlite3_column_int(res, 0),
//         .name = std::string{(char *)sqlite3_column_text(res, 1)},
//         .parent_id = std::unique_ptr<unsigned int>{new unsigned int(
//             sqlite3_column_int(res, 3))},
//     });
//   }

//   /* ---------------------------- Query Hashes --------------------------*/
//   const char *hashes_sql = "SELECT * FROM Hashes;";
//   rc = sqlite3_prepare_v2(db, hashes_sql, -1, &res, 0);

//   if (rc != SQLITE_OK) {
//     fprintf(stderr, "Could not prepare the SQLite statement: %s\n",
//             sqlite3_errmsg(db));
//     sqlite3_close(db);
//     return 1;
//   }

//   step = 0;
//   while ((step = sqlite3_step(res)) == SQLITE_ROW) {
//     hashes.push_back(
//         HashTableRow{.directory_id = (unsigned int)sqlite3_column_int(res,
//         0),
//                      .name = std::string{(char *)sqlite3_column_text(res,
//                      1)}, .hash = makeHash(sqlite3_column_blob(res, 10))});
//   }

//   sqlite3_finalize(res);
//   sqlite3_close(db);

//   return 0;
// }

// Hash makeHash(const void *blob) {
//   uint8_t *uint_blob = (uint8_t *)blob;
//   Hash hash{};
//   for (uint8_t *iter = uint_blob; iter != uint_blob + MD5_DIGEST_LENGTH + 1;
//        ++iter) {
//     hash.push_back(*iter);
//   }
//   return hash;
// }

/* -------------------------------------------------------------------------- */
/*                           Abstracting to Classes                           */
/* -------------------------------------------------------------------------- */

Hash blobToHash(const void *blob);

class UnableToConnectError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class UnableToBuildStatementError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class UnableToStepError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class SQLiteDatabase {
 public:
  SQLiteDatabase(std::string file) {
    int rc = sqlite3_open(file.c_str(), &db);

    if (rc != SQLITE_OK) {
      throw UnableToConnectError("Unable to conenct to the database.");
    }
  };

  ~SQLiteDatabase() { sqlite3_close(db); };

  sqlite3 *getDb() const { return db; }

 private:
  sqlite3 *db;
};

template <class T>
class SQLiteTableView : TableView<T> {
 public:
  SQLiteTableView(const SQLiteDatabase &db) : db(db) {}
  ~SQLiteTableView() { sqlite3_finalize(res); }

  virtual T buildTableRow() = 0;

  void prepare(std::string query) {
    int codde = sqlite3_prepare_v2(db.getDb(), query.c_str(), -1, &res, 0);

    if (codde != SQLITE_OK) {
      throw UnableToBuildStatementError("Error trying to build the statement.");
    }
  }
  bool step() {
    int rc = sqlite3_step(res);

    if (rc == SQLITE_ERROR || rc == SQLITE_MISUSE) {
      throw UnableToStepError(
          "You may have forgotten to prepare your statement before stepping!");
    }

    if (sqlite3_step(res) == SQLITE_DONE) {
      return false;
    }

    last_row_fetched = buildTableRow();

    return true;
  }

 protected:
  const SQLiteDatabase db;
  sqlite3_stmt *res;
};

class DirectoryTableView : public SQLiteTableView<DirectoryTableRow> {
 public:
  using SQLiteTableView<DirectoryTableRow>::SQLiteTableView;

  DirectoryTableRow buildTableRow() {
    return DirectoryTableRow{.id = sqlite3_column_int(res, 0),
                             .name = sqlite3_column_text(res, 1),
                             .parent_id = sqlite3_column_int(res, 3)};
  }
};

class HashTableView : public SQLiteTableView<HashTableRow> {
 public:
  using SQLiteTableView<HashTableRow>::SQLiteTableView;

  HashTableRow buildTableRow() {
    return {.directory_id = sqlite3_column_int(res, 0),
            .name = sqlite3_column_text(res, 1),
            .hash = sqlite3_column_blob(res, 10)};
  }
};

const std::string CONNECTION_URI = "/home/jack/.cache/fdupes/hash.db";

int main() {
  DirectoryTableRow::Rows directories{};
  HashTableRow::Rows hashes{};

  SQLiteDatabase sqlite{CONNECTION_URI};
  DirectoryTableView directory_view{sqlite, "SELECT * FROM Directories;"};
  HashTableView hash_view{sqlite, "SELECT * FROM Hashes;"};

  /* ---------------------------- Query Directories ----------------------*/
  while (directory_view.step()) {
    directories.push_back(directory_view.getLastRowFetched());
  }

  /* ---------------------------- Query Hashes --------------------------*/
  while (hash_view.step()) {
    hashes.push_back(hash_view.getLastRowFetched());
  }

  return 0;
}

Hash blobToHash(const void *blob) {
  uint8_t *uint_blob = (uint8_t *)blob;
  Hash hash{};
  for (uint8_t *iter = uint_blob; iter != uint_blob + MD5_DIGEST_LENGTH + 1;
       ++iter) {
    hash.push_back(*iter);
  }
  return hash;
}