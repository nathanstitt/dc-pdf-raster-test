// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <string.h>
#include <string>
#include <utility>

#include <FreeImage.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <fstream>

#include "fpdf_dataavail.h"
#include "fpdf_ext.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"
#include "v8.h"

#include "bitmap.hpp"
#include "common.hpp"


static void WriteImage( SourcePDF *source, int stride,
                        int page_num, const char* buffer, int width, int height )
{
    bitmap_image bmp;
    bmp.setwidth_height(width, height,true);
    for (int h = 0; h < height; ++h) {
        const char* src_line = buffer + (stride * h);
        for (int w = 0; w < width; ++w) {
            bmp.set_pixel(w, h,
                          src_line[(w * 4) + 2],
                          src_line[(w * 4) + 1],
                          src_line[w * 4]);
        }
    }

    if ( Configuration::BMP_FORMAT == source->config->format ){
        bmp.save_image( source->dest_path_for(page_num, 1000) );
    } else {
        for (int page_width : DESIRED_PAGE_WIDTHS ) {
            std::string filename = source->dest_path_for(page_num, page_width);
            int new_height = ( page_width / (float)width  ) * height;
            // std::cout << width << "x" << height << " = " << page_width << "x" << new_height << std::endl;
            FIBITMAP *src_fi = FreeImage_ConvertFromRawBits((BYTE*)bmp.data(),
                                                         width, height,bmp.pitch(), 24, 1,1,1, true);
            FIBITMAP *resized = FreeImage_Rescale(src_fi,page_width,new_height,FILTER_BOX);
            FreeImage_Unload(src_fi);

            bool success = false;
            if ( Configuration::PNG_FORMAT == source->config->format ){
                success = FreeImage_Save(FIF_PNG, resized, filename.c_str(), 0);
            } else if ( Configuration::GIF_FORMAT == source->config->format ){
                FIBITMAP *gif = FreeImage_ColorQuantize(resized, FIQ_WUQUANT);
                success = FreeImage_Save(FIF_GIF, gif, filename.c_str(), GIF_DEFAULT);
                FreeImage_Unload(gif);
            }
            FreeImage_Unload(resized);
        }
    }

}

int Form_Alert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int) {
  printf("Form_Alert called.\n");
  return 0;
}

void Unsupported_Handler(UNSUPPORT_INFO*, int type) {
  std::string feature = "Unknown";
  switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
      feature = "XFA";
      break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
      feature = "Portfolios_Packages";
      break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
      feature = "Attachment";
      break;
    case FPDF_UNSP_DOC_SECURITY:
      feature = "Rights_Management";
      break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
      feature = "Shared_Review";
      break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
      feature = "Shared_Form";
      break;
    case FPDF_UNSP_ANNOT_3DANNOT:
      feature = "3D";
      break;
    case FPDF_UNSP_ANNOT_MOVIE:
      feature = "Movie";
      break;
    case FPDF_UNSP_ANNOT_SOUND:
      feature = "Sound";
      break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
      feature = "Screen";
      break;
    case FPDF_UNSP_ANNOT_SIG:
      feature = "Digital_Signature";
      break;
  }
  printf("Unsupported feature: %s.\n", feature.c_str());
}

class TestLoader {
public:
    TestLoader(const char* pBuf, size_t len);

    const char* m_pBuf;
    size_t m_Len;
};

TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len) {
}

int Get_Block(void* param, unsigned long pos, unsigned char* pBuf,
              unsigned long size) {
  TestLoader* pLoader = (TestLoader*) param;
  if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
  memcpy(pBuf, pLoader->m_pBuf + pos, size);
  return 1;
}

bool Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

