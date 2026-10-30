#ifndef SHELL_H
#define SHELL_H

void shell_init(void);
void shell_poll(void);   // call every pass of the main loop; never blocks
void shell_run_line(char *line);

#endif
