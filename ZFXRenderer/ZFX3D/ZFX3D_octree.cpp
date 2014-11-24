//File: ZFX3D_octree.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

ZFXOctree::ZFXOctree()
{
   m_NumPolys  = 0;
   m_Pos       = -1;
   m_pPolys    = NULL;
   m_pRoot     = NULL;
   m_pParent   = NULL;

   for ( int i = 0; i < 8; i++ )
   {
      m_pChild[i] = NULL;
   }

   memset( &m_AABB, 0, sizeof( ZFXAABB ) );
}

ZFXOctree::~ZFXOctree()
{
   m_NumPolys = 0;

   if ( m_pPolys )
   {
      delete m_pPolys;
      m_pPolys = NULL;
   }

   for ( int i = 0; i < 8; i++ )
   {
      if ( m_pChild[i] )
      {
         delete m_pChild[i];
         m_pChild[i] = NULL;
      }
   }
}

void ZFXOctree::InitChildObject( int ChildID, ZFXOctree *pParent )
{
   ZFXAABB aabb;

   float xmin = m_AABB.vcMin.x, xcen = m_AABB.vcCenter.x, xmax = m_AABB.vcMax.x;
   float ymin = m_AABB.vcMin.y, ycen = m_AABB.vcCenter.y, ymax = m_AABB.vcMax.y;
   float zmin = m_AABB.vcMin.z, zcen = m_AABB.vcCenter.z, zmax = m_AABB.vcMax.z;

   switch ( ChildID )
   {
      case UP_NW:
         aabb.vcMax = ZFXVector( xcen, ymax, zmax );
         aabb.vcMin = ZFXVector( xmin, ycen, zcen );
         break;
      case UP_NE:
         aabb.vcMax = m_AABB.vcMax;
         aabb.vcMin = m_AABB.vcCenter;
         break;      
      case UP_SW:
         aabb.vcMax = ZFXVector( xcen, ymax, zcen );
         aabb.vcMin = ZFXVector( xmin, ycen, zmin );
         break;      
      case UP_SE:
         aabb.vcMax = ZFXVector( xmax, ymax, zcen );
         aabb.vcMin = ZFXVector( xcen, ycen, zmin );
         break;
      case LW_NW:
         aabb.vcMax = ZFXVector( xcen, ycen, zmax );
         aabb.vcMin = ZFXVector( xmin, ymin, zcen );
         break;
      case LW_NE:
         aabb.vcMax = ZFXVector( xmax, ycen, zmax );
         aabb.vcMin = ZFXVector( xcen, ymin, zcen );
         break;
      case LW_SW:
         aabb.vcMax = m_AABB.vcCenter;
         aabb.vcMin = m_AABB.vcMin;
         break;
      case LW_SE:
         aabb.vcMax = ZFXVector( xmax, ycen, zcen );
         aabb.vcMin = ZFXVector( xcen, ymin, zmin );
         break;
      default: break;
   }

   aabb.vcCenter = ( aabb.vcMax + aabb.vcMin ) / 2.0f;

   m_pChild[ChildID] = new ZFXOctree();
   m_pChild[ChildID]->SetBoundingBox( aabb );
   m_pChild[ChildID]->m_Pos = ChildID;
   m_pChild[ChildID]->m_pParent = pParent;
}

void ZFXOctree::BuildTree( const ZFXPolygon *pPolys, UINT Num )
{
   m_pRoot = this;

   if ( Num < 1 )
   {
      return;
   }

   //calculate AABB for the root node
   CalcBoundingBox( pPolys, Num );

   m_pPolys = new ZFXPolygon[Num];
   m_NumPolys = Num;

   for ( UINT i = 0; i < Num; i++ )
   {
      m_pPolys[i].CopyOf( pPolys[i] );
   }

   //calculate the children
   CreateChildren( this );

   //data should stay in leaves, so remove redundancy here
   if ( m_pPolys )
   {
      delete[] m_pPolys;
      m_pPolys = NULL;
   }
}

