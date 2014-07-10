extern "C" {
#include "mupdf/fitz.h"
};
#include "bitmap.hpp"
#include "common.hpp"

#include <FreeImage.h>

void render_page(fz_context *ctx,fz_document *doc, int pagenumber, SourcePDF *file ){
    // Retrieve the number of pages (not used in this example).

    // Load the page we want. Page numbering starts from zero.

    fz_page *page = fz_load_page(doc, pagenumber);

    for (int width : DESIRED_PAGE_WIDTHS ) {

        // Calculate a transform to use when rendering. This transform
        // contains the scale and rotation. Convert zoom percentage to a
        // scaling factor. Without scaling the resolution is 72 dpi.

        fz_matrix transform;
        fz_rotate(&transform, 0);
        fz_pre_scale(&transform, 1.0, 1.0 );

        // Take the page bounds and transform them by the same matrix that
        // we will use to render the page.

        fz_rect bounds;
        fz_bound_page(doc, page, &bounds);
        fz_transform_rect(&bounds, &transform);

        int page_height = bounds.y1-bounds.y0,
            page_width  = bounds.x1-bounds.x0;

        int new_height = ( (float)width/(float)page_width  ) * page_height;

        float scalex = width / (float)page_width;
        float scaley = new_height / (float)page_height;

        // std::cout << page_width << "x" << page_height 
        //           << " * " << scalex << "x" << scaley
        //           << " = " << width << "x" << new_height
        //           << std::endl;

        // for later use
        page_height = new_height;
        page_width = width;

        fz_matrix scale_mat;

        fz_scale(&scale_mat, scalex, scaley);
        fz_concat(&transform, &transform, &scale_mat);
        fz_transform_rect(&bounds, &transform);

        // Create a blank pixmap to hold the result of rendering. The
        // pixmap bounds used here are the same as the transformed page
        // bounds, so it will contain the entire page. The page coordinate
        // space has the origin at the top left corner and the x axis
        // extends to the right and the y axis extends down.

        fz_irect bbox;
        fz_round_rect(&bbox, &bounds);
        fz_pixmap *pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), &bbox);
        fz_clear_pixmap_with_value(ctx, pix, 0xff);

        // Create a draw device with the pixmap as its target.
        // Run the page with the transform.

        fz_device *dev = fz_new_draw_device(ctx, pix);
        fz_run_page(doc, page, dev, &transform, NULL);
        fz_free_device(dev);

        std::string filename = file->dest_path_for(pagenumber,width);
        // Save the pixmap to a file.
        if ( Configuration::PNG_FORMAT == file->config->format ){
            fz_write_png(ctx, pix, (char*)filename.c_str(), 0);
        } else {
            // we write to a pnm since that appars to impose the
            // least amount of work on libfitz
            fz_buffer *buffer = fz_new_buffer(ctx, page_width * page_height * 3);
            fz_output *out = fz_new_output_with_buffer(ctx,buffer);
            fz_output_pnm_header(out, pix->w, pix->h, pix->n);
            fz_output_pnm_band(out, pix->w, pix->h, pix->n, 0, pix->h, pix->samples);
            fz_close_output(out);

            FIMEMORY *stream = FreeImage_OpenMemory(buffer->data, buffer->len);
            FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_PBM,stream);
            FreeImage_CloseMemory(stream);
            bool success = false;
            if ( Configuration::BMP_FORMAT == file->config->format ){
                success = FreeImage_Save(FIF_BMP, dib, filename.c_str(), 0);
            } else if ( Configuration::GIF_FORMAT == file->config->format ){
                FIBITMAP *gif = FreeImage_ColorQuantize(dib, FIQ_WUQUANT);
                success = FreeImage_Save(FIF_GIF, gif, filename.c_str(), GIF_DEFAULT);
                FreeImage_Unload(gif);
            }
            if ( ! success ){
                std::cerr << "Failed to safe image " << filename << std::endl;
            }
            // Clean up.
            FreeImage_Unload(dib);
            fz_drop_buffer(ctx,buffer);
        }
        fz_drop_pixmap(ctx, pix);
    }
    fz_free_page(doc, page);
}



int main(int argc,const char **argv)
{
    Configuration conf(argc,argv);
    if (!conf.valid){
        return 1;
    }

    // Create a context to hold the exception stack and various caches.
    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    fz_register_document_handlers(ctx);

    auto files = conf.source_files();
    for (auto file : files) {
        fz_document *doc = fz_open_document(ctx, file.path.c_str() );
        std::cout << file.basefilename << std::endl;
        int pagecount = fz_count_pages(doc);
        for ( int page = 0; page < pagecount; page++ ){
            render_page(ctx,doc,page,&file);
        }
        fz_close_document(doc);
    }
    fz_free_context(ctx);

    return conf.valid;
}
