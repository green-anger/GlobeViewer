#pragma once

#include <vector>


namespace gv {


struct TileHead
{
    int z;          //!< map zoom level
    int x;          //!< map tile coordinate x
    int y;          //!< map tile coordinate y

    TileHead( int az, int ax, int ay ) : z( az ), x( ax ), y( ay ) {}
    TileHead( const TileHead& rhs ) = default;
    TileHead( TileHead&& rhs ) = default;

    TileHead& operator=( const TileHead& rhs )
    {
        z = rhs.z;
        x = rhs.x;
        y = rhs.y;
    }

    TileHead& operator=( TileHead&& rhs )
    {
        z = std::move( rhs.z );
        x = std::move( rhs.x );
        y = std::move( rhs.y );
    }

    bool operator==( const TileHead& rhs ) const
    {
        return rhs.z == z && rhs.x == x && rhs.y == y;
    }
};


struct TileBody
{
    double lon0;    //!< longitude of left-bottom corner
    double lat0;    //!< latitude of left-bottom corner
    double lon1;    //!< longitude of right-top corner
    double lat1;    //!< latitude of right-top corner
    float tx0;      //!< texture x coordinate of left-bottom corner
    float ty0;      //!< texture y coordinate of left-bottom corner
    float tx1;      //!< texture x coordinate of right-top corner
    float ty1;      //!< texture y coordinate of right-top corner
    bool visible;   //!< currently visible
    std::vector<unsigned char> data;    //!< tile image data

    TileBody() = default;
    TileBody( const TileBody& rhs ) = default;
    TileBody( TileBody&& rhs ) = default;

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
        visible = rhs.visible;
        data = rhs.data;
        return *this;
    }

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
        visible = std::move( rhs.visible );
        data = std::move( rhs.data );
        return *this;
    }
};


struct Tile
{
    TileHead head;
    TileBody body;

    Tile( int az, int ax, int ay ) : head( az, ax, ay ) {}
    Tile( const TileHead& ahead, const TileBody& abody = {} ) : head( ahead ), body( abody ) {}
    Tile( TileHead&& ahead, TileBody&& abody = {} ) : head( std::move( ahead ) ), body( std::move( abody ) ) {}
    Tile( const Tile& rhs ) = default;
    Tile( Tile&& rhs ) = default;

    Tile& operator=( const Tile& rhs )
    {
        head = rhs.head;
        body = rhs.body;
    }

    Tile& operator=( Tile&& rhs )
    {
        head = std::move( rhs.head );
        body = std::move( rhs.body );
    }
};


}
