#include "stdafx.h"
#include "pngwriter.h"
#include "png.h"

//extern "C" void exit (int) throw (); // WHY I DO NOT KNOW

PNGWriter::PNGWriter(void)
{
	filename = "output";
	filenumber = 1;
	writeascii = false;
}

PNGWriter::PNGWriter(string filename)
{
	this->filename = filename;
	filenumber = 1;
	writeascii = false;
}

PNGWriter::~PNGWriter(void)
{
}

void PNGWriter::SetFileName(string file)
{
	filename = file;
}

bool PNGWriter::WriteImage(void * ptr)
{
	if (!OpenFile(true))
		return false;

	// Determine size of current buffer
	int width, height;
	int dims[4];    
	glGetIntegerv(GL_VIEWPORT, dims);
	width = dims[2];
	height = dims[3];
	unsigned char* imageData;

	
	// Grab image data
	if (ptr == NULL)
	{
		//imageData = (unsigned char*)malloc(sizeof(unsigned char)*width*height*3);
		imageData = new unsigned char[width*height*3];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glFinish();
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, imageData);
		glFinish();
	}
	else
	{
		imageData = (unsigned char*)ptr;
	}

	// Initialize PNG data structures

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
    NULL, NULL);
    if (!png_ptr)
       return (ERROR);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
       png_destroy_write_struct(&png_ptr,
         (png_infopp)NULL);
       return (ERROR);
    }

	if(setjmp(png_jmpbuf(png_ptr)))
    {
       png_destroy_write_struct(&png_ptr, &info_ptr);
       fclose(fp);
       return (ERROR);
	}

	png_init_io(png_ptr, fp);

	// Now setup the png header data
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Load image data into structures
	png_bytepp row_pointers;
	row_pointers = (png_bytepp)png_malloc(png_ptr, height*sizeof(png_bytep)); // allocate pointers
	for (int i = 0; i < height; i++)
		row_pointers[i]=(png_bytep)png_malloc(png_ptr, width*3); // Allocate row data
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width*3; j+=3) // swap row order from imageData
		{
            row_pointers[i][j] = (png_byte)imageData[((height-1-i)*width)*3 + j + 0];
			row_pointers[i][j+1] = (png_byte)imageData[((height-1-i)*width)*3 + j + 1];
			row_pointers[i][j+2] = (png_byte)imageData[((height-1-i)*width)*3 + j + 2];
		}
	}

	png_set_rows(png_ptr, info_ptr, row_pointers);

    // Write PNG file
	int png_transforms = PNG_TRANSFORM_IDENTITY;
	png_write_png(png_ptr, info_ptr, png_transforms, NULL);

	for (int i = 0; i < height; i++)
		free(row_pointers[i]);
	free(row_pointers);

	fclose(fp);
	// Now create an ASCII .dat file with greyscale values
	if (writeascii)
	{
		OpenFile(false);
		double red, green, blue;
		//fprintf(fp, "%d, %d\r\n", width, height);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width*3; j+=3)
			{
				red = (double)imageData[((height-1-i)*width)*3 + j + 0];
				green = (double)imageData[((height-1-i)*width)*3 + j + 1];
				blue = (double)imageData[((height-1-i)*width)*3 + j + 2];
				if (j != (width-1)*3)
					fprintf(fp, "%d, ", (int)((red + green + blue) / 3.0) );
				else
					fprintf(fp, "%d\r\n", (int)((red + green + blue) / 3.0) );
			}
		}
		fclose(fp);
	}

	if (ptr == NULL)
		delete[] imageData;

	fileopen = false;
	filenumber++;
	return true;
}

bool PNGWriter::OpenFile(bool png)
{
	char buffer[510];
	if (png)
		sprintf(buffer, "%s%06d.png", filename.c_str(), filenumber); 
	else
		sprintf(buffer, "%s%06d.dat", filename.c_str(), filenumber);
	fp = fopen(buffer, "wb");
	if (!fp)
	{
		fileopen = false;
		return false;
	}
	fileopen = true;
	return true;
}