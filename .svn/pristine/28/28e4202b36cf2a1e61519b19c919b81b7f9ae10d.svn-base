#include <stdio.h>
#include "yaml.h"

int main ()
{
    yaml_parser_t parser;
    yaml_event_t event;

    int done = 0;

/* Create the Parser object. */
    yaml_parser_initialize(&parser);

/* Set a string input. */
    const char *input = "...";
    size_t length = strlen(input);

    yaml_parser_set_input_string(&parser, (const unsigned char*) input, length);

/* Set a file input. */
    FILE *fp = fopen("...", "rb");

    yaml_parser_set_input_file(&parser, fp);

/* Set a generic reader. */
#if defined (commentout)
    void *ext = ...;
    int read_handler(void *ext, char *buffer, int size, int *length) {
        /* ... */
        *buffer = ...;
        *length = ...;
        /* ... */
        return error ? 0 : 1;
    }

    yaml_parser_set_input(&parser, read_handler, ext);
#endif
    
    /* Read the event sequence. */
    while (!done) {

        /* Get the next event. */
        if (!yaml_parser_parse(&parser, &event))
            goto error;

        /*
          ...
          Process the event.
          ...
        */

        /* Are we finished? */
        done = (event.type == YAML_STREAM_END_EVENT);

        /* The application is responsible for destroying the event object. */
        yaml_event_delete(&event);

    }

/* Destroy the Parser object. */
    yaml_parser_delete(&parser);

    return 1;

/* On error. */
error:

    /* Destroy the Parser object. */
    yaml_parser_delete(&parser);

    return 0;
}
