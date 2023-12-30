#include "stdafx.h"
#include "engine.h"
#include "renderer.h"

#include "openglstuffs.h"

#include <glm/gtx/component_wise.hpp>

AABB::AABB()
{
  setNull();
}

AABB::AABB(const glm::vec3& center, float radius)
{
  setNull();
  extend(center, radius);
}

AABB::AABB(const glm::vec3& p1, const glm::vec3& p2)
{
  setNull();
  extend(p1);
  extend(p2);
}

AABB::AABB(const AABB& aabb)
{
  setNull();
  extend(aabb);
}

AABB::~AABB()
{
}

void AABB::extend(float val)
{
  if (!isNull())
  {
    mMin -= glm::vec3(val);
    mMax += glm::vec3(val);
  }
}

void AABB::extend(const glm::vec3& p)
{
  if (!isNull())
  {
    mMin = glm::min(p, mMin);
    mMax = glm::max(p, mMax);
  }
  else
  {
    mMin = p;
    mMax = p;
  }
}

void AABB::extend(const glm::vec3& p, float radius)
{
  glm::vec3 r(radius);
  if (!isNull())
  {
    mMin = glm::min(p - r, mMin);
    mMax = glm::max(p + r, mMax);
  }
  else
  {
    mMin = p - r;
    mMax = p + r;
  }
}

void AABB::extend(const AABB& aabb)
{
  if (!aabb.isNull())
  {
    extend(aabb.mMin);
    extend(aabb.mMax);
  }
}

void AABB::extendDisk(const glm::vec3& c, const glm::vec3& n, float r)
{
  if (glm::length(n) < 1.e-12) { extend(c); return; }
  glm::vec3 norm = glm::normalize(n);
  float x = sqrt(1 - norm.x) * r;
  float y = sqrt(1 - norm.y) * r;
  float z = sqrt(1 - norm.z) * r;
  extend(c + glm::vec3(x,y,z));
  extend(c - glm::vec3(x,y,z));
}

glm::vec3 AABB::getDiagonal() const
{
  if (!isNull())
    return mMax - mMin;
  else
    return glm::vec3(0);
}

float AABB::getLongestEdge() const
{
  return glm::compMax(getDiagonal());
}

float AABB::getShortestEdge() const
{
  return glm::compMin(getDiagonal());
}

glm::vec3 AABB::getCenter() const
{
  if (!isNull())
  {
    glm::vec3 d = getDiagonal();
    return mMin + (d * float(0.5));
  }
  else
  {
    return glm::vec3(0.0);
  }
}

void AABB::transform(const glm::mat4& m) {
    if (!isNull()) {
        glm::vec4 mn(mMin, 1);
        glm::vec4 mx(mMax, 1);

        mn = m * mn;
        mx = m * mx;

        mMin = {mn.x, mn.y, mn.z};
        mMax = {mx.x, mx.y, mx.z};
    }
}

void AABB::translate(const glm::vec3& v)
{
  if (!isNull())
  {
    mMin += v;
    mMax += v;
  }
}

void AABB::scale(const glm::vec3& s, const glm::vec3& o)
{
  if (!isNull())
  {
    mMin -= o;
    mMax -= o;

    mMin *= s;
    mMax *= s;

    mMin += o;
    mMax += o;
  }
}

bool AABB::overlaps(const AABB& bb) const
{
  if (isNull() || bb.isNull())
    return false;

  if( bb.mMin.x > mMax.x || bb.mMax.x < mMin.x)
    return false;
  else if( bb.mMin.y > mMax.y || bb.mMax.y < mMin.y)
    return false;
  else if( bb.mMin.z > mMax.z || bb.mMax.z < mMin.z)
    return false;

  return true;
}

INTERSECTION_TYPE AABB::intersect(const AABB& b) const
{
  if (isNull() || b.isNull())
    return OUTSIDE;

  if ((mMax.x < b.mMin.x) || (mMin.x > b.mMax.x) ||
      (mMax.y < b.mMin.y) || (mMin.y > b.mMax.y) ||
      (mMax.z < b.mMin.z) || (mMin.z > b.mMax.z))
  {
    return OUTSIDE;
  }

  if ((mMin.x <= b.mMin.x) && (mMax.x >= b.mMax.x) &&
      (mMin.y <= b.mMin.y) && (mMax.y >= b.mMax.y) &&
      (mMin.z <= b.mMin.z) && (mMax.z >= b.mMax.z))
  {
    return INSIDE;
  }

  return INTERSECT;
}


