// Polygon.h provides abstractions for geometric calculations involving polygons and line segments
// Code is repurposed from my search path algorithm written for AUVSI
#pragma once
#include <cmath>
#include <vector>
#include <sstream>
#include <assert.h>
#include <string>
#include <cfloat>

#define float_type double
#define PI 3.14159265358979323846
#define EPSILON DBL_EPSILON
#define INF 1000000 // An effective infinity that will not overflow the float type.

namespace harv // Wrap everything in my own namespace to avoid conflicts
{
    //============================================================
    // Prototypes
    //============================================================
    struct Coord; // A 2-D coordinate. Also used to represent the positional vector from the origin to the coordinate.
    struct Edge; // An edge consisting of 2 coordinates.
    struct Polygon; // A polygon consisting of a list of coordinates in CCW order.
    float_type distance(const Coord&, const Coord&); // Find the distance between two vertices.
    bool intersection(const Edge&, const Edge&, Coord&); // Find the intersection of two line segments (represented as Edges) and return the intersection or NULL if it does not exist.

    //============================================================
    // Definitions
    //============================================================
    struct Coord // Simple struct representing a coordinate
    {
        float_type x, y;
        
        Coord(float_type xCoord = 0, float_type yCoord = 0)
        {
        x = xCoord;
        y = yCoord;
        }
        std::string str() const // For debugging. Can be removed in the final implementation
        {
        std::ostringstream s;
        s << "(" << x << "," << y << ")";
        return s.str();
        }
        bool operator==(const Coord &op) const
        { return (abs(x - op.x) < EPSILON && abs(y - op.y) < EPSILON); }
        bool operator !=(const Coord &op) const
        { return !((*this) == op); }
        // Some arithmetic operators to aid in vector arithmetic
        Coord operator+(const Coord &op) const
        { return Coord(x + op.x, y + op.y); }
        Coord operator-(const Coord &op) const
        { return Coord(x - op.x, y - op.y); }
        float_type operator*(const Coord &op) const // Dot product
        { return x * op.x + y * op.y; }
        Coord operator*(float_type mult) const // Scalar multiplication
        { return Coord(mult * x, mult * y); }
        float_type vectorLength() const // Return length of vector from origin to the coordinate
        { return sqrt((x * x) + (y * y)); }
    };

    struct Edge // Edge composed of two coordinates
    {
        Coord v1, v2;
        float_type a, b, c; // Coeffs for the standard form of the line (ax + by + c).
        
        Edge()
        {
            v1 = v2 = Coord();
            a = b = c = 0;
        }
        Edge(Coord vert1, Coord vert2)
        {
            v1 = vert1;
            v2 = vert2;
            float_type m;
            if (v1.x == v2.x) // The edge is perfectly vertical
                m = INFINITY; // Avoid division by zero. This value for the slope is merely a placeholder in this case.
            else
                m = (vert2.y - vert1.y) / (vert2.x - vert1.x);
            // Assign the values for the coeffs of the line in standard form
            a = -m;
            b = 1;
            c = (m * vert1.x) - vert1.y;
        }
        float_type slope() const
        { return -a; }
        bool isVertical() const
        { return v1.x == v2.x; }
        float_type length() const
        { return distance(v1, v2); }
        float_type theta() const // Returns the edge's angle from the horizontal in radians
        {
            if (isVertical())
            {
                if (v1.y < v2.y)
                return PI / 2.0;
                else
                return -(PI / 2.0);
            }
            else if (v1.y == v2.y) // Edge is horizontal
            {
                if (v1.x < v2.x)
                return 0;
                else
                return PI;
            }
            else
                return atan2(v2.y - v1.y, v2.x - v1.x);
        }
        bool operator==(const Edge &op) const
        { return ((v1 == op.v1) && (v2 == op.v2)) || ((v1 == op.v2) && (v2 == op.v1)); }
    };

    struct Polygon // A list of vertices stored in CCW order. The polygon is assumed to be simple with no holes.
    {
        // Using std::vector to store the list of vertices for efficient random access. This can be replaced with a more memory efficient container later if necessary
        std::vector<Coord> v; // The vertices of the polygon in counter-clockwise order
        
