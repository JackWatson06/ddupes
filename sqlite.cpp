#include "sqlite.h"

SQLiteDatabase::SQLiteDatabase(std::string file) {
  int rc = sqlite3_open(file.c_str(), &db);

  if (rc != SQLITE_OK) {
    throw UnableToConnectError("Unable to conenct to the database.");
  }
}

SQLiteDatabase::~SQLiteDatabase() { sqlite3_close(db); }

template <class T>
void SQLiteTableView<T>::prepare(std::string query) {
  int code = sqlite3_prepare_v2(db.getDb(), query.c_str(), -1, &res, 0);

  if (code != SQLITE_OK) {
    throw UnableToBuildStatementError("Error trying to build the statement: " +
                                      query);
  }
}

template <class T>
bool SQLiteTableView<T>::step() {
  int rc = sqlite3_step(res);

  if (rc == SQLITE_ERROR || rc == SQLITE_MISUSE) {
    throw UnableToStepError(
        "You may have forgotten to prepare your statement before stepping!");
  }

  if (rc == SQLITE_DONE) {
    return false;
  }

  SQLiteTableView<T>::last_row_fetched = buildTableRow();

  return true;
}

DirectoryTableRow DirectoryTableView::buildTableRow() {
  // Parent Id
  int parent_id = sqlite3_column_int(res, 3);
  parent_id = parent_id == 0 ? -1 : parent_id;

  return DirectoryTableRow{
      .id = sqlite3_column_int(res, 0),
      .name = std::string((const char*)sqlite3_column_text(res, 1)),
      .parent_id = parent_id};
}

HashTableRow HashTableView::buildTableRow() {
  // Hash
  // I believe I have to do this below because fupdes may have an error where
  // sometimes the blob is not filled out and it's only the partial blob.
  const uint8_t* blob = (const uint8_t*)sqlite3_column_blob(res, 10);
  const uint8_t* partial_blob = (const uint8_t*)sqlite3_column_blob(res, 8);

  if (blob == nullptr && partial_blob == nullptr) {
    throw InvalidHashTableError(
        "Hash table entry has an empty blob and partial blob!");
  }

  if (blob == nullptr) {
    blob = partial_blob;
  }

  uint8_t* blob_buffer = new uint8_t[MD5_DIGEST_LENGTH];
  std::memcpy(blob_buffer, blob, MD5_DIGEST_LENGTH);

  return {sqlite3_column_int(res, 0),
          std::string((const char*)sqlite3_column_text(res, 1)), blob_buffer};
}