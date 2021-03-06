#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <form.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __linux__
#include <bsd/bsd.h>
#endif

#define KEY_CTRL(K) ((K) & 0x1f)
#define CUTBUF_LEN 67
#define NFIELDS 20

void ull2bin(unsigned long long v, unsigned end, char *buf)
{
  int s;
  int i;

  s = end + 1;
  for (i = s; i > 0; i--)
    buf[s - i] = (v & (1ULL << (i - 1))) ? '1' : '0';
  buf[s - i] = '\0';
}

void ull2ascii(unsigned long long v, unsigned end, char *buf)
{
  int c;
  int i;
  char ch;

  for (i = 0, c = 0; i < 64; c++, i += 8)
  {
    ch = ((v >> i) & 0xff);
    if ((ch > ' ') && (ch < 0x7f))
      buf[c] = ch;
    else
      buf[c] = ' ';
  }

  buf[c] = '\0';
}

void do_update(FORM *form, FIELD **field)
{
  unsigned long long a;
  unsigned long long b;
  unsigned long long v;
  unsigned start;
  unsigned end;
  unsigned tmp;
  char buf[CUTBUF_LEN];
  char *val;
  char *bit;
  char *ep;
  char *op;
  int set;

  form_driver(form, REQ_VALIDATION);
  val = field_buffer(field[0], 0);
  bit = field_buffer(field[1], 0);

  while (*val && (*val == ' '))
    val++;
  if (!strncmp(val, "0b", 2))
    a = strtoull(val + 2, &op, 2);
  else
    a = strtoull(val, &op, 0);
  if (isxdigit(*op))
    a = strtoull(val, &op, 16);

  while (*op && (*op == ' '))
    op++;
  switch (*op)
  {
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
    ep = op + 1;
    while (*ep && (*ep == ' '))
      ep++;
    if (!strncmp(ep, "0b", 2))
      b = strtoull(ep + 2, &val, 2);
    else
      b = strtoull(ep, &val, 0);
    if (isxdigit(*val))
      b = strtoull(ep, NULL, 16);
    switch(*op)
    {
    case '+':
      v = a + b;
      break;
    case '-':
      v = a - b;
      break;
    case '*':
      v = a * b;
      break;
    case '/':
      if (b)
	v = a / b;
      else
	v = a;
      break;
    case '%':
      if (b)
	v = a % b;
      else
	v = a;
      break;
    }
    break;
  default:
    v = a;
    break;
  }

  ull2bin(v, 63, buf);
  set_field_buffer(field[2], 0, buf);

  set = 0;
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
      set = 1;
      end -= start;
      v >>= start;
      v <<= 63 - end;
      v >>= 63 - end;
    }
  }
  if (set == 0)
  {
    start = 0;
    end = 63;
  }

  memset(buf, ' ', CUTBUF_LEN - 1);
  buf[CUTBUF_LEN - 1] = '\0';
  buf[63-(end+start)] = '^';
  buf[63-start] = '^';
  set_field_buffer(field[3], 0, buf);
  ull2bin(v, end, buf);
  set_field_buffer(field[4], 0, buf);
  snprintf(buf, CUTBUF_LEN - 1, "%llo", v);
  set_field_buffer(field[5], 0, buf);
  snprintf(buf, CUTBUF_LEN - 1, "%llu", v);
  set_field_buffer(field[6], 0, buf);
  snprintf(buf, CUTBUF_LEN - 1, "%llx", v);
  set_field_buffer(field[7], 0, buf);
  ull2ascii(v, end, buf);
  set_field_buffer(field[8], 0, buf);
  snprintf(buf, CUTBUF_LEN - 1, "[%u:0] = %u bits", end, end + 1);
  set_field_buffer(field[9], 0, buf);
}

int main(int ac, char **av)
{
  FIELD *field[NFIELDS+1];
  FORM *form;
  char cutbuf[CUTBUF_LEN];
  int quit;
  int ch;
  int i;

  cutbuf[0] = '\0';

  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  for (i = 0; i < (NFIELDS / 2); i++)
    field[i] = new_field(1, CUTBUF_LEN - 1, i < 2 ? i + 1 : i + 2, 12, 0, 0);
  for (; i < NFIELDS; i++)
    field[i] = new_field(1, 10, i < 12 ? i - 9 : i - 8, 1, 0, 0);
  field[NFIELDS] = NULL;

  for (i = 0; i < 2; i++)
  {
    set_field_back(field[i], A_UNDERLINE);
    field_opts_off(field[i], O_AUTOSKIP);
    field_opts_off(field[i], O_BLANK);
  }

  for (i = 2; i < NFIELDS; i++)
    field_opts_off(field[i], O_ACTIVE);
  set_field_buffer(field[10], 0, "value   :");
  set_field_buffer(field[11], 0, "bit sli.:");
  set_field_buffer(field[12], 0, "full bin:");
  set_field_buffer(field[13], 0, "range   :");
  set_field_buffer(field[14], 0, "binary  :");
  set_field_buffer(field[15], 0, "octal   :");
  set_field_buffer(field[16], 0, "decimal :");
  set_field_buffer(field[17], 0, "hexa    :");
  set_field_buffer(field[18], 0, "ascii   :");
  set_field_buffer(field[19], 0, "bit vec.:");

  form = new_form(field);
  post_form(form);

  refresh();

  for (quit = 0; !quit;)
  {
    do_update(form, field);
    ch = getch();
    switch(ch)
    {
    case KEY_CTRL('z'):
      def_prog_mode();
      endwin();
      kill(getpid(), SIGSTOP);
      reset_prog_mode();
      refresh();
      break;
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
    case KEY_CTRL('d'):
      form_driver(form, REQ_DEL_CHAR);
      break;
    case 0x7f:
    case KEY_BACKSPACE:
      form_driver(form, REQ_DEL_PREV);
      break;
    default:
      form_driver(form, ch);
      break;
    }
  }

  unpost_form(form);
  free_form(form);
  for (i = 0; i < NFIELDS; i++)
    free_field(field[i]);

  endwin();
  return EXIT_SUCCESS;
}
