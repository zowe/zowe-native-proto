#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include "zb64.h"

#define FIFO_PATH "/tmp/zowe-test-fifo"

void downloadB64(const char *filepath, size_t chunksize)
{
    // Open the input file for reading
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    // Remove existing FIFO if any
    // unlink(FIFO_PATH);

    // Create FIFO with permission 0600
    // if (mkfifo(FIFO_PATH, 0600) == -1)
    // {
    //     perror("Failed to create FIFO");
    //     fclose(file);
    //     exit(EXIT_FAILURE);
    // }

    // Open FIFO for writing
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1)
    {
        perror("Failed to open FIFO for writing");
        fclose(file);
        unlink(FIFO_PATH);
        exit(EXIT_FAILURE);
    }

    // Allocate buffer for reading chunks
    int input_size = chunksize * 3 / 4;
    unsigned char *buffer = malloc(input_size);
    char *encoded = malloc(chunksize);
    if (!buffer || !encoded)
    {
        fprintf(stderr, "Failed to allocate buffer\n");
        fclose(file);
        close(fifo_fd);
        unlink(FIFO_PATH);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, input_size, file)) > 0)
    {
        int encoded_len;
        encoded = base64(buffer, bytes_read, &encoded_len);
        // fprintf(stderr, "Chunk size: (%d, %d), Bytes read: %d, Encoded len: %d\n", chunksize, input_size, bytes_read, encoded_len);
        // fprintf(stderr, "Encoded: %s\n", encoded);
        size_t total_written = 0;
        while (total_written < encoded_len)
        {
            ssize_t bytes_written = write(fifo_fd, encoded + total_written, encoded_len - total_written);
            // write(STDERR_FILENO, encoded + total_written, encoded_len - total_written);
            if (bytes_written == -1)
            {
                perror("Failed to write to FIFO");
                free(buffer);
                fclose(file);
                close(fifo_fd);
                unlink(FIFO_PATH);
                exit(EXIT_FAILURE);
            }
            total_written += bytes_written;
        }
    }

    if (ferror(file))
    {
        fprintf(stderr, "Error reading input file\n");
    }

    // Cleanup
    free(buffer);
    free(encoded);
    fclose(file);
    close(fifo_fd);

    // FIFO remains on disk; remove if desired
    // unlink(FIFO_PATH);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <filepath> <chunksize>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filepath = argv[1];
    size_t chunksize = (size_t)atoi(argv[2]);
    if (chunksize == 0)
    {
        fprintf(stderr, "Invalid chunksize\n");
        return EXIT_FAILURE;
    }

    downloadB64(filepath, chunksize);
    return EXIT_SUCCESS;
}
