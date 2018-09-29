#include <vector>

//#define GLM_FORCE_RADIANS
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include <DataKeeper.h>


namespace gv
{


DataKeeper::DataKeeper()
{
    const GLfloat side = 1.0f;
    const std::vector<GLfloat> stVerts =
    {
        -side, -side,
        +side, -side,
         0.0f, +side
    };
    numST_ = 3;

    glGenVertexArrays( 1, &vaoST_ );
    glGenBuffers( 1, &vboST_ );
    glBindVertexArray( vaoST_ );
    glBindBuffer( GL_ARRAY_BUFFER, vboST_ );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * stVerts.size(), &stVerts[0], GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), ( void* )0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
}


DataKeeper::~DataKeeper()
{
}


std::tuple<GLuint, std::size_t> DataKeeper::simpleTriangle() const
{
    return std::make_tuple( vaoST_, numST_ );
}


}