        void addVert(Coord vert)
        { v.push_back(vert); }
        unsigned int size() const
        { return v.size(); }
        Edge edge(unsigned int i) const // Constructs and returns a specific edge of the polygon indexed by the index of the first vertex
        { return Edge(v.at(i), v.at((i + 1) % v.size())); }
        bool adjacent(Polygon &p, int *edgeIndex1 = NULL, int *edgeIndex2 = NULL) const // Return true if p is adjacent and store the indeces for the edge in edgeIndex1 and 2 respectively. Else, return false and set indices to -1.
        { // This is the O(nm) solution which should be good enough unless our polygons are crazy. If more speed is required, use a hashing solution for linear time complexity.
            for (unsigned int i = 0; i < v.size(); ++i)
            {
                Edge e1 = edge(i);
                for (unsigned int j = 0; j < p.size(); ++j)
                {
                Edge e2 = p.edge(j);
                if (e1 == e2)
                {
                    if (edgeIndex1 != NULL)
                    *edgeIndex1 = i;
                    if (edgeIndex2 != NULL)
                    *edgeIndex2 = j;
                    return true;
                }
                }		
            }
            if (edgeIndex1 != NULL)
                *edgeIndex1 = -1;
            if (edgeIndex2 != NULL)
                *edgeIndex2 = -1;
            return false;
        }
        Coord center() const // Return the center of the bounding box for the polygon
        {
            float_type xMin, xMax;
            float_type yMin, yMax;
            xMin = xMax = v[0].x;
            yMin = yMax = v[0].y;
            for (unsigned int i = 1; i < v.size(); ++i)
            {
                if (v[i].x < xMin)
                xMin = v[i].x;
                if (v[i].x > xMax)
                xMax = v[i].x;
                if (v[i].y < yMin)
                yMin = v[i].y;
                if (v[i].y > yMax)
                yMax = v[i].y;
            }
            return Coord(((xMin + xMax) / 2), ((yMin + yMax) / 2));
        }
        std::string str() const
        {
            std::ostringstream result;
            for (unsigned int i = 0; i < size(); ++i)
                result << v[i].str() << '\n';
            return result.str();
        }
        Polygon& operator=(const Polygon &op)
        {
            v = op.v;
            return *this;
        }
    };

    inline float_type cross(const Coord &c1, const Coord &c2) // Cross two coordinates by treating them as positional vectors and return the result
    { return c1.x * c2.y - c2.x * c1.y; }

    bool intersection(const Edge &e1, const Edge &e2, Coord &intersect) // Finds the intersection of two line segments and stores the result in intersect.
    // Otherwise, intersect will be NULL.
    {
        // References this: https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282
        // and this: https://www.codeproject.com/tips/862988/find-the-intersection-point-of-two-line-segments
        // We'll use the naming convention used in the reference posts
        Coord p1 = e1.v1; Coord p2 = e1.v2;
        Coord q1 = e2.v1; Coord q2 = e2.v2;
        Coord r = Coord(p2.x - p1.x, p2.y - p1.y);
        Coord s = Coord(q2.x - q1.x, q2.y - q1.y);
        float_type rxs = cross(r, s);
        float_type qpxr = cross(Coord(q1.x - p1.x, q1.y - p1.y), r);
        if (abs(rxs) < EPSILON && abs(qpxr) < EPSILON) // The two edges are collinear
        {
            intersect = e1.v1;
            return true;
        }
        if (abs(rxs) < EPSILON && !(abs(qpxr) < EPSILON)) // The two lines are parallel and non-intersecting
            return false;
        float_type t = cross(q1 - p1, s) / rxs;
        float_type u = cross (q1 - p1, r) / rxs;
        if (!(abs(rxs) < EPSILON) && (0 <= t && t <= 1) && (0 <= u && u <= 1)) // The two segments meet at point p1 + tr = q1 + us
        {
            intersect = (p1 + (r * t));
            return true;
        }
            return false; // No intersection was found
    }

    float_type distance(const Coord &v1, const Coord &v2) // Find the distance between two vertices
    { return sqrt(pow((v2.y - v1.y), 2) + pow(v2.x - v1.x, 2)); }
}

