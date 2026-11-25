struct Contact{
    bool hasContact;
    float depth;
    uvec2 handleA;
    uvec2 handleB;
    vec3 normal;
    vec3 witnessA;
    vec3 witnessB;
};

struct CollisionPairData{
    uint typeA;
    uint typeB;
    uint indexA;
    uint indexB;
    uvec2 instanceHandleA;
    uvec2 instanceHandleB;
    uint sizeA;
    uint sizeB;
};

struct TransformData {
    uint index;
    uint size;
    Motor3D motor;
    float scale;
};