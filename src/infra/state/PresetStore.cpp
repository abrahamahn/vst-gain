/**
  PresetStore.cpp
  ---------------
  SQLite-backed preset storage.

  Table:
  - name (primary key)
  - data (binary blob of APVTS state)
  - updated_at (unix timestamp)
*/
#include "PresetStore.h"

#include <chrono>

#if USE_SQLITE
  #include <sqlite3.h>
#endif

struct PresetStore::Impl
{
#if USE_SQLITE
  sqlite3* db { nullptr };
#endif
};

namespace
{
#if USE_SQLITE
int64_t nowUnixSeconds()
{
  using namespace std::chrono;
  return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}
#endif
}

PresetStore::PresetStore() : impl(new Impl())
{
}

PresetStore::~PresetStore()
{
  close();
  delete impl;
}

bool PresetStore::open(const std::string& filePath)
{
#if USE_SQLITE
  lastErr.clear();

  if (impl->db)
    close();

  if (sqlite3_open(filePath.c_str(), &impl->db) != SQLITE_OK)
  {
    lastErr = sqlite3_errmsg(impl->db);
    sqlite3_close(impl->db);
    impl->db = nullptr;
    return false;
  }

  const char* sql =
    "CREATE TABLE IF NOT EXISTS presets("
    "name TEXT PRIMARY KEY,"
    "data BLOB NOT NULL,"
    "updated_at INTEGER NOT NULL"
    ");";

  char* errMsg = nullptr;
  if (sqlite3_exec(impl->db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
  {
    lastErr = errMsg ? errMsg : "Failed to create presets table";
    sqlite3_free(errMsg);
    return false;
  }

  return true;
#else
  (void) filePath;
  return false;
#endif
}

void PresetStore::close()
{
#if USE_SQLITE
  if (impl && impl->db)
  {
    sqlite3_close(impl->db);
    impl->db = nullptr;
  }
#endif
}

bool PresetStore::savePreset(const std::string& name, const std::string& blob)
{
#if USE_SQLITE
  if (!impl->db)
    return false;

  lastErr.clear();

  const char* sql =
    "INSERT OR REPLACE INTO presets(name, data, updated_at) VALUES(?, ?, ?);";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
  {
    lastErr = sqlite3_errmsg(impl->db);
    return false;
  }

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
  if (blob.empty())
    sqlite3_bind_blob(stmt, 2, nullptr, 0, SQLITE_TRANSIENT);
  else
    sqlite3_bind_blob(stmt, 2, blob.data(), (int) blob.size(), SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 3, nowUnixSeconds());

  const int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  if (rc != SQLITE_DONE)
  {
    lastErr = sqlite3_errmsg(impl->db);
    return false;
  }

  return true;
#else
  (void) name;
  (void) blob;
  return false;
#endif
}

bool PresetStore::loadPreset(const std::string& name, std::string& outBlob)
{
#if USE_SQLITE
  if (!impl->db)
    return false;

  lastErr.clear();

  const char* sql = "SELECT data FROM presets WHERE name = ?;";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
  {
    lastErr = sqlite3_errmsg(impl->db);
    return false;
  }

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

  const int rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW)
  {
    const void* blob = sqlite3_column_blob(stmt, 0);
    const int size = sqlite3_column_bytes(stmt, 0);
    outBlob.assign(static_cast<const char*>(blob), static_cast<size_t>(size));
    sqlite3_finalize(stmt);
    return true;
  }

  sqlite3_finalize(stmt);
  return false;
#else
  (void) name;
  (void) outBlob;
  return false;
#endif
}

bool PresetStore::deletePreset(const std::string& name)
{
#if USE_SQLITE
  if (!impl->db)
    return false;

  lastErr.clear();

  const char* sql = "DELETE FROM presets WHERE name = ?;";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
  {
    lastErr = sqlite3_errmsg(impl->db);
    return false;
  }

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

  const int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  if (rc != SQLITE_DONE)
  {
    lastErr = sqlite3_errmsg(impl->db);
    return false;
  }

  return true;
#else
  (void) name;
  return false;
#endif
}

std::vector<std::string> PresetStore::listPresets() const
{
  std::vector<std::string> names;

#if USE_SQLITE
  if (!impl->db)
    return names;

  const char* sql = "SELECT name FROM presets ORDER BY updated_at DESC;";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    return names;

  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text)
      names.emplace_back(reinterpret_cast<const char*>(text));
  }

  sqlite3_finalize(stmt);
#endif

  return names;
}
