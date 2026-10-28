struct Point3D {
    float x;
    float y;
    float z;
};

vec3 toVec(Point3D p) {
    return vec3(p.x,p.y,p.z);
}

Point3D toPoint(vec3 v) {
    return Point3D(v.x, v.y, v.z);
}

struct Vector3D {
    float x;
    float y;
    float z;
};

float magnitude(const Vector3D v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float squaredMag(const Vector3D v){
    return (v.x * v.x + v.y * v.y + v.z * v.z);
}

float dot(const Vector3D a, const Vector3D b){
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

vec3 toVec(const Vector3D v) {
    return vec3(v.x,v.y,v.z);
}

Point3D add(const Point3D p, const Vector3D v){
    return Point3D(p.x+v.x, p.y+v.y, p.z+v.z);
}

Vector3D subtract(const Point3D p1, const Point3D p2) {
    return Vector3D(p1.x-p2.x, p1.y-p2.y, p1.z-p2.z);
}
Point3D subtract(const Point3D p, const Vector3D v) {
    return Point3D(p.x-v.x, p.y-v.y, p.z-v.z);
}

Vector3D multiply(const Vector3D v, float scalar) {
    return Vector3D(v.x*scalar, v.y*scalar, v.z*scalar);
}

Vector3D divide(const Vector3D v, float scalar) {
    return Vector3D(v.x/scalar, v.y/scalar, v.z/scalar);
}

struct FlatPoint3D {
    float x;
    float y;
    float z;
    float w;
};

struct Plane3D {
    float x;
    float y;
    float z;
    float w;
};

struct Bivector3D {
    float x;
    float y;
    float z;
};

struct Circle3D {
    Plane3D g;
    Vector3D v;
    Bivector3D m;
};


float squaredRadiusNorm(const Circle3D c){
    return (c.v.x * c.v.x + c.v.y * c.v.y + c.v.z * c.v.z + (c.g.x * c.m.x + c.g.y * c.m.y + c.g.z * c.m.z) * 2.0 - c.g.w * c.g.w);
}


struct Sphere3D {
    float u;
    float x;
    float y;
    float z;
    float w;
};


FlatPoint3D flatCenter(const Sphere3D s) {
    return FlatPoint3D(s.x, s.y, s.z, -s.u);
}

float squaredRadiusNorm(const Sphere3D s){
    return (s.x * s.x + s.y * s.y + s.z * s.z - s.w * s.u * 2.0);
}

Sphere3D unitize(const Sphere3D sphere){
    float n = -1.0/ sphere.u;
    return Sphere3D(-1.0, sphere.x *n, sphere.y *n, sphere.z *n, sphere.w *n);
}

Circle3D antiwedge(const Sphere3D s, const Sphere3D t){
    const Plane3D g = Plane3D(s.u * t.x - s.x * t.u, 
                        s.u * t.y - s.y * t.u,
                        s.u * t.z - s.z * t.u,
                        s.u * t.w - s.w * t.u);
                        
    const Vector3D v = Vector3D(s.z * t.y - s.y * t.z,
                          s.x * t.z - s.z * t.x,
                          s.y * t.x - s.x * t.y);

    const Bivector3D m = Bivector3D(s.x * t.w - s.w * t.x,
                              s.y * t.w - s.w * t.y,
                              s.z * t.w - s.w * t.z);

    return Circle3D(g, v, m);
}

struct Capsule {
    Point3D a;
    Point3D b;
    float radius;
};
