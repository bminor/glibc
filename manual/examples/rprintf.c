#include <stdio.h>
#include <printf.h>
#include <stdarg.h>

typedef struct widget
  {
    char *name;
  } Widget;

int 
print_widget (FILE *stream, const struct printf_info *info, va_list *app)
{
  Widget *w;
  char *buffer;
  int len, fill, i;

  /* Format the output into a string. */
  w = va_arg (*app, Widget *);
  len = asprintf (&buffer, "<Widget %p: %s>", w, w->name);
  if (len == -1)
    return -1;
  fill = info->width - len;
  if (fill < 0)
    fill = 0;

  /* Pad to the minimum field width and print to the stream. */
  if (!info->left)
    for (i = 0; i < fill; i++)
      putc (' ', stream);
  fputs (buffer, stream);
  if (info->left)
    for (i = 0; i < fill; i++)
      putc (' ', stream);

  /* Clean up and return. */
  free (buffer);
  return (len + fill);
}


void 
main (void)
{

  /* Make a widget to print. */
  Widget mywidget;
  mywidget.name = "mywidget";

  /* Register the print function for widgets. */
  register_printf_function ('W', print_widget);

  /* Now print the widget. */
  printf ("|%W|\n", &mywidget);
  printf ("|%35W|\n", &mywidget);
  printf ("|%-35W|\n", &mywidget);
}
