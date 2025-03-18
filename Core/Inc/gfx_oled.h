#ifndef INC_GFX_OLED_H_
#define INC_GFX_OLED_H_

#include "main.h"
#include "ssd1306_oled.h"

#define GFX_DrawPixel(x, y, color) SSD1306_DrawPixel(x, y, color)

#define WIDTH SSD1306_LCDWIDTH
#define WIDTH SSD1306_LCDHEIGHT

#define PIXEL_BLACK BLACK
#define PIXEL_WHITE WHITE
#define PIXEL_INVERSE INVERSE

/**************************************************************************/
/*!
   @brief    Draw a line
    @param    x0  Start point x coordinate
    @param    y0  Start point y coordinate
    @param    x1  End point x coordinate
    @param    y1  End point y coordinate
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void GFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
/**************************************************************************/
/*!
   @brief    Draw a circle outline
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/

void GFX_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
/**************************************************************************/
/*!
   @brief    Draw a circle with filled color
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/

void GFX_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
/**************************************************************************/
/*!
   @brief   Draw a rectangle with no fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/

void GFX_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
/**************************************************************************/
/*!
   @brief   Draw a rounded rectangle with no fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    r   Radius of corner rounding
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/

void GFX_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void GFX_WriteFillRect(int x, int y, uint16_t w, uint16_t h, uint8_t color);
/**************************************************************************/
/*!
   @brief   Draw a rounded rectangle with fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    r   Radius of corner rounding
    @param    color 16-bit 5-6-5 Color to draw/fill with
*/
/**************************************************************************/

void GFX_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
/**************************************************************************/
/*!
   @brief   Draw a triangle with no fill color
    @param    x0  Vertex #0 x coordinate
    @param    y0  Vertex #0 y coordinate
    @param    x1  Vertex #1 x coordinate
    @param    y1  Vertex #1 y coordinate
    @param    x2  Vertex #2 x coordinate
    @param    y2  Vertex #2 y coordinate
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/

void GFX_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
/**************************************************************************/
/*!
@brief     Draw a triangle with color-fill
@param    x0  Vertex #0 x coordinate
@param    y0  Vertex #0 y coordinate
@param    x1  Vertex #1 x coordinate
@param    y1  Vertex #1 y coordinate
@param    x2  Vertex #2 x coordinate
@param    y2  Vertex #2 y coordinate
@param    color 16-bit 5-6-5 Color to fill/draw with
*/
/**************************************************************************/

void GFX_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void GFX_DrawString(int x, int y, char* str, uint8_t color, uint8_t background);
void GFX_DrawChar(int x, int y, char chr, uint8_t color, uint8_t background);
void GFX_SetFontSize(uint8_t size_t);
void GFX_SetFont(const uint8_t* font_t);
void GFX_Image(int x, int y, const uint8_t *img, uint8_t w, uint8_t h, uint8_t color);
#endif