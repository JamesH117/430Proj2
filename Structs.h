typedef struct obj_camera{
    double width, height;
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


