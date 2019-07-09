#pragma once
#include "higanbana/core/system/memview.hpp"
#include "higanbana/core/global_debug.hpp"
#include "higanbana/core/datastructures/proxy.hpp"
#include <cstdio>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>

namespace higanbana
{
  class WatchFile
  {
    std::shared_ptr<std::atomic<bool>> m_updated = nullptr;

  public:

    WatchFile()
    {}

    WatchFile(std::shared_ptr<std::atomic<bool>> updated)
      : m_updated(updated)
    {}

    void react()
    {
      if (m_updated)
        *m_updated = false;
    }

    bool updated()
    {
      if (m_updated)
        return *m_updated;
      return false;
    }

    void update()
    {
      *m_updated = true;
    }

    bool empty()
    {
      return !m_updated;
    }
  };

  class MemoryBlob
  {
  private:
    std::vector<uint8_t> m_data;
  public:
    MemoryBlob();
    MemoryBlob(std::vector<uint8_t> data);
    size_t size();
    uint8_t* data();
  };

  class FileSystem
  {
  private:

    struct FileObj
    {
      size_t timeModified;
      vector<uint8_t> data;
    };
    std::string m_resolvedFullPath;
    higanbana::unordered_map<std::string, FileObj> m_files;
    higanbana::unordered_set<std::string> m_dirs;

    // watch
    std::unordered_map<std::string, vector<WatchFile>> m_watchedFiles;
    int rollingUpdate = 0;

    // 
    std::mutex m_lock;

    std::string getBasePath();

  public:
    FileSystem();
    FileSystem(std::string relativeOffset);
    bool fileExists(std::string path);
    MemoryBlob readFile(std::string path);
    higanbana::MemView<const uint8_t> viewToFile(std::string path);
    void loadDirectoryContentsRecursive(std::string path);
	void getFilesWithinDir(std::string path, std::function<void(std::string&, MemView<const uint8_t>)> func);
    size_t timeModified(std::string path);

    bool writeFile(std::string path, const uint8_t* ptr, size_t size);
    bool writeFile(std::string path, higanbana::MemView<const uint8_t> view);

    WatchFile watchFile(std::string path);
    void updateWatchedFiles();
  private:
    bool loadFileFromHDD(std::string path, size_t& size);
  };

}