#include <stdio.h>
#include <stdlib.h>

#include <jpeglib.h>
#include <png.h>

int load_jpeg_texture_image(const char *filename,
		unsigned int *width, unsigned int *height,
		unsigned char **_imgbuf)
{
	struct jpeg_decompress_struct cinfo;

	struct jpeg_error_mgr jerr;

	FILE *fp;
	JSAMPARRAY buffer;
	int row_stride;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "File %s cannot be opened.", filename);
		return 1;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);
	
	int w = cinfo.output_width;
	int h = cinfo.output_height;
	int c = cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr)&cinfo, JPOOL_IMAGE, w * c, 1);
	unsigned char *imgbuf = malloc(h * w * c);
	for (int j = 0; j < h; j = cinfo.output_scanline) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		for (int i = 0; i < w; ++i) {
			for (int k = 0; k < c; ++k) {
				imgbuf[(w * j + i) * c + k]
					= buffer[0][i * c + k];
			}
		}
	}

	jpeg_finish_decompress(&cinfo);
	fclose(fp);

	*width = w;
	*height = h;
	*_imgbuf = imgbuf;
	return 0;
}
