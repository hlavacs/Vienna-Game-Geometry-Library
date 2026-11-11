struct Contact{
    bool hasContact;
    float depth;
    vec2 handleA;
    vec2 handleB;
    vec3 normal;
    vec3 witnessA;
    vec3 witnessB;
};

struct CollisionPairData{
    uint typeA;
    uint typeB;
    uint indexA;
    uint indexB;
    vec2 instanceHandleA;
    vec2 instanceHandleB;
    uint sizeA;
    uint sizeB;
    Motor3D motorA;
    Motor3D motorB;
};
