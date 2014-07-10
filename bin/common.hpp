#ifndef _INCLUDE_COMMON_HPP_
#define _INCLUDE_COMMON_HPP_

#include <list>
#include <functional>
#include <string>

class Configuration;

class SourcePDF {
public:
    std::string path;
    std::string basefilename;
    Configuration *config;

    std::string dest_path_for( int page_number, int size );

    void contents( std::function<void(char* ,unsigned int)> func );

};

static int DESIRED_PAGE_WIDTHS [] = { 1000, 700, 180, 60 };

class Configuration {

public:

    enum file_format_t {GIF_FORMAT,PNG_FORMAT,BMP_FORMAT};
    std::string src_dir, dest_dir;
    file_format_t format;
    bool valid;

    Configuration(int argc, const char* argv[]);

    std::list<SourcePDF>
    source_files();
};

#endif // _INCLUDE_COMMON_HPP_
