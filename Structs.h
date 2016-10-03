typedef struct obj_camera{
    float width, height;
} obj_camera;

typedef struct obj_sphere{
    double radius;
    double *color;
    double *position;
} obj_sphere;

typedef struct obj_plane{
    double *color;
    double *position;
    double *normal;
} obj_plane;

typedef struct scene_object{
        char type;
        float *color;
        //What is position relative to?
        //Position is relative from origin in 3D space
        //Camera is at origin and looking down positive z
        float *position;
        float *normal;
        float radius;

} scene_object;
typedef struct pixels{
    unsigned char r,g,b;
} pixels;

