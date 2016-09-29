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
        float *position;
        float *normal;
        float radius;

} scene_object;
typedef struct pixels{
    unsigned char r,g,b;
} pixels;

