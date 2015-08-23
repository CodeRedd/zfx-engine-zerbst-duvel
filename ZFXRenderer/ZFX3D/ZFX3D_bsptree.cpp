//File: ZFX3D_bsptree.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"
#include <memory.h>

ZFXBSPTree::ZFXBSPTree()
{
   m_NumPolys  = 0;
   m_pBack     = NULL;
   m_pFront    = NULL;
   m_pRoot     = NULL;
   m_pParent   = NULL;
   m_pPolys    = NULL;
}

ZFXBSPTree::~ZFXBSPTree()
{
   m_NumPolys = 0;

   if ( m_pPolys )
   {
      free( m_pPolys );
      m_pPolys = NULL;
   }

   if ( m_pFront )
   {
      delete m_pFront;
      m_pFront = NULL;
   }

   if ( m_pBack )
   {
      delete m_pBack;
      m_pBack = NULL;
   }
}

void ZFXBSPTree::BuildTree( const ZFXPolygon *pPolys, UINT Num )
{
   m_pRoot = this;
   m_pParent = NULL;

   if ( Num < 1 )
   {
      return;
   }

   //allocate memory
   int nSize = sizeof(ZFXPolygon)*Num;
   m_pPolys = (ZFXPolygon*) malloc(nSize);
   memset(m_pPolys, 0, nSize);
   m_NumPolys = Num;

   for ( UINT i = 0; i < Num; i++ )
   {
      m_pPolys[i].CopyOf(pPolys[i]);
   }

   //start recursion
   CreateChildren();
}

void ZFXBSPTree::CreateChildren()
{
   float       fDot=0.0f;
   ZFXPolygon  plyFront, plyBack;
   int         nFront = 0, nBack = 0, nClass = 0;

   CalcBoundingBox(m_pPolys, m_NumPolys);

   //if no splitter found, this is a leaf
   if ( !FindBestSplitter() )
   {
      ZFXBSPTree::m_sNum += m_NumPolys;
      return;
   }

   //create objects for the two children
   m_pFront = new ZFXBSPTree;
   m_pBack  = new ZFXBSPTree;
   m_pFront->SetRelationship(m_pRoot, this);
   m_pBack->SetRelationship(m_pRoot, this);

   //sort polygons into children
   for ( UINT i = 0; i < m_NumPolys; i++ )
   {
      nClass = m_Plane.Classify(m_pPolys[i]);

      if ( nClass == ZFXFRONT )
      {
         m_pFront->AddPolygon(m_pPolys[i]);
      }
      else if ( nClass == ZFXBACK )
      {
         m_pBack->AddPolygon( m_pPolys[i] );
      }
      else if ( nClass == ZFXCLIPPED )
      {
         //split polygon at the plane
         m_pPolys[i].Clip(m_Plane, &plyFront, &plyBack);

         m_pFront->AddPolygon(plyFront);
         m_pBack->AddPolygon(plyBack);
      }
      else if ( nClass == ZFXPLANAR )
      {
         fDot = m_Plane.m_vcN * m_pPolys[i].GetPlane().m_vcN;
         if ( fDot >= 0.0f )
         {
            m_pFront->AddPolygon( m_pPolys[i] );
         }
         else
         {
            m_pBack->AddPolygon( m_pPolys[i] );

         }
      }
   }

   //delete polygon list on inner leaves
   if ( m_pPolys )
   {
      free( m_pPolys );
      m_pPolys = NULL;
   }

   //recurse
   m_pFront->CreateChildren();
   m_pBack->CreateChildren();
}

void ZFXBSPTree::AddPolygon( const ZFXPolygon &Poly )
{

   m_pPolys = (ZFXPolygon*) realloc( m_pPolys, sizeof( ZFXPolygon )*( m_NumPolys + 1 ) );

   memset( &m_pPolys[m_NumPolys], 0, sizeof( ZFXPolygon ) );

   m_pPolys[m_NumPolys].CopyOf( Poly );
   m_NumPolys++;
}

