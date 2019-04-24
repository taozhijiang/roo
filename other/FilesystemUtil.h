/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_OTHER_FILESYSTEM_UTIL_H__
#define __ROO_OTHER_FILESYSTEM_UTIL_H__

// 方便文件系统中文件、路径、访问等方面的便捷函数

// std::filesystem will available at C++17

#include <cstdio>
#include <unistd.h>
#include <climits>
#include <cstdlib>

#include <dirent.h>
#include <fstream>

#include <string>


namespace roo {

class FilesystemUtil {

public:

    static std::string filename(const std::string& path) {

        std::string n_path = normalize_path(path);
        if(is_directory(path, false)) 
            return "";

        auto pos = n_path.find_last_of('/');
        if(pos == std::string::npos)
            return n_path;
        
        return n_path.substr(pos + 1);
    }

    static std::string directory(const std::string& path) {

        std::string n_path = normalize_path(path);
        if(is_directory(path, false)) 
            return n_path;

        auto pos = n_path.find_last_of('/');
        if(pos == std::string::npos)
            return ".";
        
        return n_path.substr(0, pos);
    }

    static bool is_directory(const std::string& path, bool norm = false) {
        
        std::string n_path = path;
        if(norm)
            n_path = normalize_path(path);

        if(n_path.empty()) return false;

        struct stat f_stat {};
        if(S_ISDIR(f_stat.st_mode))
            return true;
        
        return false;
    }

    // 返回父目录，不带 /
    static std::string parent_directory(const std::string& path) {
        
        std::string n_path = normalize_path(path);
        if(n_path.size() == 1 && n_path[0] == '/')
            return "/";

        auto pos = n_path.find_last_of('/');
        assert(pos != std::string::npos && pos > 0);

        return n_path.substr(0, pos - 1);
    }


    // file access and operation

    static bool exists(const std::string& path) {
        std::string n_path = normalize_path(path);
        return ::access(n_path.c_str(), F_OK) == 0;
    }

    // R_OK, W_OK, X_OK
    static bool accessable(const std::string& path, int how ) {
        std::string n_path = normalize_path(path);
        return ::access(n_path.c_str(), how & (R_OK | W_OK | X_OK)) == 0;
    }

    static int read_file(const std::string& path, std::string& content) {
        
        if(!accessable(path, R_OK))
            return -1;
        
        std::ifstream ifs(path, std::ios::binary | std::ios::ate);
        if(!ifs.is_open())
            return -1;
        
        size_t filelen = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        std::vector<char> buff(filelen);
        if (ifs.read(buff.data(), filelen)) {
            content.assign(buff.begin(), buff.end());
            return 0;
        }
        
        return -1;
    }

    static int read_file(const std::string& path, std::vector<std::string>& lines) {
        
        if(!accessable(path, R_OK))
            return -1;
        
        std::ifstream ifs(path, std::ios::binary);
        if(!ifs.is_open())
            return -1;
        
        lines.clear();
        ifs.seekg(0, std::ios::beg);
        std::string line;

        while (!ifs.eof()) {
            std::getline(ifs, line);
            lines.push_back(line);
        }
        
        ifs.close();
        return 0;
    }

    static int append_file(const std::string& path, const std::string& line, bool append_nl = false) {
                
        std::ofstream ofs(path, std::ios::binary | std::ios::app);
        if(!ofs.is_open())
            return -1;

        ofs << line;
        if(append_nl)
            ofs << ofs.widen('\n');

        ofs.close();  
        return 0;
    }

    static int list_directory(const std::string& path, std::vector<std::string> dir_files) {

        std::string n_path = normalize_path(path);
        if(!is_directory(n_path) || !accessable(n_path, R_OK | X_OK) ) 
            return -1;

         // 遍历目录

        DIR *d = NULL;
        struct dirent* d_item = NULL;

        if (!(d = opendir(n_path.c_str()))) {
            return -1;
        }

        dir_files.clear();
        while ( (d_item = readdir(d)) != NULL ) {

            // 跳过隐藏目录
            if (::strncmp(d_item->d_name, ".", 1) == 0) {
                continue;
            }

            dir_files.push_back(d_item->d_name);
        }

        return 0;
    }

    // 优化路径名字, readlink会进行软连接展开、/、..等符号的处理
    // produce a canonicalized absolute pathname
    static std::string normalize_path(const std::string& str) {

        std::string copy_str = str;
        size_t index = 0;

        // trim left whitespace
        for (index = 0; index < copy_str.size() && isspace(copy_str[index]); ++index)
            /* do nothing*/;
        copy_str.erase(0, index);

        // trim right whitespace
        for (index = copy_str.size(); index > 0 && isspace(copy_str[index - 1]); --index)
            /* do nothing*/;
        copy_str.erase(index);

        // 删除连续的 /
        std::string result{};
        for (size_t i = 0; i < copy_str.size(); ++i) {
            if (copy_str[i] == '/' && !result.empty() && result.at(result.size() - 1) == '/') {
                continue;
            }
            result.push_back(copy_str[i]);
        }

        // 删除末尾的 /
        if (!result.empty() && result.at(result.size() - 1) == '/')
            result.erase(result.size() - 1);

        return result;
    }

};


} // end namespace roo


#endif // __ROO_OTHER_FILESYSTEM_UTIL_H__