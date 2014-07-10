#include "common.hpp"
#include <dirent.h>
#include <sstream>
extern "C"{
#include <libgen.h>
}
#include <cstdio>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>


/////////////////////////////////////////////////////////////////////
// SourcePDF class                                             //
/////////////////////////////////////////////////////////////////////

std::string
SourcePDF::dest_path_for( int page_number, int size ){
    std::ostringstream path;
    std::string base(  basename((char*) basefilename.c_str()) );
    path << config->dest_dir << "/" << base
         << "-" << std::to_string(page_number) << "-" << std::to_string(size);
    if ( Configuration::GIF_FORMAT == config->format ){
        path << ".gif";
    } else if ( Configuration::PNG_FORMAT == config->format ){
        path << ".png";
    } else if ( Configuration::BMP_FORMAT == config->format ){
        path << ".bmp";
    }
    return path.str();
}



void
SourcePDF::contents( std::function<void(char* ,unsigned int)> func ){
    std::ifstream is (path, std::ifstream::binary);
    if (!is) {
        std::cerr << "Unable to read " << path << std::endl;
        return;
    }

    is.seekg (0, is.end);
    unsigned int length = is.tellg();
    is.seekg (0, is.beg);

    char * buffer = new char [length];

    is.read (buffer,length);

    if (!is)
        std::cerr << "error: only " << is.gcount() << " could be read of " << path << std::endl;

    is.close();

    func( buffer, length );

    delete[] buffer;
}


 /////////////////////////////////////////////////////////////////////
 // Configuration class                                             //
 /////////////////////////////////////////////////////////////////////

Configuration::Configuration(int argc, const char* argv[]) {
    if ( argc != 4 ){
        std::cerr << "Usage is: " << argv[0] << " <png|gif> <source directory> <destination directory>" << std::endl;
        valid = false;
    } else {
        std::string ext = argv[1];
        if ( strcmp(argv[1], "png") == 0){
            format = PNG_FORMAT;
        } else if ( strcmp(argv[1], "gif") == 0){
            format = GIF_FORMAT;
        } else if ( strcmp(argv[1], "bmp") == 0){
            format = BMP_FORMAT;
        } else {
            std::cerr << "Unknown extension " << argv[1] << " must be one of <png|gif>" << std::endl;
            valid = false;
        }

        src_dir= argv[2];
        dest_dir=argv[3];
        valid = true;
    }
}

std::list<SourcePDF>
Configuration::source_files(){
    std::list<SourcePDF> files;
    DIR *dir=opendir(src_dir.c_str());
    if( dir ){
        struct dirent *entry;
        while( (entry = readdir(dir)) ){
            if( strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ){
                continue;
            }
            SourcePDF file;
            file.config = this;
            file.path = src_dir + "/" + entry->d_name;
            file.basefilename = entry->d_name;
            files.push_back(file);
        }
        closedir(dir);
    } else {
        std::cerr << "Failed to open source directory: " << src_dir << std::endl;
    }
    return files;
}
