typedef void (*tty_handler_t)(char);

void tty_set_handler(tty_handler_t* handler);
void tty_init();