void ZFXOctree::CreateChildren( ZFXOctree *pRoot )
{
   //save address of the root node
   m_pRoot = pRoot;

   //decide whether to continue?
   if ( pRoot == this || m_NumPolys > POLYS_PER_LEAF )
   {
      //initialize children
      for ( int i = 0; i < 8; i++ )
      {
         InitChildObject( i, this );

         //build polylist for child
         m_pChild[i]->ChopListToMe( m_pPolys, m_NumPolys );
         m_pChild[i]->CreateChildren( pRoot );
      }

      //data should stay in leaves, so remove redundancy here
      if ( m_pPolys )
      {
         delete[] m_pPolys;
         m_pPolys = NULL;
      }
   }
   //no? this is a leaf
   else
   {
      return;
   }
}

void ZFXOctree::ChopListToMe( ZFXPolygon *pList, UINT Num )
{
   ZFXPolygon ChoppedPoly;
   int nClass = 0;

   if ( Num < 1 )
   {
      return;
   }

   //better safe than sorry to clear the list...
   if ( m_pPolys )
   {
      delete[] m_pPolys;
      m_pPolys = NULL;
   }
   m_NumPolys = 0;

   ZFXPolygon *TempMem = new ZFXPolygon[Num];

   //loop through polylist
   for( UINT i = 0; i < Num; i++ )
   {
      if ( pList[i].GetFlag() == 1 )
      {
         continue;
      }

      ChoppedPoly.CopyOf( pList[i] );

      nClass = ChoppedPoly.Cull( m_AABB );

      //polygon outside AABB
      if ( nClass == ZFXCULLED )
      {
         continue;
      }
      //polygon in or intersecting AABB
      else
      {
         //flag if contained
         if ( nClass == ZFXCLIPPED )
         {
            pList[i].SetFlag(1);
         }
         //or else clip the polygon
         else
         {
            ChoppedPoly.Clip( m_AABB );
         }

         //add to temp list
         TempMem[m_NumPolys].CopyOf( ChoppedPoly );
         m_NumPolys++;
      }
   }

   //copy temporary list to this node
   m_pPolys = new ZFXPolygon[m_NumPolys];

   for ( UINT j = 0; j < m_NumPolys; j++ ) //Norw: this copy process is a little slow and inefficient -- perhaps the poly class should use static arrays for its data?
   {
      m_pPolys[j].CopyOf( TempMem[j] );
   }

   delete[] TempMem;
}

bool ZFXOctree::TestCollision( const ZFXAABB &aabb, ZFXPlane *pP )
{
   //test collision in this node
   if ( this != m_pRoot )
   {
      if ( !m_AABB.Intersects( aabb ) )
      {
         return false;
      }
   }

   //no geometry, just a node
   if ( !IsLeaf() )
   {
      for ( int i = 0; i < 8; i++ )
      {
         //check children for collision
         if ( m_pChild[i]->TestCollision( aabb, pP ) )
         {
            return true;
         }
      }
      
      //no children collided?
      return false;
   }
   //this is a leaf with geometry
   else
   {
      if ( !m_pPolys )
      {
         return false;
      }

      //test all polygons for collision
      for ( UINT i = 0; i < m_NumPolys; i++ )
      {
         if ( m_pPolys[i].GetAabb().Intersects( aabb ) )
         {
            if ( pP )
            {
               *pP = m_pPolys[i].GetPlane();
            }
            return true;
         }
      }
      //no collision in this leaf
      return false;
   }
}

bool ZFXOctree::TestCollision( const ZFXRay &Ray, float fL, float *pfD )
{
   bool bInCollision = false;
   float _fD = 0.0f;

   //collision in this node?
   if ( this != m_pRoot )
   {
      if ( !m_AABB.Intersects( Ray, pfD, fL ) && !m_AABB.Contains( Ray, fL ) )
      {
         return false;
      }
   }

   //not a leaf?
   if ( !IsLeaf() )
   {
      for ( int i = 0; i < 8; i++ )
      {
         //check the children
         if ( m_pChild[i]->TestCollision( Ray, fL, pfD ) )
         {
            return true;
         }
      }

      //none of the children collided
      return false;
   }
   //this is a leaf with geometry
   else
   {
      if ( !m_pPolys )
      {
         return false;
      }

      //test all polys for collision
      for ( UINT i = 0; i < m_NumPolys; i++ )
      {
         if ( m_pPolys[i].Intersects( Ray, false, &_fD, fL ) )
         {
            bInCollision = true;
            if ( !pfD )
            {
               return true;
            }

            //get the closest collision point
            if ( *pfD <= 0.0f || _fD < *pfD )
            {
               *pfD = _fD;
            }
         }
      }
      return bInCollision;
   }
   return false;
}

