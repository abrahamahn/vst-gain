#pragma once

#include <string>
#include <vector>

/**
  PresetStore (SQLite)
  --------------------
  A minimal wrapper that will store and retrieve presets from SQLite.
  This file is intentionally simple for now; it will be fleshed out
  once you decide the exact preset format and fields to store.

  Usage (future):
  - open(dbPath)
  - savePreset(name, blob)
  - loadPreset(name, outBlob)
*/
class PresetStore
{
public:
  PresetStore();
  ~PresetStore();

  bool open(const std::string& filePath);
  void close();

  bool savePreset(const std::string& name, const std::string& blob);
  bool loadPreset(const std::string& name, std::string& outBlob);
  bool deletePreset(const std::string& name);
  std::vector<std::string> listPresets() const;

  const std::string& lastError() const { return lastErr; }

private:
  struct Impl;
  Impl* impl { nullptr };
  std::string lastErr;
};
