#include "ImageLoader.h"
#include <filesystem>
namespace fs = std::filesystem;

namespace Helpers
{
	BYTE ImageLoader::GetGreyValue(float u, float v) const
	{
		u = fmod(u, 1.0f);
		v = fmod(v, 1.0f);

		// Nearest
		const int x = (int)(u * (m_width - 1));
		const int y = (int)(v * (m_height - 1));

		BYTE alpha{ m_data[(x + y * m_width) * 4 + 3] };
		if (alpha == 0)
			return 0;

		BYTE red{ m_data[(x + y * m_width) * 4] };

		if (alpha == 255 || red == 0)
			return red;

		BYTE calc = (BYTE)(red * alpha / 255.0f);
		return calc;
	}

	// Attempt to load an image from the file and path provided. Returns false on error.
	bool ImageLoader::Load(const std::string& filepath)
	{
		
		// First check file exists
		if (!exists(fs::path(filepath)))
		{
			std::cout << "File does not exist: " << filepath << std::endl;
			return false;
		}

		// Determine the format of the image.
		FREE_IMAGE_FORMAT format{ FreeImage_GetFileType(filepath.c_str(), 0) };

		// Found image, but couldn't determine the file format? Try again...
		if (format == FIF_UNKNOWN)
		{
			std::cout << "Couldn't determine file format - attempting to get from file extension..." << std::endl;

			format = FreeImage_GetFIFFromFilename(filepath.c_str());

			// Check format is supported
			if (!FreeImage_FIFSupportsReading(format))
			{
				std::cout << "Detected image format cannot be read!" << std::endl;
				return false;
			}
		}

		// If we're here we have a known image format, so load the image into a bitmap
		FIBITMAP* bitmap{ FreeImage_Load(format, filepath.c_str()) };

		// How many bits-per-pixel is the source image?
		unsigned int bitsPerPixel{ FreeImage_GetBPP(bitmap) };
				
		// Grab size
		m_width = FreeImage_GetWidth(bitmap);
		m_height = FreeImage_GetHeight(bitmap);

		// Convert our image to 32 bits (8 bits per channel, Red/Green/Blue/Alpha) if not already
		FIBITMAP* bitmap32{ nullptr };
		if (bitsPerPixel == 32)
		{
			bitmap32 = bitmap;
		}
		else
		{
			bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
			if (!bitmap32)
			{
				const FREE_IMAGE_TYPE image_type{ FreeImage_GetImageType(bitmap) };
				if (image_type == FIT_UINT16)
				{
					// FreeImage seems to have an issue converting 16 bit grey scale images to 32 so handling this manually
					UINT16* textureData{ (UINT16*)FreeImage_GetBits(bitmap) };

					m_data = new GLubyte[(size_t)m_width * (size_t)m_height * 4];
					size_t count{ 0 };
					for (size_t i = 0; i < (size_t)m_width * (size_t)m_height * 4; i += 4)
					{
						BYTE asByte = (BYTE)(textureData[count++] / 256.0f);
						m_data[i] = m_data[i + 1] = m_data[i + 2] = asByte;
						m_data[i + 3] = 255;
					}

					return true;
				}

				std::cout << "ImageLoader::Load failed to convert image to 32 bits" << std::endl;

				return false;
			}
		}		

		// Get a pointer to the texture data as an array of unsigned bytes.
		// Note: At this point bitmap32 ALWAYS holds a 32-bit colour version of our image - so we get our data from that.
		// Also, we don't need to delete or delete[] this textureData because it's not on the heap (so attempting to do
		// so will cause a crash) - just let it go out of scope and the memory will be returned to the stack.
		BYTE* textureData{ FreeImage_GetBits(bitmap32) };

		m_data = new GLubyte[(size_t)m_width * (size_t)m_height * 1 * 4];

		// Copy to mine, Note: Freeimage data format is GL_BGRA while I need GL_RGBA
		// Iterate through the pixels, copying the data
		// from 'pixels' to 'bits' except in RGBA format.		

		//for (int pix = 0; pix < m_width * m_height; pix++)
		//{
		//	m_data[pix * 4 + 0] = textureData[pix * 4 + 2];	//r = 3
		//	m_data[pix * 4 + 1] = textureData[pix * 4 + 1];	//g = 0
		//	m_data[pix * 4 + 2] = textureData[pix * 4 + 0];	//b = 1
		//	m_data[pix * 4 + 3] = textureData[pix * 4 + 3];	//a = 2
		//}
		
		// 15/04/20: Rebuilt FreeImage with correct order so now RGBA so no need to convert = quicker :)
		memcpy(m_data, textureData, (size_t)m_width * (size_t)m_height * 1 * 4);
		
		// Unload the 32-bit colour bitmap
		FreeImage_Unload(bitmap32);

		// If we had to do a conversion to 32-bit colour, then unload the original
		// non-32-bit-colour version of the image data too. Otherwise, bitmap32 and
		// bitmap point at the same data, and that data's already been free'd, so
		// don't attempt to free it again! (or we'll crash).
		if (bitsPerPixel != 32)
			FreeImage_Unload(bitmap);

		return true;
	}

	// Attempt to save an image to the file and path provided. Returns false on error.
	// Assumes RGBA 32 bit format. Therefore data size must be width * height * 4
	// Creates a .png file so you don't need to add an extension to filepath
	bool SaveImage(GLubyte* data, int width, int height, const std::string& filepath)
	{
		BOOL topDown=0;
		FIBITMAP* bitmap{ FreeImage_ConvertFromRawBits((BYTE*)data, width, height,width *4,32,0,0,0, topDown) };

		BOOL res{ FreeImage_Save(FIF_PNG, bitmap, (filepath + ".png").c_str()) };

		FreeImage_Unload(bitmap);

		return (res == 1);
	}


}