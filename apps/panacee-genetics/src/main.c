#include <stdio.h>

#include <MLV/MLV_all.h>

int main(void)
{
    MLV_create_window("Panacée Genetics", "hello world", 640, 480);
    MLV_wait_seconds(10);
    MLV_free_window();
    return 0;
}