void ZFXBSPTree::CalcBoundingBox( const ZFXPolygon *_pPolys_, unsigned int Num )
{
   ZFXVector   vcMax, vcMin, vcTemp;
   ZFXAABB     Aabb;

   // cast away const
   ZFXPolygon *pPolys = ( (ZFXPolygon*) _pPolys_ );

   if ( Num < 1 )
   {
      return;
   }

   // get arbitrary sub bounding box
   Aabb = pPolys[0].GetAabb();
   vcMax = vcMin = Aabb.vcCenter;

   for ( unsigned int i = 0; i<Num; i++ )
   {
      Aabb = pPolys[i].GetAabb();

      // get obb one side's extreme values
      vcTemp = Aabb.vcMax;

      if ( vcTemp.x > vcMax.x )
      {
         vcMax.x = vcTemp.x;
      }
      else if ( vcTemp.x < vcMin.x )
      {
         vcMin.x = vcTemp.x;
      }

      if ( vcTemp.y > vcMax.y )
      {
         vcMax.y = vcTemp.y;
      }
      else if ( vcTemp.y < vcMin.y )
      {
         vcMin.y = vcTemp.y;
      }

      if ( vcTemp.z > vcMax.z )
      {
         vcMax.z = vcTemp.z;
      }
      else if ( vcTemp.z < vcMin.z )
      {
         vcMin.z = vcTemp.z;
      }

      // get obb other side's extreme values
      vcTemp = Aabb.vcMin;

      if ( vcTemp.x > vcMax.x )
      {
         vcMax.x = vcTemp.x;
      }
      else if ( vcTemp.x < vcMin.x )
      {
         vcMin.x = vcTemp.x;
      }

      if ( vcTemp.y > vcMax.y )
      {
         vcMax.y = vcTemp.y;
      }
      else if ( vcTemp.y < vcMin.y )
      {
         vcMin.y = vcTemp.y;
      }

      if ( vcTemp.z > vcMax.z )
      {
         vcMax.z = vcTemp.z;
      }
      else if ( vcTemp.z < vcMin.z )
      {
         vcMin.z = vcTemp.z;
      }
   }

   // now calculate maximum extension
   float fMax = vcMax.x - vcMin.x;
   if ( fMax < ( vcMax.y - vcMin.y ) )
   {
      fMax = vcMax.y - vcMin.y;
   }
   if ( fMax < ( vcMax.z - vcMin.z ) )
   {
      fMax = vcMax.z - vcMin.z;
   }

   // make box cubic
   m_AABB.vcCenter = ( vcMax + vcMin ) / 2.0f;
   m_AABB.vcMax = m_AABB.vcCenter + ( fMax / 2.0f );
   m_AABB.vcMin = m_AABB.vcCenter - ( fMax / 2.0f );
}

bool ZFXBSPTree::FindBestSplitter()
{
   ZFXPolygon *pBestSplitter = NULL, *pSplitter = NULL;
   ZFXPlane    Plane;
   LONG        lFront = 0,    // how many polygons in front, back, planar, or spanning with respect
               lBack = 0,     // to each possible splitter
               lPlanar = 0,
               lSplits = 0;
   int         nClass;
   LONG        lScore, lBestScore = 100000;
   bool        bFound = false;

   for ( UINT i = 0; i < m_NumPolys; i++ )
   {
      pSplitter = &m_pPolys[i];
      Plane = pSplitter->GetPlane();

      //reset counters
      lFront = lBack = lPlanar = lSplits = 0;

      //have we used as a splitter already?
      if ( pSplitter->GetFlag() == 1 )
      {
         continue;
      }

      //test all polygons as splitters
      for ( UINT j = 0; j < m_NumPolys; j++ )
      {
         if ( i == j )
         {
            continue;
         }

         nClass = Plane.Classify( m_pPolys[j] );
         if ( nClass == ZFXFRONT )
         {
            lFront++;
         }
         else if ( nClass == ZFXBACK )
         {
            lBack++;
         }
         else if ( nClass == ZFXPLANAR )
         {
            lPlanar++;
         }
         else
         {
            lSplits++;
         }
      }

      //calculate score for this poly
      lScore = abs( lFront - lBack ) + ( lSplits * 3 );
      if ( lScore < lBestScore )
      {
         if ( ( lFront > 0 && lBack > 0 ) || lSplits > 0 )
         {
            lBestScore = lScore;
            pBestSplitter = pSplitter;
            bFound = true;
         }
      }
   }

   //no splitter found? then we are convex
   if ( !bFound )
   {
      return false;
   }

   //mark the polygon and save the splitter plane
   pBestSplitter->SetFlag( 1 );
   m_Plane = pBestSplitter->GetPlane();
   return true;
}

