/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#ifndef ZCACHE_HPP
#define ZCACHE_HPP

#include <string>
#include <unordered_map>
#include <list>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include "zds.hpp"

/**
 * Thread-local LRU cache for UID/GID lookups
 */
class UidGidCache {
public:
    static constexpr size_t MAX_CACHE_SIZE = 256;
    static constexpr time_t CACHE_TTL = 300; // 5 minutes
    
    struct CacheEntry {
        std::string name;
        time_t timestamp;
        
        CacheEntry() : timestamp(0) {}
        CacheEntry(const std::string& n, time_t t) : name(n), timestamp(t) {}
    };
    
    // Helper methods
    static void evict_oldest_uid();
    static void evict_oldest_gid();
    static void update_uid_lru(uid_t uid);
    static void update_gid_lru(gid_t gid);
    /**
     * Get username for UID with caching
     */
    static std::string get_username(uid_t uid);
    
    /**
     * Get group name for GID with caching  
     */
    static std::string get_groupname(gid_t gid);
    
    /**
     * Clear all cached entries (for testing)
     */
    static void clear_cache();
    
    /**
     * Get cache statistics (for testing/monitoring)
     */
    static void get_cache_stats(size_t& uid_entries, size_t& gid_entries);
};

struct DirectoryEntry {
    std::string name;
    struct stat stats;
    bool valid;
    
    DirectoryEntry() : valid(false) {}
    DirectoryEntry(const std::string& n, const struct stat& s, bool v) 
        : name(n), stats(s), valid(v) {}
};

/**
 * Secure per-user directory metadata cache
 */
class SecureDirectoryCache {
public:
    static constexpr time_t CACHE_TTL = 10; // 10 seconds
    static constexpr size_t MAX_CACHE_ENTRIES = 1000;
    
    struct CacheKey {
        std::string path;
        uid_t user_uid;
        gid_t user_gid;
        
        bool operator==(const CacheKey& other) const {
            return path == other.path && user_uid == other.user_uid && user_gid == other.user_gid;
        }
    };
    
    struct CacheKeyHash {
        size_t operator()(const CacheKey& key) const {
            return std::hash<std::string>{}(key.path) ^ 
                   std::hash<uid_t>{}(key.user_uid) ^ 
                   std::hash<gid_t>{}(key.user_gid);
        }
    };
    
    struct CachedDirListing {
        std::vector<DirectoryEntry> entries;
        time_t cache_time;
        time_t dir_mtime;
        time_t dir_ctime;
        uid_t cached_for_uid;
        
        CachedDirListing() : cache_time(0), dir_mtime(0), dir_ctime(0), cached_for_uid(0) {}
    };
    
    // Helper methods
    static bool is_cache_entry_valid(const CacheKey& key, const CachedDirListing& cached);
    static void evict_oldest_entry();
    static void update_lru(const CacheKey& key);
    /**
     * Get cached directory listing if available and valid
     */
    static bool get_cached_listing(const std::string& dir_path, 
                                 std::vector<DirectoryEntry>& entries);
    
    /**
     * Cache directory listing with security validation
     */
    static void cache_listing(const std::string& dir_path, 
                            const std::vector<DirectoryEntry>& entries);
    
    /**
     * Clear all cached entries
     */
    static void clear_cache();
    
    /**
     * Get cache statistics
     */
    static size_t get_cache_size();
};

/**
 * Secure PDS member list cache
 */
class PDSMemberCache {
public:
    static constexpr time_t CACHE_TTL = 30; // 30 seconds for PDS (longer than USS)
    static constexpr size_t MAX_CACHE_ENTRIES = 500;
    
    struct CachedMemberList {
        std::vector<ZDSMem> members;
        time_t cache_time;
        uid_t cached_for_uid;
        std::string dataset_key; // For invalidation detection
        
        CachedMemberList() : cache_time(0), cached_for_uid(0) {}
    };
    
    // Helper methods
    static bool is_cache_entry_valid(const std::string& dsn, const CachedMemberList& cached);
    static void evict_oldest_entry();
    static void update_lru(const std::string& dsn);
    static std::string generate_dataset_key(const std::string& dsn);
    /**
     * Get cached PDS member list if available and valid
     */
    static bool get_cached_members(const std::string& dsn, std::vector<ZDSMem>& members);
    
    /**
     * Cache PDS member list
     */
    static void cache_members(const std::string& dsn, const std::vector<ZDSMem>& members);
    
    /**
     * Clear all cached entries
     */
    static void clear_cache();
    
    /**
     * Invalidate cache entry for a specific dataset
     */
    static void invalidate_dataset(const std::string& dsn);
    
    /**
     * Get cache statistics
     */
    static size_t get_cache_size();
};

#endif