void RenderPdf( SourcePDF *source, const char* pBuf, size_t len ) {
    std::cout << "Rendering PDF file " << source->basefilename << std::endl;

    IPDF_JSPLATFORM platform_callbacks;
    memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
    platform_callbacks.version = 1;
    platform_callbacks.app_alert = Form_Alert;

    FPDF_FORMFILLINFO form_callbacks;
    memset(&form_callbacks, '\0', sizeof(form_callbacks));
    form_callbacks.version = 1;
    form_callbacks.m_pJsPlatform = &platform_callbacks;

    TestLoader loader(pBuf, len);

    FPDF_FILEACCESS file_access;
    memset(&file_access, '\0', sizeof(file_access));
    file_access.m_FileLen = static_cast<unsigned long>(len);
    file_access.m_GetBlock = Get_Block;
    file_access.m_Param = &loader;

    FX_FILEAVAIL file_avail;
    memset(&file_avail, '\0', sizeof(file_avail));
    file_avail.version = 1;
    file_avail.IsDataAvail = Is_Data_Avail;

    FX_DOWNLOADHINTS hints;
    memset(&hints, '\0', sizeof(hints));
    hints.version = 1;
    hints.AddSegment = Add_Segment;

    FPDF_DOCUMENT doc;
    FPDF_AVAIL pdf_avail = FPDFAvail_Create(&file_avail, &file_access);

    (void) FPDFAvail_IsDocAvail(pdf_avail, &hints);

    if (!FPDFAvail_IsLinearized(pdf_avail)) {
        //        printf("Non-linearized path...\n");
        doc = FPDF_LoadCustomDocument(&file_access, NULL);
    } else {
        //        printf("Linearized path...\n");
        doc = FPDFAvail_GetDocument(pdf_avail, NULL);
    }

    (void) FPDF_GetDocPermissions(doc);
    (void) FPDFAvail_IsFormAvail(pdf_avail, &hints);

    FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnviroument(doc, &form_callbacks);
    FPDF_SetFormFieldHighlightColor(form, 0, 0xFFE4DD);
    FPDF_SetFormFieldHighlightAlpha(form, 100);

    int first_page = FPDFAvail_GetFirstPageNum(doc);
    (void) FPDFAvail_IsPageAvail(pdf_avail, first_page, &hints);

    int page_count = FPDF_GetPageCount(doc);
    for (int i = 0; i < page_count; ++i) {
        (void) FPDFAvail_IsPageAvail(pdf_avail, i, &hints);
    }

    FORM_DoDocumentJSAction(form);
    FORM_DoDocumentOpenAction(form);

    for (int i = 0; i < page_count; ++i) {
        FPDF_PAGE page = FPDF_LoadPage(doc, i);
        FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
        FORM_OnAfterLoadPage(page, form);
        FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_OPEN);

        int std_width = static_cast<int>(FPDF_GetPageWidth(page));
        int std_height = static_cast<int>(FPDF_GetPageHeight(page));
        int width  = 1000;
        int height = ( width / (float)std_width  ) * std_height;

        FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
        FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 255, 255, 255, 255);

        FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
        FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, 0);

        const char* buffer = reinterpret_cast<const char*>(
                                                           FPDFBitmap_GetBuffer(bitmap));
        int stride = FPDFBitmap_GetStride(bitmap);
        WriteImage(source, stride, i, buffer, width, height);

        FPDFBitmap_Destroy(bitmap);

        FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
        FORM_OnBeforeClosePage(page, form);
        FPDFText_ClosePage(text_page);
        FPDF_ClosePage(page);
    }

    FORM_DoDocumentAAction(form, FPDFDOC_AACTION_WC);
    FPDFDOC_ExitFormFillEnviroument(form);
    FPDF_CloseDocument(doc);
    FPDFAvail_Destroy(pdf_avail);

    printf("Loaded, parsed and rendered %d pages.\n", page_count);
}


int main(int argc, const char* argv[]) {
    v8::V8::InitializeICU();
    Configuration conf(argc,argv);
    if (!conf.valid){
        return 1;
    }

    FPDF_InitLibrary(NULL);

    UNSUPPORT_INFO unsuppored_info;
    memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
    unsuppored_info.version = 1;
    unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;

    FSDK_SetUnSpObjProcessHandler(&unsuppored_info);


    for (auto file : conf.source_files()) {

        std::cout << file.path << std::endl;
        file.contents( [&]( char* contents, unsigned int length ) {

                RenderPdf(&file, contents, length);

            });
    }
    // int exit_code=0;
    // DIR *dir=opendir(conf.src_dir.c_str());
    // if( dir ){
    //     struct dirent *entry;
    //     while( (entry = readdir(dir)) ){
    //         if( strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ){
    //             continue;
    //         }
    //         //const char* filename = files.front();
    //         const std::string src = conf.src_dir + "/" + entry->d_name;
    //         FILE* file = fopen( src.c_str(), "rb");
    //         if (!file) {
    //             fprintf(stderr, "Failed to open: %s\n", entry->d_name);
    //             continue;
    //         }
    //         (void) fseek(file, 0, SEEK_END);
    //         size_t len = ftell(file);
    //         (void) fseek(file, 0, SEEK_SET);
    //         char* pBuf = (char*) malloc(len);
    //         size_t ret = fread(pBuf, 1, len, file);
    //         (void) fclose(file);
    //         if (ret != len) {
    //             fprintf(stderr, "Failed to read: %s\n", entry->d_name);
    //         } else {
    //             RenderPdf(entry->d_name, pBuf, len,&conf);
    //         }
    //         free(pBuf);
    //     }
    //     closedir(dir);
    // } else {
    //     std::cerr << "Failed to open source directory: " << conf.src_dir << std::endl;
    //     exit_code = 1;
    // }

    FPDF_DestroyLibrary();
    return conf.valid;
}