void ZFXBSPTree::TraverseFtB( ZFXPolyList *pList, ZFXVector vcPos, const ZFXPlane *Frustum )
{
   //frustum culling for this node
   if ( m_AABB.Cull( Frustum, 6 ) == ZFXCULLED )
   {
      return;
   }

   //add polygons from leaves
   if ( IsLeaf() )
   {
      for ( UINT i = 0; i < m_NumPolys; i++ )
      {
         pList->AddPolygon( m_pPolys[i] );
      }
   }
   else
   {
      int nClass = m_Plane.Classify( vcPos );

      if ( nClass = ZFXBACK )
      {
         m_pBack->TraverseFtB( pList, vcPos, Frustum );
         m_pFront->TraverseFtB( pList, vcPos, Frustum );
      }
      else
      {
         m_pFront->TraverseFtB( pList, vcPos, Frustum );
         m_pBack->TraverseFtB( pList, vcPos, Frustum );
      }
   }
}

void ZFXBSPTree::TraverseBtF( ZFXPolyList *pList, ZFXVector vcPos, const ZFXPlane *Frustum )
{
   //frustum culling for this node
   if ( m_AABB.Cull( Frustum, 6 ) == ZFXCULLED )
   {
      return;
   }

   //add polygons from leaves
   if ( IsLeaf() )
   {
      for ( UINT i = 0; i < m_NumPolys; i++ )
      {
         pList->AddPolygon( m_pPolys[i] );
      }
   }
   else
   {
      int nClass = m_Plane.Classify( vcPos );

      if ( nClass = ZFXBACK )
      {
         m_pFront->TraverseBtF( pList, vcPos, Frustum );
         m_pBack->TraverseBtF( pList, vcPos, Frustum );
      }
      else
      {
         m_pBack->TraverseBtF( pList, vcPos, Frustum );
         m_pFront->TraverseBtF( pList, vcPos, Frustum );
      }
   }
}

bool ZFXBSPTree::TestCollision( const ZFXRay &Ray, float fL, float *pfD, ZFXVector *pvcN )
{
   ZFXRay   rayFront, rayBack;
   int      nFront = 0;

   //Test polygons if we are at a leaf
   if ( IsLeaf() )
   {
      for ( UINT i = 0; i < m_NumPolys; i++ )
      {
         //collision?
         if ( m_pPolys[i].Intersects( Ray, false, 0, fL ) )
         {
            if ( pvcN )
            {
               *pvcN = m_pPolys[i].GetPlane().m_vcN;
            }
            return true;
         }
      }
      return false;
   }
   
   //else recurse if we are at an inner node
   int nClass = m_Plane.Classify( Ray.m_vcOrig );

   //ray intersects split plane?
   if ( m_Plane.Clip( &Ray, fL, &rayFront, &rayBack ) )
   {
      //search in front to back order
      if ( nClass == ZFXBACK )
      {
         return m_pBack->TestCollision( rayBack, fL, pfD, pvcN ) || m_pFront->TestCollision( rayFront, fL, pfD, pvcN );
      }
      else
      {
         return m_pFront->TestCollision( rayFront, fL, pfD, pvcN ) || m_pBack->TestCollision( rayBack, fL, pfD, pvcN );

      }
   }
   else
   {
      if ( nClass == ZFXBACK )
      {
         return m_pBack->TestCollision( Ray, fL, pfD, pvcN );
      }
      else
      {
         return m_pFront->TestCollision( Ray, fL, pfD, pvcN );

      }
   }
}

bool ZFXBSPTree::LineOfSight( const ZFXVector &vcA, const ZFXVector &vcB )
{
   ZFXRay Ray;

   //create a ray from A to B
   ZFXVector vcDir = vcB - vcA;
   vcDir.Normalize();
   Ray.Set( vcA, vcDir );

   //test for collisions
   return !TestCollision( Ray, vcDir.GetLength(), 0, 0 );
}