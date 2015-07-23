#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <memory>
#include <malloc.h>

#include <vector>

int Width = 0;
int Height = 0;

std::vector<JOCTET> my_buffer;
#define BLOCK_SIZE 16384

void my_init_destination(j_compress_ptr cinfo)
{
	my_buffer.resize(BLOCK_SIZE);
	cinfo->dest->next_output_byte = &my_buffer[0];
	cinfo->dest->free_in_buffer = my_buffer.size();
}

boolean my_empty_output_buffer(j_compress_ptr cinfo)
{
	size_t oldsize = my_buffer.size();
	my_buffer.resize(oldsize + BLOCK_SIZE);
	cinfo->dest->next_output_byte = &my_buffer[oldsize];
	cinfo->dest->free_in_buffer = my_buffer.size() - oldsize;
	return true;
}

void my_term_destination(j_compress_ptr cinfo)
{
	my_buffer.resize(my_buffer.size() - cinfo->dest->free_in_buffer);
}


void write_jpeg_file(const char* fileName, unsigned char* imageBuffer)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	FILE * outfile;
	unsigned char* line;

	if ((outfile = fopen(fileName, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", fileName);
		exit(1);
	}

	jpeg_stdio_dest(&cinfo, outfile);


	cinfo.image_width = Width;
	cinfo.image_height = Height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);


	cinfo.dct_method = JDCT_FLOAT;
	jpeg_set_quality(&cinfo, 15, true);


	jpeg_start_compress(&cinfo, TRUE);

	JSAMPROW row_pointer[1];
	int row_stride;
	row_stride = Width * 3;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = &imageBuffer[cinfo.next_scanline * cinfo.image_width * cinfo.input_components];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(outfile);
}

unsigned char* write_jpeg_memory(unsigned char* imageBuffer)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	unsigned long buf_size = 0;
	unsigned char* buf = NULL;

	jpeg_mem_dest(&cinfo, &buf, &buf_size);


	cinfo.image_width = Width;
	cinfo.image_height = Height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);


	cinfo.dct_method = JDCT_FLOAT;
	jpeg_set_quality(&cinfo, 60, true);


	jpeg_start_compress(&cinfo, TRUE);

	JSAMPROW row_pointer[1];
	int row_stride;
	row_stride = Width * 3;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = &imageBuffer[cinfo.next_scanline * cinfo.image_width * cinfo.input_components];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	return buf;
}

unsigned char* read_jpeg_file(const char* fileName)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	unsigned long location = 0;
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	FILE * infile;
	unsigned char* line;
	
	if ((infile = fopen(fileName, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", fileName);
		exit(1);
	}
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);

	printf("JPEG File Information: \n");
	printf("Image width and height: %d pixels and %d pixels.\n", cinfo.image_width, cinfo.image_height);
	printf("Color components per pixel: %d.\n", cinfo.num_components);
	printf("Color space: %d.\n", cinfo.jpeg_color_space);

	jpeg_start_decompress(&cinfo);

	Width = cinfo.image_width;
	Height = cinfo.image_height;

	size_t size = cinfo.output_width*cinfo.output_height*cinfo.num_components;
	unsigned char* dest = (unsigned char*)malloc(size);

	JSAMPROW row_pointer[1];
	row_pointer[0] = (unsigned char *)malloc(cinfo.output_width*cinfo.num_components);
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for (int i = 0; i < cinfo.image_width*cinfo.num_components; i++)
		{
			dest[location++] = row_pointer[0][i];
		}
	}
		/*line = dest + (cinfo.input_scan_number * cinfo.output_scanline);
		jpeg_read_scanlines(&cinfo, &line, 1);*/

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	FILE* outFile = fopen("res.raw", "wb");
	fwrite(dest, size, 1, outFile);
	fclose(outFile);

	//free(dest);
	return dest;
}

unsigned char* read_jpeg_memory(unsigned char* buf)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	unsigned long location = 0;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	//FILE * infile;
	unsigned char* line;

	unsigned long buf_size = _msize(buf);//====================================================================================
	//unsigned long buf_size = malloc_
	jpeg_mem_src(&cinfo, buf, buf_size);
	jpeg_read_header(&cinfo, TRUE);
	

	printf("JPEG File Information: \n");
	printf("Image width and height: %d pixels and %d pixels.\n", cinfo.image_width, cinfo.image_height);
	printf("Color components per pixel: %d.\n", cinfo.num_components);
	printf("Color space: %d.\n", cinfo.jpeg_color_space);
	printf("value: %s", buf);

	jpeg_start_decompress(&cinfo);

	Width = cinfo.image_width;
	Height = cinfo.image_height;

	size_t size = cinfo.output_width*cinfo.output_height*cinfo.num_components;
	unsigned char* dest = (unsigned char*)malloc(size);

	JSAMPROW row_pointer[1];
	row_pointer[0] = (unsigned char *)malloc(cinfo.output_width*cinfo.num_components);
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for (int i = 0; i < cinfo.image_width*cinfo.num_components; i++)
		{
			dest[location++] = row_pointer[0][i];
		}
	}
	
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	//free(dest);
	return dest;
}

unsigned char* read_file(char* fileName)
{
	struct stat filestatus;

	FILE* inFile = fopen(fileName, "rb");

	stat(fileName, &filestatus);

	unsigned char* dest = (unsigned char*)malloc(filestatus.st_size);
	//memset(dest, 0, filestatus.st_size);
	
	size_t read = fread(dest, filestatus.st_size, 1, inFile);

	fclose(inFile);

	return dest;
}

int main(int argc, char* argv[])
{
	//unsigned char* dest = read_jpeg_file("img.jpg");
	unsigned char* dest_t = read_file("img.jpg");

	unsigned char* dest = read_jpeg_memory(dest_t);

	//write_jpeg_file("out_jpg.jpg", dest);

	unsigned char* res = write_jpeg_memory(dest);

	FILE* outF = fopen("res_jpg.jpg", "wb");
	fwrite(res, _msize(res), 1, outF);
	fclose(outF);

	free(res);
	free(dest);
	free(dest_t);

	return 0;
}