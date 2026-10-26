namespace vgeo::internal::gpu{

struct AaBox{
    float min_x;
    float min_y;
    float min_z;
    float min_padding;
    float max_x;
    float max_y;
    float max_z;
    float max_padding;
};

}