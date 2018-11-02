#pragma once

#include <vector>


namespace gv {


/*!
 * \brief Tile header.
 *
 * It's used as tile identifier as no two different tiles have
 * same combination of values {z, x, y}.
 */
struct TileHead
{
    int z;  //!< Map zoom level.
    int x;  //!< Map tile coordinate x.
    int y;  //!< Map tile coordinate y.

    //! Constructor requires values for all data fields z, x, y.
    TileHead( int az, int ax, int ay ) : z( az ), x( ax ), y( ay ) {}

    //! Default copy constructor.
    TileHead( const TileHead& rhs ) = default;
    
    //! Default move constructor.
    TileHead( TileHead&& rhs ) = default;

    //! Standard copy assignment operator.
    TileHead& operator=( const TileHead& rhs )
    {
        z = rhs.z;
        x = rhs.x;
        y = rhs.y;
        return *this;
    }

    //! Standard move assignment operator.
    TileHead& operator=( TileHead&& rhs )
    {
        z = std::move( rhs.z );
        x = std::move( rhs.x );
        y = std::move( rhs.y );
        return *this;
    }

    //! Equality operator.
    bool operator==( const TileHead& rhs ) const
    {
        return rhs.z == z && rhs.x == x && rhs.y == y;
    }
};


/*!
 * \brief Tile body.
 *
 * It makes sense to have tile body filled once the whole map texture
 * dimensions are known. Then a body can be assigned its position in
 * the texture: row, column and two corners (tx0, ty0), (tx1, ty1).
 * The same corners in geographic coordinates must be calculated
 * depending on the corresponding TileHead.
 */
struct TileBody
{
    double lon0;    //!< Longitude of left-bottom corner.
    double lat0;    //!< Latitude of left-bottom corner.
    double lon1;    //!< Longitude of right-top corner.
    double lat1;    //!< Latitude of right-top corner.
    float tx0;      //!< Texture x coordinate of left-bottom corner.
    float ty0;      //!< Texture y coordinate of left-bottom corner.
    float tx1;      //!< Texture x coordinate of right-top corner.
    float ty1;      //!< Texture y coordinate of right-top corner.
    int row;        //!< Row number in texture.
    int col;        //!< Column number in texture.

    //! Default constructor.
    TileBody() = default;

    //! Default copy constructor.
    TileBody( const TileBody& rhs ) = default;
 
    //! Default move constructor.
    TileBody( TileBody&& rhs ) = default;

    //! Standard copy assignment operator.
    TileBody& operator=( const TileBody& rhs )
    {
        lon0 = rhs.lon0;
        lat0 = rhs.lat0;
        lon1 = rhs.lon1;
        lat1 = rhs.lat1;
        tx0 = rhs.tx0;
        ty0 = rhs.ty0;
        tx1 = rhs.tx1;
        ty1 = rhs.ty1;
        row = rhs.row;
        col = rhs.col;
        return *this;
    }

    //! Standard move assignment operator.
    TileBody& operator=( TileBody&& rhs )
    {
        lon0 = std::move( rhs.lon0 );
        lat0 = std::move( rhs.lat0 );
        lon1 = std::move( rhs.lon1 );
        lat1 = std::move( rhs.lat1 );
        tx0 = std::move( rhs.tx0 );
        ty0 = std::move( rhs.ty0 );
        tx1 = std::move( rhs.tx1 );
        ty1 = std::move( rhs.ty1 );
        row = std::move( rhs.row );
        col = std::move( rhs.col );
        return *this;
    }
};


/*!
 * \brief Tile image.
 *
 * Consists of raw bytes that can be directly copied to OpenGL texture.
 */
struct TileData
{
    std::vector<unsigned char> data;        //!< Tile image data

    //! Copy constructor (may accept empty vector).
    TileData( const std::vector<unsigned char>& vec = {} ) : data( vec ) {}

    //! Move constructor.
    TileData( std::vector<unsigned char>&& vec ) : data( std::move( vec ) ) {}

    //! Default copy constructor.
    TileData( const TileData& rhs ) = default;
    
    //! Default move constructor.
    TileData( TileData&& rhs ) = default;

    //! Standard copy assignment operator.
    TileData& operator=( const TileData& rhs )
    {
        data = rhs.data;
        return *this;
    }

    //! Standard move assignment operator.
    TileData& operator=( TileData&& rhs )
    {
        data = std::move( rhs.data );
        return *this;
    }
};


/*!
 * \brief Tile fully describes map tile.
 */
struct Tile
{
    TileHead head;      //!< Tile header.
    TileBody body;      //!< Tile body.
    TileData data;      //!< Tile image.

    //! TileHead information must be provided.
    Tile( int az, int ax, int ay ) : head( az, ax, ay ) {}

    //! Construct by copying parts. Only TileHead information is required.
    Tile( const TileHead& ahead, const TileBody& abody = {}, const TileData& adata = {} ) : head( ahead ), body( abody ), data( adata ) {}

    //! Construct by moving parts. Only TileHead information is required.
    Tile( TileHead&& ahead, TileBody&& abody = {}, TileData&& adata = {} ) : head( std::move( ahead ) ), body( std::move( abody ) ), data( std::move( adata ) ) {}

    //! Default copy constructor.
    Tile( const Tile& rhs ) = default;
    
    //! Default move constructor.
    Tile( Tile&& rhs ) = default;

    //! Standard copy assignment operator.
    Tile& operator=( const Tile& rhs )
    {
        head = rhs.head;
        body = rhs.body;
        data = rhs.data;
    }

    //! Standard move assignment operator.
    Tile& operator=( Tile&& rhs )
    {
        head = std::move( rhs.head );
        body = std::move( rhs.body );
        data = std::move( rhs.data );
    }
};


/*!
 * \brief Identifiable map tile image.
 *
 * Consists of header to identify the tile and the image itself.
 */
struct TileImage
{
    TileHead head;      //!< Tile header.
    TileData data;      //!< Tile image.

    //! TileHead information must be provided.
    TileImage( int az, int ax, int ay ) : head( az, ax, ay ) {}

    //! Construct by copying parts. Only TileHead information is required.
    TileImage( const TileHead& ahead, const TileData& adata = {} ) : head( ahead ), data( adata ) {}
   
    //! Construct by moving parts. Only TileHead information is required.
    TileImage( TileHead&& ahead, TileData&& adata = {} ) : head( std::move( ahead ) ), data( std::move( adata ) ) {}

    //! Default copy constructor.
    TileImage( const TileImage& rhs ) = default;
   
    //! Default move constructor.
    TileImage( TileImage&& rhs ) = default;

    //! Standard copy assignment operator.
    TileImage& operator=( const TileImage& rhs )
    {
        head = rhs.head;
        data = rhs.data;
    }

    //! Standard move assignment operator.
    TileImage& operator=( TileImage&& rhs )
    {
        head = std::move( rhs.head );
        data = std::move( rhs.data );
    }
};


}
