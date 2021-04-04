/* Dig Dug Textures - Created by Cfir Aguston
   Textures are extracted directly from the binary file */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "SDL.h"

#define WINDOW_WIDTH		320		/* CGA width									*/
#define WINDOW_HEIGHT		200		/* CGA height									*/
#define CGA_PIXELS_PER_BYTE	4		/* CGA pixels per byte							*/
#define CGA_PIXEL_SIZE		2		/* Number of pixels to draw per CGA pixel		*/
#define TEXTURE_MARGIN		3		/* Margin between each drawn texture on screen	*/

/* Draw one CGA pixel on screen */
void PutPixel(uint16_t x, uint16_t y, uint8_t c, SDL_Renderer* renderer)
{
	/* Switch current pixel color according to CGA Mode 4 Pallete 1 High Intensity */
	switch (c)
	{
	case 0:
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		break;
	case 1:
		SDL_SetRenderDrawColor(renderer, 0x55, 0xFF, 0xFF, 0x00);
		break;
	case 2:
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x55, 0xFF, 0x00);
		break;
	case 3:
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
		break;
	}

	/* Output one CGA pixel to screen */
	for (uint16_t i = 0; i < CGA_PIXEL_SIZE; i++)
	{
		for (uint16_t j = 0; j < CGA_PIXEL_SIZE; j++)
		{
			SDL_RenderDrawPoint(renderer, (x * CGA_PIXEL_SIZE) + j, (y * CGA_PIXEL_SIZE) + i);
		}
	}
}

/* Draw a complete texture out of binary data */
void DrawTexture(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, SDL_Renderer* renderer)
{
	uint8_t* pui8Buffer = buffer;
	
	for (uint16_t ui16Y = 0; ui16Y < height; ui16Y++)
	{
		for (uint16_t ui16X = 0; ui16X < width; ui16X+=CGA_PIXELS_PER_BYTE)
		{
			for (uint16_t ui8P = 0; ui8P < (CGA_PIXELS_PER_BYTE * 2); ui8P += 2)
			{
				uint8_t ui8CurrentPixel = (((*pui8Buffer) >> ui8P) & 0x03);

				/* put pixel */
				/* each byte pixel is reversed order */
				PutPixel(
					x + ui16X + (4 - (ui8P / 2)), 
					y + ui16Y, 
					ui8CurrentPixel, 
					renderer);
			}
			pui8Buffer++;
		}
	}
}

/* Round an integer up */
int RoundUp4(int num)
{
	int remainder = num % 4;

	if (remainder == 0)
	{
		return num;
	}
	else
	{
		return num + 4 - remainder;
	}
}

/* Go over binary buffer and parse textures */
void BinaryParseAndDraw(uint8_t* buffer, uint16_t NumOfTextures, uint16_t *XPos, uint16_t *YPos, uint16_t *MaxHeight, SDL_Renderer* renderer)
{
	for (int i = 0; i < NumOfTextures; i++)
	{
		/* First two bytes of textures are its width and height */
		uint8_t x = buffer[0];
		uint8_t y = buffer[1];
		buffer += 2;

		/* Handle prety drawing to display */
		if (((*XPos) + x) > WINDOW_WIDTH)
		{
			(*XPos) = 0;
			(*YPos) += ((*MaxHeight) + TEXTURE_MARGIN);
			(*MaxHeight) = 0;
		}

		DrawTexture(buffer, (*XPos), (*YPos), RoundUp4(x), y, renderer);

		/* Handle prety drawing to display */
		if (y > (*MaxHeight))
		{
			(*MaxHeight) = y;
		}

		buffer += (RoundUp4(x) / 4) * y;
		(*XPos) += RoundUp4(x) + TEXTURE_MARGIN;
	}
}

int main(int argc, char* argv[])
{
	FILE* fp = NULL;
	uint32_t ui32FileSize = 0u;
	uint8_t* pui8Program = NULL;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	SDL_Event event;
	SDL_Surface* screenshot = NULL;
	uint32_t ui32VideoIndex = 0u;
	uint16_t ui16XPos = 0u;
	uint16_t ui16YPos = 0u;
	uint16_t ui16MaxHeight = 0u;

	/* input file should be Dig Dug binary file */
	if (argc < 2)
	{
		printf("Missing input file\n");
		return -1;
	}

	/* read file from command line */
	fp = fopen(argv[1], "rb");
	if (NULL == fp)
	{
		printf("Error openning file %s\n", argv[1]);
		return -1;
	}

	/* get file size */
	fseek(fp, 0L, SEEK_END);
	ui32FileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	/* read video buffer data */
	pui8Program = (uint8_t*)malloc(ui32FileSize);
	if (fread(pui8Program, sizeof(uint8_t), ui32FileSize, fp) != ui32FileSize)
	{
		printf("Error reading file %s\n", argv[1]);
		free(pui8Program);
		fclose(fp);
		return -1;
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer((WINDOW_WIDTH * CGA_PIXEL_SIZE), (WINDOW_HEIGHT * CGA_PIXEL_SIZE), 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 30, 30, 30, 0);
	SDL_RenderClear(renderer);

	ui16XPos = 0u;
	ui16YPos = 0u;
	ui16MaxHeight = 0u;
	
	/*
	[0x4270-0x57B8] -> 103 textures
	[0x579E-0x59AD] -> 32 textures
	[0x5A2E-0x5E1D] -> 63 textures
	*/
	BinaryParseAndDraw(&(pui8Program[0x4270]), 103, &ui16XPos, &ui16YPos, &ui16MaxHeight, renderer);
	BinaryParseAndDraw(&(pui8Program[0x579E]), 32, &ui16XPos, &ui16YPos, &ui16MaxHeight, renderer);
	BinaryParseAndDraw(&(pui8Program[0x5A2E]), 63, &ui16XPos, &ui16YPos, &ui16MaxHeight, renderer);

	SDL_RenderPresent(renderer);

	screenshot = SDL_CreateRGBSurface(
		0,
		(WINDOW_WIDTH * CGA_PIXEL_SIZE),
		(WINDOW_HEIGHT * CGA_PIXEL_SIZE),
		32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0x00000000);

	SDL_RenderReadPixels(renderer, NULL, SDL_GetWindowPixelFormat(window), screenshot->pixels, screenshot->pitch);

	/* Create the bmp screenshot file */
	SDL_SaveBMP(screenshot, "Screenshot.bmp");

	/* Destroy the screenshot surface */
	SDL_FreeSurface(screenshot);

	while (1)
	{
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
			break;
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	/* free memory */
	free(pui8Program);

	/* close file */
	fclose(fp);

	return 0;
}
