/* npmount.c */
int main(int argc, char *argv[]);
int np_mount(const char *mount_point);
int np_umount(const char *mount_point);
void read_config(mount_point_t mount_points[]);
void usage(char *argv[]);
