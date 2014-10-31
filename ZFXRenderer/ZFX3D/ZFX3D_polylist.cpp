//File: ZFX3D_polylist.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

ZFXPolyList::ZFXPolyList()
{
   m_pPolys = NULL;
   m_Num = 0;
   m_Max = 0;
}

ZFXPolyList::~ZFXPolyList()
{
   Reset();
}

bool ZFXPolyList::AddPolygon( const ZFXPolygon &Poly )
{
   if ( !CheckMem() )
   {
      return false;
   }

   m_pPolys[m_Num].CopyOf(Poly);
   m_Num++;
   return true;
}

void ZFXPolyList::Reset()
{
   if ( m_pPolys )
   {
      free(m_pPolys);
      m_pPolys = NULL;
   }
   m_Num = 0;
   m_Max = 0;
}

bool ZFXPolyList::CheckMem()
{
   if ( m_Num < m_Max )
   {
      return true;
   }

   m_Max += 100;
   int nSize = sizeof(ZFXPolygon) * m_Max;
   m_pPolys = (ZFXPolygon*) realloc(m_pPolys, nSize);
   memset(&m_pPolys[m_Num], 0, sizeof(ZFXPolygon)*100);
   return m_pPolys != NULL;
}