bool AABB::isSimilarTo(const AABB& b, float diff) const
{
  if (isNull() || b.isNull()) return false;

  glm::vec3 acceptable_diff=( (getDiagonal()+b.getDiagonal()) / float(2.0))*diff;
  glm::vec3 min_diff(mMin-b.mMin);
  min_diff = glm::vec3(fabs(min_diff.x),fabs(min_diff.y),fabs(min_diff.z));
  if (min_diff.x > acceptable_diff.x) return false;
  if (min_diff.y > acceptable_diff.y) return false;
  if (min_diff.z > acceptable_diff.z) return false;
  glm::vec3 max_diff(mMax-b.mMax);
  max_diff = glm::vec3(fabs(max_diff.x),fabs(max_diff.y),fabs(max_diff.z));
  if (max_diff.x > acceptable_diff.x) return false;
  if (max_diff.y > acceptable_diff.y) return false;
  if (max_diff.z > acceptable_diff.z) return false;
  return true;
}

bool glfwSetWindowCenter( GLFWwindow * window )
{
    if( !window )
        return false;

    int sx = 0, sy = 0;
    int px = 0, py = 0;
    int mx = 0, my = 0;
    int monitor_count = 0;
    int best_area = 0;
    int final_x = 0, final_y = 0;

    glfwGetWindowSize( window , &sx, &sy );
    glfwGetWindowPos( window , &px, &py );

    // Iterate throug all monitors
    GLFWmonitor ** m = glfwGetMonitors( &monitor_count );
    if( !m )
        return false;

    for( int j = 0; j < monitor_count ; ++j )
    {

        glfwGetMonitorPos( m[j] , &mx, &my );
        const GLFWvidmode * mode = glfwGetVideoMode( m[j] );
        if( !mode )
            continue;

        // Get intersection of two rectangles - screen and window
        int minX = std::max( mx , px );
        int minY = std::max( my , py );

        int maxX = std::min( mx+mode->width , px+sx );
        int maxY = std::min( my+mode->height , py+sy );

        // Calculate area of the intersection
        int area = std::max( maxX - minX , 0 ) * std::max( maxY - minY , 0 );

        // If its bigger than actual (window covers more space on this monitor)
        if( area > best_area )
        {
            // Calculate proper position in this monitor
            final_x = mx + (mode->width-sx)/2;
            final_y = my + (mode->height-sy)/2;

            best_area = area;
        }

    }

    // We found something
    if( best_area )
        glfwSetWindowPos( window , final_x , final_y );

    // Something is wrong - current window has NOT any intersection with any monitors. Move it to the default one.
    else
    {
        GLFWmonitor * primary = glfwGetPrimaryMonitor( );
        if( primary )
        {
            const GLFWvidmode * desktop = glfwGetVideoMode( primary );

            if( desktop )
                glfwSetWindowPos( window , (desktop->width-sx)/2 , (desktop->height-sy)/2 );
            else
                return false;
        }
        else
            return false;
    }

    return true;
}

void prepareText(const std::string& text, const glm::vec2& pos, float size, std::vector<Vertex>& vertices, float offx) {
    float x = pos.x;
    float y = pos.y;

    for(size_t i = 0;i < text.length();i++) {
        char character = text[i];
        float uv_x = (character%16)/16.0f;
        float uv_y = (character/16)/16.0f;

        glm::vec3 vertex_up_left    = glm::vec3( x+i*offx*size     , y+size, 1 );
        glm::vec3 vertex_up_right   = glm::vec3( x+i*offx*size+size, y+size, 1 );
        glm::vec3 vertex_down_right = glm::vec3( x+i*offx*size+size, y, 1      );
        glm::vec3 vertex_down_left  = glm::vec3( x+i*offx*size     , y, 1      );

        glm::vec2 uv_up_left    = glm::vec2( uv_x           ,  uv_y + 1.0f/16.0f );
        glm::vec2 uv_up_right   = glm::vec2( uv_x+1.0f/16.0f,  uv_y + 1.0f/16.0f );
        glm::vec2 uv_down_right = glm::vec2( uv_x+1.0f/16.0f,  (uv_y) );
        glm::vec2 uv_down_left  = glm::vec2( uv_x           ,  (uv_y) );

        vertices.push_back(Vertex(vertex_up_left, uv_up_left   ));
        vertices.push_back(Vertex(vertex_down_left, uv_down_left ));
        vertices.push_back(Vertex(vertex_up_right, uv_up_right  ));

        vertices.push_back(Vertex(vertex_down_right, uv_down_right));
        vertices.push_back(Vertex(vertex_up_right, uv_up_right));
        vertices.push_back(Vertex(vertex_down_left, uv_down_left));
    }
}