bool ZFXOctree::IntersectsDownwardRay( const ZFXVector &vcOrig, float f )
{
   //ray origin below this node
   if ( vcOrig.y < m_AABB.vcMin.y )
   {
      return false;
   }

   //check x-axis outside this node
   if ( vcOrig.x < m_AABB.vcMin.x )
   {
      return false;
   }
   if ( vcOrig.x > m_AABB.vcMax.x )
   {
      return false;
   }

   //check z-axis outside this node
   if ( vcOrig.z < m_AABB.vcMin.z )
   {
      return false;
   }
   if ( vcOrig.z > m_AABB.vcMax.z )
   {
      return false;
   }

   //is minimal possible distance to this node already greater than current intersection found in f?
   if ( f < fabs( m_AABB.vcMax.y - vcOrig.y ) )
   {
      return false;
   }

   return true;
}

bool ZFXOctree::GetFloor(const ZFXVector &vcPos, float *pF, ZFXPlane *pPlane)
{
   float fAABBDist = 0, fHitDist = 0;
   bool  bHit = false;
   ZFXAABB aabb;
   ZFXRay Ray;

   //if this is the root
   if ( this == m_pRoot )
   {
      *pF = 99999.0f;
   }

   //no geometry if not a leaf
   if ( !IsLeaf() )
   {
      for ( int i = 0; i < 8; i++ )
      {
         //is ray intersecting any child at all?
         if ( m_pChild[i]->IntersectsDownwardRay( vcPos, *pF ) )
         {
            //closer than current one?
            if ( m_pChild[i]->GetFloor( vcPos, pF, pPlane ) )
            {
               bHit = true;
            }
         }
      }
      return bHit;
   }
   //leaf with geometry
   else
   {
      if ( !m_pPolys )
      {
         return false;
      }

      Ray.Set( vcPos, ZFXVector( 0.0f, -1.0f, 0.0f ) );

      for ( UINT i = 0; i < m_NumPolys; i++ )
      {
         aabb = m_pPolys[i].GetAabb();

         //quick-test ray against polygon's bounding box
         if ( Ray.m_vcOrig.x < aabb.vcMin.x || Ray.m_vcOrig.x > aabb.vcMax.x ||
              Ray.m_vcOrig.z < aabb.vcMin.z || Ray.m_vcOrig.z > aabb.vcMax.z ||
              Ray.m_vcOrig.y < aabb.vcMin.y )
         {
            continue;
         }

         //full collision test
         if ( m_pPolys[i].Intersects( Ray, true, &fHitDist, *pF ) )
         {
            *pF = fHitDist;
            bHit = true;
         }
      }
      return bHit;
   }
}

void ZFXOctree::Traverse( ZFXPolyList *pList, ZFXPolyList *pAABBList, const ZFXPlane *pFrustum )
{
   if ( m_AABB.Cull( pFrustum, 6 ) == ZFXCULLED )
   {
      return;
   }

   if ( IsLeaf() )
   {
      if ( pList )
      {
         for ( unsigned int i = 0; i < m_NumPolys; i++ )
         {
            pList->AddPolygon( m_pPolys[i] );
         }
      }

      if ( pAABBList )
      {
         GetAABBAsPolygons( pAABBList );
      }
   }
   else
   {
      m_pChild[0]->Traverse( pList, pAABBList, pFrustum );
      m_pChild[1]->Traverse( pList, pAABBList, pFrustum );
      m_pChild[2]->Traverse( pList, pAABBList, pFrustum );
      m_pChild[3]->Traverse( pList, pAABBList, pFrustum );
      m_pChild[4]->Traverse( pList, pAABBList, pFrustum );
      m_pChild[5]->Traverse( pList, pAABBList, pFrustum );
      m_pChild[6]->Traverse( pList, pAABBList, pFrustum );
      m_pChild[7]->Traverse( pList, pAABBList, pFrustum );
   }
}

