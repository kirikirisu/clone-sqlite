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

/* メタコマンド = .から始まるコマンド */
typedef enum
{
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
} PrepareResult;

typedef enum
{
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

typedef struct
{
  StatementType type;
} Statement;

void close_input_buffer(InputBuffer *);

/* インスタンスを初期化する関数 */
InputBuffer *new_input_buffer()
{
  InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer)
{
  if (strcmp(input_buffer->buffer, ".exit") == 0)
  {
    close_input_buffer(input_buffer);
    exit(EXIT_SUCCESS);
  }
  else
  {
    // 1を返す
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

/* readme 2 */
PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
  if (strncmp(input_buffer->buffer, "insert", 6) == 0)
  {
    statement->type = STATEMENT_INSERT;
    return PREPARE_SUCCESS;
  }
  else
  {
    if (strcmp(input_buffer->buffer, "select") == 0)
    {
      statement->type = STATEMENT_SELECT;
      return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
  }
}

/* readme 2 */
void execute_statement(Statement *statement)
{
  switch (statement->type)
  {
  case (STATEMENT_INSERT):
    printf("this is where we would do an insert. \n");
    break;
  case (STATEMENT_SELECT):
    printf("this is where we would do a select. \n");
    break;
  }
}

void print_prompt()
{
  printf("db > ");
}

void read_input(InputBuffer *input_buffer)
{
  /* readme 1 */
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
  // printf("%zd\n", bytes_read);

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

    if (input_buffer->buffer[0] == '.')
    {
      switch (do_meta_command(input_buffer))
      {
      case (META_COMMAND_SUCCESS):
        /* while文の残りの処理をスキップして、whileの最初に戻る */
        continue;
      case (META_COMMAND_UNRECOGNIZED_COMMAND):
        printf("Unrecognized command '%s'\n", input_buffer->buffer);
        continue;
      }
    }

    Statement statement;
    switch (prepare_statement(input_buffer, &statement))
    {
    case (PREPARE_SUCCESS):
      /* switchを抜けて、while文の残りの処理を実行する */
      break;
    case (PREPARE_UNRECOGNIZED_STATEMENT):
      printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
      continue;
    }

    execute_statement(&statement);
    printf("Executed.\n");
  }
}
