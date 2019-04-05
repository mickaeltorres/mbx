#include <stdlib.h>
#include <ncurses.h>
#include <form.h>
#include <string.h>
#include <stdio.h>

#define KEY_CTRL(K) ((K) & 0x1f)
#define CUTBUF_LEN 67

void ull2bin(unsigned long long v, char *buf)
{
  int i;

  for (i = 64; i > 0; i--)
    buf[64 - i] = (v & (1ULL << (i - 1))) ? '1' : '0';
  buf[64] = '\0';
}

void do_update(FORM *form, FIELD **field)
{
  unsigned long long v;
  unsigned start;
  unsigned end;
  unsigned tmp;
  char buf[CUTBUF_LEN];
  char *val;
  char *bit;
  
  form_driver(form, REQ_VALIDATION);
  val = field_buffer(field[0], 0);
  bit = field_buffer(field[1], 0);
  
  if (!strncmp(val, "0b", 2))
    v = strtoull(val + 2, NULL, 2);
  else if (!strncmp(val, "0x", 2))
    v = strtoull(val + 2, NULL, 16);
  else if (val[0] == '0')
    v = strtoull(val + 1, NULL, 8);
  else
    v = strtoull(val, NULL, 10);

  if (sscanf(bit, "%u:%u", &start, &end) == 2)
  {
    if ((start < 64) && (end < 64))
    {
      if (start > end)
      {
	tmp = start;
	start = end;
	end = tmp;
      }
      end -= start;
      v >>= start;
      v <<= 63 - end;
      v >>= 63 - end;
    }
  }

  ull2bin(v, buf);
  set_field_buffer(field[2], 0, buf);
  snprintf(buf, CUTBUF_LEN - 1, "%llo", v);
  set_field_buffer(field[3], 0, buf);
  snprintf(buf, CUTBUF_LEN - 1, "%llu", v);
  set_field_buffer(field[4], 0, buf);
  snprintf(buf, CUTBUF_LEN - 1, "%llx", v);
  set_field_buffer(field[5], 0, buf);
}

int main(int ac, char **av)
{
  FIELD *field[13];
  FORM *form;
  char cutbuf[CUTBUF_LEN];
  int quit;
  int ch;

  cutbuf[0] = '\0';
  
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  field[0] = new_field(1, CUTBUF_LEN - 1, 1, 12, 0, 0);
  field[1] = new_field(1, CUTBUF_LEN - 1, 2, 12, 0, 0);
  field[2] = new_field(1, CUTBUF_LEN - 1, 4, 12, 0, 0);
  field[3] = new_field(1, CUTBUF_LEN - 1, 5, 12, 0, 0);
  field[4] = new_field(1, CUTBUF_LEN - 1, 6, 12, 0, 0);
  field[5] = new_field(1, CUTBUF_LEN - 1, 7, 12, 0, 0);
  field[6] = new_field(1, 10, 1, 1, 0, 0);
  field[7] = new_field(1, 10, 2, 1, 0, 0);
  field[8] = new_field(1, 10, 4, 1, 0, 0);
  field[9] = new_field(1, 10, 5, 1, 0, 0);
  field[10] = new_field(1, 10, 6, 1, 0, 0);
  field[11] = new_field(1, 10, 7, 1, 0, 0);
  field[12] = NULL;

  set_field_back(field[0], A_UNDERLINE);
  field_opts_off(field[0], O_AUTOSKIP);
  field_opts_off(field[0], O_BLANK);
  set_field_back(field[1], A_UNDERLINE);
  field_opts_off(field[1], O_AUTOSKIP);
  field_opts_off(field[1], O_BLANK);

  field_opts_off(field[2], O_ACTIVE);
  field_opts_off(field[3], O_ACTIVE);
  field_opts_off(field[4], O_ACTIVE);
  field_opts_off(field[5], O_ACTIVE);
  field_opts_off(field[6], O_ACTIVE);
  field_opts_off(field[7], O_ACTIVE);
  field_opts_off(field[8], O_ACTIVE);
  field_opts_off(field[9], O_ACTIVE);
  field_opts_off(field[10], O_ACTIVE);
  field_opts_off(field[11], O_ACTIVE);

  set_field_buffer(field[6], 0, "value   :");
  set_field_buffer(field[7], 0, "bit sli.:");
  set_field_buffer(field[8], 0, "binary  :");
  set_field_buffer(field[9], 0, "octal   :");
  set_field_buffer(field[10], 0, "decimal :");
  set_field_buffer(field[11], 0, "hexa    :");
  
  form = new_form(field);
  post_form(form);
  
  refresh();

  for (quit = 0; !quit;)
  {
    ch = getch();
    switch(ch)
    {
    case KEY_CTRL('c'):
    case KEY_CTRL('q'):
      quit = 1;
      break;
    case KEY_CTRL('a'):
    case KEY_HOME:
      form_driver(form, REQ_BEG_LINE);
      break;
    case KEY_CTRL('e'):
    case KEY_END:
      form_driver(form, REQ_END_LINE);
      break;
    case KEY_CTRL('k'):
      strlcpy(cutbuf, field_buffer(current_field(form), 0), CUTBUF_LEN);
      form_driver(form, REQ_CLR_EOL);
      break;
    case KEY_CTRL('y'):
      set_field_buffer(current_field(form), 0, cutbuf);
      form_driver(form, REQ_END_LINE);
      break;
    case '\n':
    case '\t':
    case KEY_DOWN:
      form_driver(form, REQ_NEXT_FIELD);
      form_driver(form, REQ_END_LINE);
      break;
    case KEY_UP:
      form_driver(form, REQ_PREV_FIELD);
      form_driver(form, REQ_END_LINE);
      break;
    case KEY_LEFT:
      form_driver(form, REQ_PREV_CHAR);
      break;
    case KEY_RIGHT:
      form_driver(form, REQ_NEXT_CHAR);
      break;
    case KEY_BACKSPACE:
      form_driver(form, REQ_DEL_PREV);
      break;
    default:
      form_driver(form, ch);
      break;
    }
    do_update(form, field);
  }

  unpost_form(form);
  free_form(form);
  free_field(field[0]);
  free_field(field[1]);
  
  endwin();
  return EXIT_SUCCESS;
}
