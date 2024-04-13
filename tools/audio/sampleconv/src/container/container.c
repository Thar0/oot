#include <stdlib.h>

#include "container.h"

int
container_destroy(container_data *data)
{
    if (data->data != NULL)
        free(data->data);
    if (data->loops != NULL)
        free(data->loops);
    if (data->vadpcm.book_data != NULL)
        free(data->vadpcm.book_data);
    if (data->vadpcm.loops != NULL)
        free(data->vadpcm.loops);
    return 0;
}