//Warning: This function is SLOW! Only use for visualizing the octree - don't use at runtime!
void ZFXOctree::GetAABBAsPolygons( ZFXPolyList *pList )
{
   ZFXPolygon Poly;
   ZFXVector  vcPoints[24];
   unsigned int nIndic[6] = { 0, 1, 2, 2, 3, 0 };

   float fW = m_AABB.vcMax.x - m_AABB.vcMin.x;
   float fH = m_AABB.vcMax.y - m_AABB.vcMin.y;
   float fD = m_AABB.vcMax.z - m_AABB.vcMin.z;

   //top rectangle
   vcPoints[0].Set( m_AABB.vcCenter.x - ( fW / 2.0f ),
                    m_AABB.vcCenter.y + ( fH / 2.0f ),
                    m_AABB.vcCenter.z - ( fD / 2.0f ) );
   vcPoints[1].Set( m_AABB.vcCenter.x - ( fW / 2.0f ),
                    m_AABB.vcCenter.y + ( fH / 2.0f ),
                    m_AABB.vcCenter.z + ( fD / 2.0f ) );
   vcPoints[2].Set( m_AABB.vcCenter.x + ( fW / 2.0f ),
                    m_AABB.vcCenter.y + ( fH / 2.0f ),
                    m_AABB.vcCenter.z + ( fD / 2.0f ) );
   vcPoints[3].Set( m_AABB.vcCenter.x + ( fW / 2.0f ),
                    m_AABB.vcCenter.y + ( fH / 2.0f ),
                    m_AABB.vcCenter.z - ( fD / 2.0f ) );
   Poly.Set( &vcPoints[0], 4, nIndic, 6 );
   pList->AddPolygon( Poly );

   //right rectangle
   vcPoints[4] = vcPoints[3];
   vcPoints[5] = vcPoints[2];
   vcPoints[6].Set( m_AABB.vcCenter.x + ( fW / 2.0f ),
                    m_AABB.vcCenter.y - ( fH / 2.0f ),
                    m_AABB.vcCenter.z + ( fD / 2.0f ) );
   vcPoints[7].Set( m_AABB.vcCenter.x + ( fW / 2.0f ),
                    m_AABB.vcCenter.y - ( fH / 2.0f ),
                    m_AABB.vcCenter.z - ( fD / 2.0f ) );
   Poly.Set( &vcPoints[4], 4, nIndic, 6 );
   pList->AddPolygon( Poly );

   //left rectangle
   vcPoints[8] = vcPoints[0];
   vcPoints[9] = vcPoints[1];
   vcPoints[10].Set( m_AABB.vcCenter.x - ( fW / 2.0f ),
                    m_AABB.vcCenter.y - ( fH / 2.0f ),
                    m_AABB.vcCenter.z + ( fD / 2.0f ) );
   vcPoints[11].Set( m_AABB.vcCenter.x - ( fW / 2.0f ),
                    m_AABB.vcCenter.y - ( fH / 2.0f ),
                    m_AABB.vcCenter.z - ( fD / 2.0f ) );
   Poly.Set( &vcPoints[8], 4, nIndic, 6 );
   pList->AddPolygon( Poly );

   //back rectangle
   vcPoints[12] = vcPoints[2];
   vcPoints[13] = vcPoints[1];
   vcPoints[14] = vcPoints[10];
   vcPoints[15] = vcPoints[6];

   Poly.Set( &vcPoints[12], 4, nIndic, 6 );
   pList->AddPolygon( Poly );

   //front rectangle
   vcPoints[16] = vcPoints[0];
   vcPoints[17] = vcPoints[3];
   vcPoints[18] = vcPoints[7];
   vcPoints[19] = vcPoints[11];

   Poly.Set( &vcPoints[16], 4, nIndic, 6 );
   pList->AddPolygon( Poly );

   //bottom rectangle
   vcPoints[20] = vcPoints[7];
   vcPoints[21] = vcPoints[6];
   vcPoints[22] = vcPoints[10];
   vcPoints[23] = vcPoints[11];

   Poly.Set( &vcPoints[20], 4, nIndic, 6 );
   pList->AddPolygon( Poly );
}