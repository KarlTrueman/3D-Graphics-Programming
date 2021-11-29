#pragma once

#include "ExternalLibraryHeaders.h"

namespace Helpers
{
	// Helper utilising FreeImage to load images / textures
	// Loaded format is guaranteed to be 32 bit RGBA layout
	class ImageLoader
	{
	private:
		int m_width{ 0 };
		int m_height{ 0 };
		BYTE* m_data{ nullptr };
	public:
		~ImageLoader() { delete []m_data; }

		// Width in texels of the image
		int Width() const { return m_width; }

		// Height in texels of the image
		int Height() const { return m_height; }

		// Attempt to load an image from the file and path provided. Returns false on error.
		bool Load(const std::string& filepath);

		// Allows access to the raw bytes that make up the image laid out in RGBA format (8 bits per channel)
		BYTE* GetData() const { return m_data; }

		// Returns a grey scale value at provided uv, useful for RMA textures
		BYTE GetGreyValue(float u, float v) const;
	};

	// Saves an image to the file and path provided. Returns false on error.
	// Assumes RGBA 32 bit format. Therefore data size must be width * height * 4
	// Creates a .png file so you don't add an extension to the passed in filepath
	bool SaveImage(GLubyte* data, int width,int height, const std::string& filepath);
}