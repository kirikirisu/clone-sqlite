#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

// インスタンスを初期化する関数
InputBuffer *new_input_buffer()
{
  InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void print_prompt()
{
  printf("db > ");
}

void read_input(InputBuffer *input_buffer)
{
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
  printf("%zd\n", bytes_read);

  if (bytes_read <= 0)
  {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // bytes_readは改行を含めた文字数。-1で入力された文字数。
  input_buffer->input_length = bytes_read - 1;
  // buffer[bytes_read -1]は文字列の最後の改行を示す。
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer *input_buffer)
{
  free(input_buffer->buffer);
  free(input_buffer);
}

int main(int argc, char *argv[])
{
  InputBuffer *input_buffer = new_input_buffer();
  while (true)
  {
    print_prompt();
    read_input(input_buffer);

    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
      close_input_buffer(input_buffer);
      exit(EXIT_SUCCESS);
    }
    else
    {
      printf("Unrecognized command '%s'.\n", input_buffer->buffer);
    }
  }
}
