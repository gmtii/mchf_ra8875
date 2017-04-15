/*
 * ui_ra8875_fonts.h
 *
 *  Created on: 15/11/2016
 *      Author: gmtii
 */

#ifndef RA8875_UI_RA8875_FONTS_H_
#define RA8875_UI_RA8875_FONTS_H_


typedef struct tFont
{
  const unsigned short *table;
  unsigned short Width;
  unsigned short Height;
} sFONT;

typedef struct tFont32
{
  const unsigned int *table;
  unsigned short Width;
  unsigned short Height;
} sFONT32;

#endif


