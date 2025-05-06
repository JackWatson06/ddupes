#pragma once

#include <sqlite3.h>

#include <stdexcept>
#include <vector>

#include "../lib.h"

/* -------------------------------------------------------------------------- */
/*                                  Database                                  */
/* -------------------------------------------------------------------------- */
sqlite3 *initDB(char const *const file_name);
void resetDB(sqlite3 *db);
void freeDB(sqlite3 *db_handle);

/* -------------------------------------------------------------------------- */
/*                               Table Gateways                               */
/* -------------------------------------------------------------------------- */
struct directory_table_row {
  typedef std::vector<directory_table_row> rows;

  int id;
  str_const name;
  int parent_id;

  bool operator==(const directory_table_row &rhs) const;
};

typedef directory_table_row const directory_table_row_const;

struct hash_table_row {
  typedef std::vector<hash_table_row> rows;
  int id;
  int const directory_id;
  str_const name;
  hash_const hash;

  bool operator==(hash_table_row const &rhs) const;
};

typedef hash_table_row const hash_table_row_const;

struct scan_meta_data_table_row {
  str_const root_dir;

  bool operator==(scan_meta_data_table_row const &rhs) const;
};

struct directory_input {
  int const parent_id;
  str_const name;

  bool operator==(directory_input const &rhs) const;
};

struct hash_input {
  int const directory_id;
  str_const name;
  hash_const hash;

  bool operator==(hash_input const &rhs) const;
};

struct scan_meta_data_input {
  str_const root_dir;

  bool operator==(scan_meta_data_input const &rhs) const;
};

directory_table_row::rows fetchAllDirectories(sqlite3 *db);
int createDirectory(sqlite3 *db, directory_input const &directory_table_input);

hash_table_row::rows fetchAllHashes(sqlite3 *db);
int createHash(sqlite3 *db, hash_input const &hash_table_input);
void deleteHash(sqlite3 *db, int id);

scan_meta_data_table_row fetchScanMetaData(sqlite3 *db);
void createScanMetaData(sqlite3 *db,
                        scan_meta_data_input const &scan_meta_data_input);

/* -------------------------------------------------------------------------- */
/*                                   Errors                                   */
/* -------------------------------------------------------------------------- */
class unable_to_connect_error : public std::runtime_error {
public:
  unable_to_connect_error(const char *message) : std::runtime_error(message) {}
};

class unable_to_create_table_error : public std::runtime_error {
public:
  unable_to_create_table_error(const char *message)
      : std::runtime_error(message) {}
};

class unable_to_build_statement_error : public std::runtime_error {
public:
  unable_to_build_statement_error(const char *message)
      : std::runtime_error(message) {}
};

class unable_to_step_error : public std::runtime_error {
public:
  unable_to_step_error(const char *message) : std::runtime_error(message) {}
};

class unable_to_insert_error : public std::runtime_error {
public:
  unable_to_insert_error(const char *message) : std::runtime_error(message) {}
};

class unable_to_delete_error : public std::runtime_error {
public:
  unable_to_delete_error(const char *message) : std::runtime_error(message) {}
};

class not_found_error : public std::runtime_error {
public:
  not_found_error(const char *message) : std::runtime_error(message) {}
};
